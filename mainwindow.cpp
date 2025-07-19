#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pdfviewer.h"
#include <QMessageBox>
#include <QSqlError>
#include <QDebug>
#include <QSqlQuery>
#include <QFileDialog>
#include <QInputDialog>
#include <QFileInfo>
#include <poppler/qt6/poppler-qt6.h>
#include <memory>
#include "progressbar_delegate.h"
#include "pdfcoverdelegate.h"
#include <QInputDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , pdfViewer(nullptr)
{
    ui->setupUi(this);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    setupDB();
    loadData();

    PdfCoverDelegate *coverDelegate = new PdfCoverDelegate(this);
    ui->tableView->setItemDelegateForColumn(1, coverDelegate);
    ui->tableView->verticalHeader()->setDefaultSectionSize(40);


}

MainWindow::~MainWindow()
{
    qDebug() << "Уничтожение MainWindow";
    if (pdfViewer) {
        pdfViewer->deleteLater();
    }
    delete ui;
}

void MainWindow::setupDB()
{
    qDebug() << "Настройка базы данных";
    if (QSqlDatabase::contains("book_manager_connection")) {
        db = QSqlDatabase::database("book_manager_connection");
        if (db.isOpen()) {
            qDebug() << "Соединение уже открыто";
        }
    } else {
        db = QSqlDatabase::addDatabase("QPSQL", "book_manager_connection");
    }

    db.setHostName("localhost");
    db.setDatabaseName("book_manager");
    db.setUserName("postgres");
    db.setPassword(qEnvironmentVariable("DB_PASSWORD", "1973"));

    if (!db.open()) {
        QMessageBox::critical(this, "Ошибка подключения к БД", db.lastError().text());
        qDebug() << "Database connection failed:" << db.lastError().text();
        return;
    }

    QSqlQuery q(db);

    // Создание таблицы documents
    QString createDocumentsTable = R"(
        CREATE TABLE IF NOT EXISTS documents (
            id SERIAL PRIMARY KEY,
            title TEXT NOT NULL,
            author TEXT,
            filepath TEXT NOT NULL UNIQUE,
            date_added DATE DEFAULT CURRENT_DATE
        )
    )";
    if (!q.exec(createDocumentsTable)) {
        QMessageBox::critical(this, "Ошибка создания таблицы documents", q.lastError().text());
        qDebug() << "Failed to create documents table:" << q.lastError().text();
        return;
    }

    // Создание таблицы reading_progress
    QString createReadingProgressTable = R"(
        CREATE TABLE IF NOT EXISTS reading_progress (
            id SERIAL PRIMARY KEY,
            document_id INTEGER NOT NULL REFERENCES documents(id) ON DELETE CASCADE,
            last_page INTEGER DEFAULT 0,
            total_pages INTEGER DEFAULT 0,
            last_opened TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
            CONSTRAINT unique_document_id UNIQUE (document_id)
        )
    )";
    if (!q.exec(createReadingProgressTable)) {
        QMessageBox::critical(this, "Ошибка создания таблицы reading_progress", q.lastError().text());
        qDebug() << "Failed to create reading_progress table:" << q.lastError().text();
        return;
    }

    // Добавление уникального ограничения, если оно отсутствует
    QString addUniqueConstraint = R"(
        DO $$
        BEGIN
            IF NOT EXISTS (
                SELECT 1
                FROM pg_constraint
                WHERE conname = 'unique_document_id'
                AND conrelid = 'reading_progress'::regclass
            ) THEN
                ALTER TABLE reading_progress
                ADD CONSTRAINT unique_document_id UNIQUE (document_id);
            END IF;
        END $$;
    )";
    if (!q.exec(addUniqueConstraint)) {
        QMessageBox::critical(this, "Ошибка добавления уникального ограничения", q.lastError().text());
        qDebug() << "Failed to add unique constraint to reading_progress:" << q.lastError().text();
        return;
    }

    // Создание таблицы open_stats
    QString createOpenStatsTable = R"(
        CREATE TABLE IF NOT EXISTS open_stats (
            id SERIAL PRIMARY KEY,
            document_id INTEGER NOT NULL REFERENCES documents(id) ON DELETE CASCADE,
            open_time TIMESTAMP NOT NULL DEFAULT CURRENT_TIMESTAMP,
            close_time TIMESTAMP
        )
    )";
    if (!q.exec(createOpenStatsTable)) {
        QMessageBox::critical(this, "Ошибка создания таблицы open_stats", q.lastError().text());
        qDebug() << "Failed to create open_stats table:" << q.lastError().text();
        return;
    }

    qDebug() << "Database and tables initialized successfully";
}

void MainWindow::loadData()
{
    qDebug() << "Загрузка данных в таблицу";
    model = new QSqlQueryModel(this);

    QString query = R"(
        SELECT
            d.id,
            d.title,
            d.author,
            d.filepath,
            d.date_added,
            COALESCE(ROUND((rp.last_page::FLOAT / NULLIF(rp.total_pages, 0) * 100)::NUMERIC, 2), 0) AS progress
        FROM documents d
        LEFT JOIN reading_progress rp ON d.id = rp.document_id
    )";

    model->setQuery(query, db);

    if (model->lastError().isValid()) {
        qDebug() << "Ошибка выполнения запроса:" << model->lastError().text();
        QMessageBox::critical(this, "Ошибка", "Не удалось загрузить данные: " + model->lastError().text());
        return;
    }

    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Название"));
    model->setHeaderData(2, Qt::Horizontal, tr("Автор"));
    model->setHeaderData(3, Qt::Horizontal, tr("Путь к файлу"));
    model->setHeaderData(4, Qt::Horizontal, tr("Дата добавления"));
    model->setHeaderData(5, Qt::Horizontal, tr("Прогресс (%)"));

    ui->tableView->setModel(model);
    ui->tableView->setColumnHidden(0, true);
    ui->tableView->setColumnHidden(3, true);
    ui->tableView->setColumnHidden(4, true);

    ui->tableView->setItemDelegateForColumn(5, new ProgressBarDelegate(this));




    QHeaderView *header = ui->tableView->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    if (model->rowCount() == 0) {
        QMessageBox::information(this, "Информация", "Таблица документов пуста. Добавьте новый документ.");
    }

    qDebug() << "Данные успешно загружены, строк:" << model->rowCount();
}

void MainWindow::on_AddBtn_clicked()
{
    qDebug() << "Нажата кнопка AddBtn";
    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("Выберите PDF-файл"),
        QDir::homePath(),
        tr("PDF Files (*.pdf)")
        );

    if (filePath.isEmpty()) {
        qDebug() << "Выбор файла отменен";
        return;
    }

    QFileInfo fileInfo(filePath);
    QString defaultTitle = fileInfo.baseName();

    bool ok;
    QString title = QInputDialog::getText(
        this,
        tr("Название документа"),
        tr("Введите название документа:"),
        QLineEdit::Normal,
        defaultTitle,
        &ok
        );
    if (!ok || title.isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Название документа не указано!");
        qDebug() << "Название документа не указано";
        return;
    }

    QString author = QInputDialog::getText(
        this,
        tr("Автор документа"),
        tr("Введите автора документа (можно оставить пустым):"),
        QLineEdit::Normal,
        "",
        &ok
        );
    if (!ok) {
        author = "";
    }

    if (!db.isOpen()) {
        qDebug() << "База данных закрыта, пытаемся открыть";
        if (!db.open()) {
            qDebug() << "Не удалось открыть базу данных: " << db.lastError().text();
            QMessageBox::critical(this, "Ошибка", "Не удалось подключиться к базе данных!");
            return;
        }
    }

    QSqlQuery query(db);
    query.prepare(R"(
        INSERT INTO documents (title, author, filepath)
        VALUES (:title, :author, :filepath)
        RETURNING id
    )");
    query.bindValue(":title", title);
    query.bindValue(":author", author.isEmpty() ? QVariant(QVariant::String) : author);
    query.bindValue(":filepath", filePath);

    if (!query.exec()) {
        QString errorText = query.lastError().text();
        if (errorText.contains("unique constraint")) {
            QMessageBox::critical(this, "Ошибка", "Файл уже добавлен в базу данных!");
        } else {
            QMessageBox::critical(this, "Ошибка", "Не удалось добавить документ: " + errorText);
        }
        qDebug() << "Ошибка добавления документа:" << errorText;
        return;
    }

    loadData();
    qDebug() << "Документ добавлен, путь:" << filePath;
}

void MainWindow::on_SearchBtn_clicked()
{
    qDebug() << "Нажата кнопка SearchBtn";
    QString searchText = ui->SearchEdit->text().toLower();
    QString queryString = R"(
    SELECT
        d.id,
        d.title,
        d.author,
        d.filepath,
        d.date_added,
        COALESCE(ROUND((rp.last_page::FLOAT / NULLIF(rp.total_pages, 0) * 100)::NUMERIC, 2), 0) AS progress
    FROM documents d
    LEFT JOIN reading_progress rp ON d.id = rp.document_id
)";
    if (!searchText.isEmpty()) {
        queryString += " WHERE LOWER(d.title) LIKE :os OR LOWER(d.author) LIKE :os";
    }
    QSqlQuery query(db);
    query.prepare(queryString);
    if (!searchText.isEmpty()) {
        query.bindValue(":os", "%" + searchText + "%");
    }
    query.exec();
    model->setQuery(query);

    if (model->lastError().isValid()) {
        qDebug() << "Ошибка выполнения запроса:" << model->lastError().text();
        QMessageBox::critical(this, "Ошибка", "Не удалось загрузить данные: " + model->lastError().text());
        return;
    }

    model->setHeaderData(0, Qt::Horizontal, tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, tr("Название"));
    model->setHeaderData(2, Qt::Horizontal, tr("Автор"));
    model->setHeaderData(3, Qt::Horizontal, tr("Путь к файлу"));
    model->setHeaderData(4, Qt::Horizontal, tr("Дата добавления"));
    model->setHeaderData(5, Qt::Horizontal, tr("Прогресс (%)"));

    ui->tableView->setModel(model);
    ui->tableView->setColumnHidden(0, true);
    ui->tableView->setColumnHidden(3, true);
    ui->tableView->setColumnHidden(4, true);

    ui->tableView->setItemDelegateForColumn(5, new ProgressBarDelegate(this));


    QHeaderView *header = ui->tableView->horizontalHeader();
    header->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void MainWindow::on_DeleteBtn_clicked()
{
    qDebug() << "Нажата кнопка DeleteBtn";
    // TODO: Реализовать удаление документа
}

void MainWindow::on_OpenBtn_clicked()
{
    qDebug() << "Нажата кнопка OpenBtn";
    qDebug() << "MainWindow видим:" << isVisible();

    // Получаем индекс выбранной строки
    QModelIndexList selectedRows = ui->tableView->selectionModel()->selectedRows();
    qDebug() << "Выбрано строк:" << selectedRows.size();
    if (selectedRows.isEmpty()) {
        QMessageBox::warning(this, "Предупреждение", "Выберите документ для открытия!");
        qDebug() << "Документ не выбран";
        return;
    }

    // Берем первую выбранную строку
    int row = selectedRows.first().row();
    int docId = model->data(model->index(row, 0)).toInt();
    QString filePath = model->data(model->index(row, 3)).toString();
    qDebug() << "Выбран документ: ID =" << docId << ", путь =" << filePath;

    // Проверяем, существует ли файл
    if (!QFileInfo::exists(filePath)) {
        QMessageBox::critical(this, "Ошибка", "Файл не найден: " + filePath);
        qDebug() << "Файл не найден:" << filePath;
        return;
    }

    // Открываем PDF в PdfViewer
    if (pdfViewer) {
        qDebug() << "Закрытие предыдущего PdfViewer";
        pdfViewer->deleteLater();
        pdfViewer = nullptr;
    }

    int lastPage = 0; // Значение по умолчанию
    QSqlQuery q(db);
    QString queryStr = QString(R"(SELECT last_page FROM reading_progress WHERE document_id = %1)").arg(docId);
    if (!q.exec(queryStr)) {
        qDebug() << "Ошибка запроса last_page: " << q.lastError().text();
    } else if (q.next()) {
        lastPage = q.value("last_page").toInt();
        qDebug() << "lastPage получен: page =" << lastPage << ", documentId =" << docId;
    } else {
        qDebug() << "Запись в reading_progress не найдена для documentId =" << docId << ", lastPage = 0";
    }
    try {
        pdfViewer = new PdfViewer(filePath, docId, lastPage, nullptr);
        qDebug() << "PdfViewer создан";

        // Подключаем сигналы
        connect(pdfViewer, &PdfViewer::pageChanged, this, [this, docId](int documentId, int page, int totalPages) {
            qDebug() << "Сигнал pageChanged получен: page =" << page << ", documentId =" << documentId << ", totalPages =" << totalPages;
            if (!db.isOpen()) {
                qDebug() << "База данных закрыта, пытаемся открыть";
                if (!db.open()) {
                    qDebug() << "Не удалось открыть базу данных: " << db.lastError().text();
                    return;
                }
            }
            QSqlQuery query(db);
            QString queryStr = QString(R"(
                INSERT INTO reading_progress (document_id, last_page, total_pages, last_opened)
                VALUES (%1, %2, %3, CURRENT_TIMESTAMP)
                ON CONFLICT ON CONSTRAINT unique_document_id DO UPDATE
                SET last_page = %2, total_pages = %3, last_opened = CURRENT_TIMESTAMP
            )").arg(documentId).arg(page).arg(totalPages);
            if (!query.exec(queryStr)) {
                qDebug() << "Ошибка обновления reading_progress: " << query.lastError().text();
            } else {
                qDebug() << "reading_progress обновлен: page =" << page << ", documentId =" << documentId << ", totalPages =" << totalPages;
            }
            loadData();
        });

        connect(pdfViewer, &PdfViewer::closed, this, [this, docId]() {
            qDebug() << "Сигнал closed получен для documentId:" << docId;
            if (!db.isOpen()) {
                qDebug() << "База данных закрыта, пытаемся открыть";
                if (!db.open()) {
                    qDebug() << "Не удалось открыть базу данных: " << db.lastError().text();
                    return;
                }
            }
            QSqlQuery query(db);
            QString queryStr = QString(R"(
                UPDATE open_stats
                SET close_time = CURRENT_TIMESTAMP
                WHERE id = (
                    SELECT id
                    FROM open_stats
                    WHERE document_id = %1 AND close_time IS NULL
                    ORDER BY open_time DESC
                    FETCH FIRST 1 ROW ONLY
                )
            )").arg(docId);
            if (!query.exec(queryStr)) {
                qDebug() << "Ошибка обновления close_time: " << query.lastError().text();
            } else {
                qDebug() << "close_time обновлен для documentId:" << docId;
            }
            loadData();
            if (pdfViewer) {
                qDebug() << "Удаление PdfViewer через deleteLater";
                pdfViewer->deleteLater();
                pdfViewer = nullptr;
            }
        });

        // Показываем PdfViewer
        pdfViewer->show();
        qDebug() << "PdfViewer отображен";

        // Добавляем запись в open_stats
        if (!db.isOpen()) {
            qDebug() << "База данных закрыта, пытаемся открыть";
            if (!db.open()) {
                qDebug() << "Не удалось открыть базу данных: " << db.lastError().text();
                return;
            }
        }
        QSqlQuery query(db);
        QString queryStr = QString(R"(
            INSERT INTO open_stats (document_id, open_time)
            VALUES (%1, CURRENT_TIMESTAMP)
        )").arg(docId);
        if (!query.exec(queryStr)) {
            qDebug() << "Ошибка добавления в open_stats: " << query.lastError().text();
        } else {
            qDebug() << "open_stats обновлен: documentId =" << docId;
        }

        // Обновляем reading_progress при открытии
        std::unique_ptr<Poppler::Document> document = Poppler::Document::load(filePath);
        int totalPages = (document && !document->isLocked()) ? document->numPages() : 0;
        queryStr = QString(R"(
            INSERT INTO reading_progress (document_id, last_page, total_pages, last_opened)
            VALUES (%1, 0, %2, CURRENT_TIMESTAMP)
            ON CONFLICT ON CONSTRAINT unique_document_id DO UPDATE
            SET last_opened = CURRENT_TIMESTAMP, total_pages = %2
        )").arg(docId).arg(totalPages);
        if (!query.exec(queryStr)) {
            qDebug() << "Ошибка обновления reading_progress при открытии: " << query.lastError().text();
        } else {
            qDebug() << "reading_progress инициализирован: total_pages =" << totalPages << ", documentId =" << docId;
        }

        loadData();
    } catch (const std::exception &e) {
        qDebug() << "Исключение при создании PdfViewer: " << e.what();
        QMessageBox::critical(this, "Ошибка", "Ошибка при открытии PDF: " + QString(e.what()));
        if (pdfViewer) {
            pdfViewer->deleteLater();
            pdfViewer = nullptr;
        }
        return;
    }
}


void MainWindow::on_SearchEdit_textEdited(const QString &arg1)
{
    if (ui->SearchEdit->text().isEmpty()){
        loadData();
    } else {
        on_SearchBtn_clicked();
    }
}


void MainWindow::on_action_csv_triggered()
{

    QString filename = QInputDialog::getText(this, "Введите названия для .csv", "Название");

    qDebug() << "Экспорт данных в" << filename;
    if (!db.isOpen()) {
        qDebug() << "Ошибка: база данных не открыта";
        QMessageBox::critical(this, "Ошибка", "База данных не доступна");
        return;
    }

    QString queryString = R"(
        SELECT
            d.id,
            d.title,
            d.author,
            d.filepath,
            d.date_added,
            COALESCE(ROUND((rp.last_page::FLOAT / NULLIF(rp.total_pages, 0) * 100)::NUMERIC, 2), 0) AS progress
        FROM documents d
        LEFT JOIN reading_progress rp ON d.id = rp.document_id
    )";

    QSqlQuery query(db);
    query.prepare(queryString);
    if (!query.exec()) {
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();
        QMessageBox::critical(this, "Ошибка", "Не удалось выполнить запрос: " + query.lastError().text());
        return;
    }

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Не удалось открыть файл для записи:" << filename;
        QMessageBox::critical(this, "Ошибка", "Не удалось создать файл CSV");
        return;
    }

    QTextStream out(&file);
    //out.setCodec("UTF-8");
    out << "id,title,author,filepath,date_added,progress\n";
    while (query.next()) {
        QStringList row;
        QString title = query.value("title").toString().replace("\"", "\"\"").replace(",", "\\,");
        QString author = query.value("author").toString().replace("\"", "\"\"").replace(",", "\\,");
        if (author.isEmpty()) author = "Unknown"; // Заменяем пустые авторы
        QString filepath = query.value("filepath").toString().replace("\"", "\"\"").replace(",", "\\,").replace(">", "\\>");
        QString date_added = query.value("date_added").toString();
        if (date_added.isEmpty()) date_added = "1970-01-01";
        row << query.value("id").toString()
            << "\"" + title + "\""
            << author // Без кавычек для author
            << "\"" + filepath + "\""
            << date_added // Без кавычек для date_added
            << query.value("progress").toString();
        out << row.join(",") << "\n";
    }
    file.close();
    qDebug() << "Данные экспортированы в" << filename;
}

