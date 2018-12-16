#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_OneRoomClient.h"
#include "UserView.h"
#include "MessageView.h"

class OneRoomClient : public QMainWindow
{
	Q_OBJECT

public:
	OneRoomClient(QWidget *parent = Q_NULLPTR);
	void updateUserList();
	void handleUserinfo(UserInfo *userInfo, QListWidgetItem *item, QString nickName, QString userName, QString loginTime);

	void handleMessage(Message *message, QListWidgetItem *item, QString text, QString time, Message::User_Type type);
	void handleMessageTime(QString curMsgTime);

private slots:
	void on_sendMsgBtn_clicked();
	void on_newMsg_come(QString msg, QString sendTime);

private:
	Ui::OneRoomClientClass ui;
	QList<UserInfo> userList;	// 在线用户列表
	// 重载事件函数
	void resizeEvent(QResizeEvent *event);
};
