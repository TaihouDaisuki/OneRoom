#pragma once

#include <QtWidgets/QMainWindow>
#include <QTextCodec>
#include <qqueue.h>
#include <qevent.h>

#include "ui_OneRoomClient.h"
#include "UserView.h"
#include "MessageView.h"
#include "define.h"
#include "Socket.h"
#include "Socket.h"
#include "LoginWindow.h"
#include "SettingBoard.h"

class OneRoomClient : public QMainWindow
{
	Q_OBJECT

public:
	OneRoomClient(QWidget *parent = Q_NULLPTR);
	void updateUserList();
	void handleUserinfo(UserInfo *userInfo, QListWidgetItem *item, QString nickName, QString userName, QString loginTime);

	void handleMessage(Message *message, QListWidgetItem *item, QString text, QString time, Message::User_Type type, Message::Msg_Type msgType = Message::Msg_Text, QString imgPath = QString::null);
	void handleMessageTime(QString curMsgTime);
	int addTargetUserData(QList<QListWidgetItem *> &itemList, char* const data, int nCount);

private slots:
	void on_sendMsgBtn_clicked();
	void on_sendFileBtn_clicked();
	void on_sendImgBtn_clicked();
	void on_logOutBtn_clicked();
	void on_package_arrived(PackageHead head, char* const data);
	void on_settingBtn_clicked();
	void on_msgHistoryBtn_clicked();
	void reshow_mainwindow(QString userName, QString password, int histroyListNum);
	void handle_socket_error(QString errorMsg);
	void send_history_num_setting(int num);
	void send_password_setting(QString password);
	void on_userListWidget_itemDoubleClicked(QListWidgetItem *item);
signals:
	void change_password_result(int result);

private:
	Ui::OneRoomClientClass ui;
	
	Socket socket;
	UserInfo currentUser;	// 自己的用户信息
	QList<UserInfo> userList;	// 在线用户列表
	//QList<QList<Message*>*> messageList;	// 每个在线用户有一个消息列表
	QQueue<Message *> sendMsgQueue;	// 发送消息队列，用于服务端确认处理
	LoginWindow *loginWindow;
	SettingBoard *settingBoard;

	void resizeEvent(QResizeEvent *event); // 重载事件函数
	bool eventFilter(QObject *obj, QEvent *e);	// testedit用事件过滤器
	void setButtonDisable();
	void setButtonEnable();
	void logout();
};



 