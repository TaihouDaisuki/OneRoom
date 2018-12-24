#include "UserView.h"
#include <QPainter>
#include <QStyle>
#include <QEvent>

UserInfo::UserInfo(const UserInfo &info)
{
	u_nickName = info.u_nickName;
	u_userName = info.u_userName;
	u_loginTime = info.u_loginTime;
	u_lineHeight = info.u_lineHeight;
	u_pixmap = info.u_pixmap;
	u_password = info.u_password;
}

UserInfo::UserInfo(QWidget *parent) : QWidget(parent)
{
	QFont u_font = this->font();
	u_font.setFamily("MicrosoftYaHei");
	u_font.setPointSize(12);
	this->setFont(u_font);

	u_pixmap = QPixmap(":/Resources/Resources/img/Customer Copy.png");	// 头像
}

UserInfo::UserInfo(QString nickName, QString userName, QString loginTime, QString password)
{
	u_nickName = nickName;
	u_userName = userName;
	u_loginTime = loginTime;
	u_password = password;
}

void UserInfo::operator=(const UserInfo &info)
{
	u_nickName = info.u_nickName;
	u_userName = info.u_userName;
	u_loginTime = info.u_loginTime;
	u_lineHeight = info.u_lineHeight;
	u_pixmap = info.u_pixmap;
	u_password = info.u_password;
}

void UserInfo::setInfo(QString nickName, QString userName, QString loginTime, QString password)
{
	u_nickName = nickName;
	u_userName = userName;
	u_loginTime = loginTime;
	u_password = password;

	this->update();
}

QSize UserInfo::rectSize()
{
	QFontMetricsF fm(this->font());
	u_lineHeight = fm.lineSpacing();
	int height = 60;
	int iconWH = 40;
	int iconSpaceW = 20;
	int iconRectW = 5;
	int iconTMPH = 10;
	u_nickNameW = 100;
	u_iconLeftRect = QRect(iconSpaceW, iconTMPH, iconWH, iconWH);
	u_nickNameRect.setRect(iconWH + iconSpaceW + iconRectW, u_lineHeight / 2, u_nickNameW, height - u_lineHeight);
	return QSize(this->width(), height);
}

void UserInfo::paintEvent(QPaintEvent *event)
{
	Q_UNUSED(event);

	QPainter painter(this);
	painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);//消锯齿
	painter.setPen(Qt::NoPen);
	painter.setBrush(QBrush(Qt::gray));
	
	// 头像
	painter.drawPixmap(u_iconLeftRect, u_pixmap);
	// 文字
	QPen penText;
	penText.setColor(Qt::GlobalColor::white);
	painter.setPen(penText);
	QTextOption option(Qt::AlignLeft | Qt::AlignVCenter);
	option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
	painter.setFont(this->font());
	painter.drawText(u_nickNameRect, u_nickName, option);
	
	// debug
	//painter.setBrush(Qt::NoBrush);
	//painter.drawRoundedRect(u_iconRightRect, 2, 2);
	//painter.setBrush(Qt::NoBrush);
	//painter.drawRoundedRect(u_nickNameRect, 2, 2);
	//painter.setBrush(Qt::NoBrush);
	//painter.drawRoundedRect(this->rect(), 2, 2);
}