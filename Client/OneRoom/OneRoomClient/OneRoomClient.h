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
	void setUserList();

	void handleMessage(Message *message, QListWidgetItem *item, QString text, QString time, Message::User_Type type);
	void handleMessageTime(QString curMsgTime);

private slots:
	void on_sendMsgBtn_clicked();

private:
	Ui::OneRoomClientClass ui;
	// 重载事件函数
	void resizeEvent(QResizeEvent *event);
};
