#include "pdfcoverdelegate.h"
#include <QPainter>
#include <QApplication>
#include <QFontMetrics>
#include <QDebug>

PdfCoverDelegate::PdfCoverDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

QImage PdfCoverDelegate::loadPdfCover(const QString &filePath) const
{
    if (coverCache.contains(filePath)) {
        qDebug() << "Использована кэшированная обложка:" << filePath;
        return coverCache[filePath];
    }
    std::unique_ptr<Poppler::Document> document(Poppler::Document::load(filePath));
    if (!document || document->isLocked()) {
        qDebug() << "Не удалось загрузить PDF:" << filePath;
        return QImage();
    }
    std::unique_ptr<Poppler::Page> page(document->page(0));
    if (!page) {
        qDebug() << "Не удалось загрузить первую страницу:" << filePath;
        return QImage();
    }
    QImage image = page->renderToImage(72.0, 72.0, -1, -1, -1, -1);
    if (image.isNull()) {
        qDebug() << "Не удалось отрендерить обложку:" << filePath;
        return QImage();
    }
    // Масштабируем по высоте строки (40 - 4 = 36 пикселей для изображения)
    QImage scaledImage = image.scaledToHeight(36, Qt::SmoothTransformation);
    coverCache[filePath] = scaledImage;
    return scaledImage;
}

void PdfCoverDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    if (index.column() == 1) {
        QString filePath = index.model()->index(index.row(), 3).data().toString();
        QString title = index.data().toString();

        painter->save();

        QStyleOptionViewItem opt = option;
        initStyleOption(&opt, index);
        QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();
        style->drawPrimitive(QStyle::PE_PanelItemViewItem, &opt, painter, opt.widget);

        QRect coverRect = option.rect.adjusted(2, 2, -2, -2);
        coverRect.setWidth(36); // Ширина подстраивается под масштабированную обложку
        QRect textRect = option.rect.adjusted(40, 2, -2, -2);

        QImage cover = loadPdfCover(filePath);
        if (!cover.isNull()) {
            int coverY = option.rect.top() + (option.rect.height() - cover.height()) / 2;
            painter->drawImage(QPoint(coverRect.left(), coverY), cover);
        } else {
            painter->drawText(coverRect, Qt::AlignCenter, "No Cover");
        }

        painter->drawText(textRect, Qt::AlignVCenter | Qt::TextWordWrap, title);

        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize PdfCoverDelegate::sizeHint(const QStyleOptionViewItem &option,
                                 const QModelIndex &index) const
{
    if (index.column() == 1) {
        return QSize(200, 40); // Фиксированная высота строки
    }
    return QStyledItemDelegate::sizeHint(option, index);
}
