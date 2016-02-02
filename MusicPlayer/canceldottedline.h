#ifndef CANCELDOTTEDLINE
#define CANCELDOTTEDLINE

#include <QStyleOptionViewItem>
#include <QStyledItemDelegate>
#include <QTableView>
#include <QPen>
#include "musicplaylist.h"

class CancelDottedLine : public QStyledItemDelegate
{
    //去掉注释会有莫名错误
    //Q_OBJECT

    //设置为友元函数来访问canceldottedline的私有成员
    //friend void MusicPlaylist::setupPlaylist();

public:
    CancelDottedLine(QTableView* tableView);
protected:
    void paint(QPainter* painter,const QStyleOptionViewItem& option,const QModelIndex& index) const;
private:
    QPen         pen;
    QTableView*  view;
};

#endif // CANCELDOTTEDLINE

