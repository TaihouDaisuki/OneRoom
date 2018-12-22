#pragma once
#include <QWidget>

class QPaintEvent;
class QPainter;
class QLabel;
class QMovie;

class Message : public QWidget
{
	Q_OBJECT
public:
	explicit Message(QWidget *parent = nullptr);	// explicit 禁用隐式类型转换

	enum User_Type {
		User_System,//系统
		User_Me,    //自己
		User_He,   //用户
		User_Time,  //时间
	};

	enum Msg_Type {
		Msg_Text,	// 文本消息
		Msg_Img,	// 图片消息
		Msg_File	// 文字消息
	};

	void setTextSuccess();
	void setText(QString text, QString time, QSize allSize, User_Type userType, Msg_Type msgType = Msg_Text, QString imgPath = QString::null);
	void setUserName(QString from, QString to) 
	{
		m_fromUserName = from;
		m_toUserName = to;
	}

	QSize getRealString(QString src);
	QSize fontRect(QString str);

	inline QString text() { return m_msg; }
	inline QString time() { return m_time; }
	inline User_Type userType() { return m_userType; }
	inline Msg_Type msgType() { return m_msgType; }
protected:
	void paintEvent(QPaintEvent *event);
private:
	QString m_msg;
	QString m_time;
	QString m_curTime;
	QString m_fromUserName;
	QString m_toUserName;

	QString m_imgPath;

	QSize m_allSize;
	User_Type m_userType = User_System;
	Msg_Type m_msgType = Msg_Text;
	int m_kuangWidth;
	int m_textWidth;
	int m_spaceWid;
	int m_lineHeight;

	QRect m_iconLeftRect;
	QRect m_iconRightRect;
	QRect m_sanjiaoLeftRect;
	QRect m_sanjiaoRightRect;
	QRect m_kuangLeftRect;
	QRect m_kuangRightRect;
	QRect m_textLeftRect;
	QRect m_textRightRect;
	QPixmap m_leftPixmap;
	QPixmap m_rightPixmap;
	QLabel* m_loading = Q_NULLPTR;
	QMovie* m_loadingMovie = Q_NULLPTR;
	bool m_isSending = false;
};
