#ifndef PDFVIEWER_H
#define PDFVIEWER_H

#include <QDialog>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <poppler/qt6/poppler-qt6.h>
#include <QFileInfo>
#include <memory>

class PdfViewer : public QDialog {
    Q_OBJECT
public:
    explicit PdfViewer(const QString &filePath, int documentId, int lastPage, QWidget *parent);
    ~PdfViewer();

signals:
    void pageChanged(int documentId, int page, int totalPages); // Добавляем totalPages в сигнал
    void closed(int documentId);

private:
    QGraphicsView *view;
    QGraphicsScene *scene;
    std::unique_ptr<Poppler::Document> document;
    int currentPage;
    int totalPages;
    int documentId;
    qreal currentScale;
    qreal dpi = 300;

    void loadPage(int page);

private slots:
    void nextPage();
    void prevPage();
    void closeViewer();
    void zoomIn();
    void zoomOut();

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif // PDFVIEWER_H
