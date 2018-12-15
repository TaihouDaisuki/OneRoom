#include "UserView.h"
#include <QPainter>
#include <QStyledItemDelegate>
#include <QStyle>
#include <QEvent>
#include <QDebug>

ItemDelegate::ItemDelegate(QObject *parent) :
	QStyledItemDelegate(parent)
{

}

ItemDelegate::~ItemDelegate()
{

}

void ItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	if (index.isValid())
	{
		
	}
}

QSize ItemDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
	return QSize(160, 60);
}