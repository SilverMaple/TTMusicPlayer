#include <QStyle>
#include <QStyledItemDelegate>

#include <QPainter>
#include "canceldottedline.h"

CancelDottedLine::CancelDottedLine(QTableView* tableView)
{
    int gridHint = tableView->style()->styleHint(QStyle::SH_Table_GridLineColor, new QStyleOptionViewItemV4());
    QColor gridColor = static_cast<QRgb>(gridHint);
    pen = QPen(gridColor, 0, tableView->gridStyle());
    view = tableView;
}

void CancelDottedLine::paint(QPainter* painter, const QStyleOptionViewItem& option,const QModelIndex& index)const
{
    QStyleOptionViewItem itemOption(option);
    if(itemOption.state & QStyle::State_HasFocus)
        itemOption.state = itemOption.state ^ QStyle::State_HasFocus;
    QStyledItemDelegate::paint(painter,itemOption,index);
    QPen oldPen = painter->pen();
    painter->setPen(pen);
    //painter->drawLine(option.rect.topRight(),option.rect.bottomRight());
    painter->drawLine(itemOption.rect.bottomLeft(),itemOption.rect.bottomRight());
    painter->setPen(oldPen);
}
