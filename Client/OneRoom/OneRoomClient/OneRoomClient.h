#pragma once

#include <QtWidgets/QMainWindow>
#include <QTextCodec>
#include "ui_OneRoomClient.h"
#include "UserView.h"
#include "MessageView.h"
#include "define.h"

class OneRoomClient : public QMainWindow
{
	Q_OBJECT

public:
	OneRoomClient(QWidget *parent = Q_NULLPTR);
	void updateUserList();
	void handleUserinfo(UserInfo *userInfo, QListWidgetItem *item, QString nickName, QString userName, QString loginTime);

	void handleMessage(Message *message, QListWidgetItem *item, QString text, QString time, Message::User_Type type);
	void handleMessageTime(QString curMsgTime);
	int addTargetUserData(QList<QListWidgetItem *> &itemList, char* data, int nCount);

private slots:
	void on_sendMsgBtn_clicked();
	void on_newMsg_come(QString msg, QString sendTime);

private:
	Ui::OneRoomClientClass ui;
	Socket socket;
	QList<UserInfo *> userList;	// 在线用户列表
	UserInfo currentUser;	// 自己的用户信息
	void resizeEvent(QResizeEvent *event); // 重载事件函数
};
