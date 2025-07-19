#ifndef PDFCOVERDELEGATE_H
#define PDFCOVERDELEGATE_H

#include <QStyledItemDelegate>
#include <poppler/qt6/poppler-qt6.h>
#include <QMap>

class PdfCoverDelegate : public QStyledItemDelegate {
    Q_OBJECT
public:
    explicit PdfCoverDelegate(QObject *parent = nullptr);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,
               const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const override;
private:
    QImage loadPdfCover(const QString &filePath) const;
    mutable QMap<QString, QImage> coverCache;
};

#endif // PDFCOVERDELEGATE_H
