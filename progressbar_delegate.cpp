#include "progressbar_delegate.h"
#include <QPainter>
#include <QProgressBar>
#include <QApplication>

ProgressBarDelegate::ProgressBarDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

void ProgressBarDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                                const QModelIndex &index) const
{
    if (index.column() == 5) { // Столбец progress
        float progress = index.data().toFloat();
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.rect = option.rect;
        progressBarOption.minimum = 0;
        progressBarOption.maximum = 100;
        progressBarOption.progress = static_cast<int>(progress);
        progressBarOption.text = QString("%1%").arg(progress, 0, 'f', 1);
        progressBarOption.textVisible = true;
        progressBarOption.textAlignment = Qt::AlignCenter;

        QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                           &progressBarOption, painter);
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize ProgressBarDelegate::sizeHint(const QStyleOptionViewItem &option,
                                    const QModelIndex &index) const
{
    if (index.column() == 5) {
        return QSize(80, option.rect.height()); // Фиксированная ширина столбца
    }
    return QStyledItemDelegate::sizeHint(option, index);
}
