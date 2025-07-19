#include "pdfviewer.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QDebug>
#include <QTimer>
#include <QToolBar>
#include <QToolButton>

PdfViewer::PdfViewer(const QString &filePath, int documentId, int lastPage, QWidget *parent)
    : QDialog(parent, Qt::Window),
    currentPage(0),
    totalPages(0),
    documentId(documentId)
{
    qDebug() << "Создание PdfViewer для файла:" << filePath << ", documentId:" << documentId;

    // Проверяем, существует ли файл
    if (!QFileInfo::exists(filePath)) {
        qDebug() << "Файл не существует:" << filePath;
        QMessageBox::critical(this, "Ошибка", "Файл не существует: " + filePath);
        reject();
        return;
    }

    // Инициализация Poppler
    try {
        document = Poppler::Document::load(filePath);
        if (!document || document->isLocked()) {
            qDebug() << "Ошибка загрузки PDF или файл заблокирован:" << filePath;
            QMessageBox::critical(this, "Ошибка", "Не удалось открыть PDF-файл!");
            document.reset();
            reject();
            return;
        }
    } catch (const std::exception &e) {
        qDebug() << "Исключение при загрузке PDF:" << e.what();
        QMessageBox::critical(this, "Ошибка", "Ошибка при загрузке PDF: " + QString(e.what()));
        document.reset();
        reject();
        return;
    }

    totalPages = document->numPages();
    if (totalPages <= 0) {
        qDebug() << "PDF пуст или поврежден:" << filePath;
        QMessageBox::critical(this, "Ошибка", "PDF-файл пуст или поврежден!");
        document.reset();
        reject();
        return;
    }

    qDebug() << "PDF успешно загружен, страниц:" << totalPages;

    currentScale = 1.0;

    document->setRenderHint(Poppler::Document::Antialiasing, true);
    document->setRenderHint(Poppler::Document::TextAntialiasing, true);

    // Настройка интерфейса
    QToolBar *toolBar = new QToolBar(this);
    QAction *zoomInAction = new QAction("+", this);
    QAction *zoomOutAction = new QAction("-", this);
    QAction *prevAction = new QAction("<", this);
    QAction *nextAction = new QAction(">", this);
    QAction *closeAction = new QAction("x", this);

    closeAction->setToolTip(QString("Close"));
    nextAction->setToolTip(QString("Next Page"));
    prevAction->setToolTip(QString("Previous Page"));
    zoomInAction->setToolTip(QString("Zoon In"));
    zoomOutAction->setToolTip(QString("Zoon Out"));

    toolBar->addAction(zoomInAction);
    toolBar->addAction(zoomOutAction);
    toolBar->addSeparator();
    toolBar->addAction(prevAction);
    toolBar->addAction(nextAction);
    toolBar->addAction(closeAction);

    view = new QGraphicsView(this);
    view->setRenderHint(QPainter::Antialiasing, true);
    view->setRenderHint(QPainter::SmoothPixmapTransform, true);

    scene = new QGraphicsScene(this);
    view->setScene(scene);

    // QPushButton *prevButton = new QPushButton("Предыдущая страница", this);
    // QPushButton *nextButton = new QPushButton("Следующая страница", this);
    // QPushButton *closeButton = new QPushButton("Закрыть", this);

    connect(prevAction, &QAction::triggered, this, &PdfViewer::prevPage);
    connect(nextAction, &QAction::triggered, this, &PdfViewer::nextPage);
    connect(closeAction, &QAction::triggered, this, &PdfViewer::closeViewer);
    connect(zoomInAction, &QAction::triggered, this, &PdfViewer::zoomIn);
    connect(zoomOutAction, &QAction::triggered, this, &PdfViewer::zoomOut);

    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(view);
    // layout->addWidget(prevButton);
    // layout->addWidget(nextButton);
    // layout->addWidget(closeButton);
    layout->addWidget(toolBar);
    setLayout(layout);

    resize(900, 700);
    setWindowTitle("PDF Viewer");
    qDebug() << "PdfViewer интерфейс настроен";

    currentPage= lastPage;
    loadPage(currentPage);
}

PdfViewer::~PdfViewer()
{
    qDebug() << "Уничтожение PdfViewer для documentId:" << documentId;
    emit closed(documentId);
}

void PdfViewer::loadPage(int page)
{
    if (!document || page < 0 || page >= totalPages) {
        qDebug() << "Ошибка загрузки страницы:" << page;
        return;
    }

    currentPage = page;
    try {
        std::unique_ptr<Poppler::Page> pdfPage(document->page(page));
        if (!pdfPage) {
            qDebug() << "Не удалось загрузить страницу:" << page;
            return;
        }

        QImage image = pdfPage->renderToImage(dpi, dpi);
        if (image.isNull()) {
            qDebug() << "Ошибка рендеринга страницы:" << page;
            return;
        }

        scene->clear();
        scene->addPixmap(QPixmap::fromImage(image));
        scene->setSceneRect(QRectF(0, 0, image.width(), image.height()));

        qDebug() << "Размер изображения:" << image.size();
        qDebug() << "SceneRect перед fitInView:" << scene->sceneRect();
        qDebug() << "Масштаб QGraphicsView:" << view->transform().m11() << "x" << view->transform().m22();

        view->resetTransform();
        QTimer::singleShot(0, this, [this]() {
            view->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
            view->viewport()->update();
            qDebug() << "Масштаб QGraphicsView после отложенного fitInView:" << view->transform().m11() << "x" << view->transform().m22();
        });
        qDebug() << "Страница загружена: documentId =" << documentId << ", page =" << page;
        qDebug() << "Размер изображения страницы" << page << ":" << image.size();
        qDebug() << "Масштаб QGraphicsView:" << view->transform().m11() << "x" << view->transform().m22();

        emit pageChanged(documentId, currentPage, totalPages); // Передаем totalPages
    } catch (const std::exception &e) {
        qDebug() << "Исключение при рендеринге страницы:" << e.what();
    }
}

void PdfViewer::nextPage()
{
    if (currentPage < totalPages - 1) {
        qDebug() << "Переход на следующую страницу: " << (currentPage + 1);
        loadPage(currentPage + 1);
    }
}

void PdfViewer::prevPage()
{
    if (currentPage > 0) {
        qDebug() << "Переход на предыдущую страницу: " << (currentPage - 1);
        loadPage(currentPage - 1);
    }
}

void PdfViewer::zoomIn()
{
    currentScale *= 1.2;
    if (currentScale > 5.0) currentScale = 5.0;
    view->resetTransform();
    view->scale(currentScale, currentScale);
    view->viewport()->update();
}

void PdfViewer::zoomOut()
{
    currentScale /= 1.2;
    if (currentScale < 0.2) currentScale = 0.2;
    view->resetTransform();
    view->scale(currentScale, currentScale);
    view->viewport()->update();
}

void PdfViewer::closeViewer()
{
    qDebug() << "Закрытие PdfViewer через кнопку для documentId:" << documentId;
    emit closed(documentId);
    reject(); // Используем reject для закрытия диалога
}

void PdfViewer::closeEvent(QCloseEvent *event)
{
    qDebug() << "Закрытие PdfViewer через closeEvent для documentId:" << documentId;
    emit closed(documentId);
    QDialog::closeEvent(event);
}
