#pragma once
#include <qstring.h>
#include <qmetatype.h>
#include <QStyledItemDelegate>
#include <QModelIndex>
#include <QStandardItemModel>

struct UserItem
{
	QString nickName;
	QString userName;
};


class ItemDelegate : public QStyledItemDelegate
{
	Q_OBJECT
signals:

public:
	explicit ItemDelegate(QObject *parent = 0);
	~ItemDelegate();

	//ÖØÐ´ÖØ»­º¯Êý
	void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
	QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;

private:

};
Q_DECLARE_METATYPE(UserItem);