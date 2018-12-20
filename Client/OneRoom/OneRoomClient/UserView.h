#pragma once
#include <qstring.h>
#include <qwidget.h>

class UserInfo : public QWidget
{
	Q_OBJECT
public:
	UserInfo(const UserInfo &info);
	explicit UserInfo(QWidget *parent = nullptr);
	UserInfo(QString nickName, QString userName, QString loginTime, QString password = QString::null);
	void operator=(const UserInfo &info);	
	void setInfo(QString nickName, QString userName, QString loginTime, QString password = QString::null);
	QSize rectSize();
	inline QString nickName() { return u_nickName; }
	inline QString userName() { return u_userName; }
	inline QString loginTime() { return u_loginTime; }
	inline QString password() { return u_password; }

protected:
	void paintEvent(QPaintEvent *event);

private:
	QString u_nickName;
	QString u_userName;
	QString u_loginTime;
	QString u_password;

	int u_nickNameW;
	int u_userNameW;
	int u_loginTimeW;
	int u_lineHeight;
	
	QRect u_nickNameRect;
	QRect u_iconLeftRect;
	QRect u_iconRightRect;
	QPixmap u_pixmap;
};

