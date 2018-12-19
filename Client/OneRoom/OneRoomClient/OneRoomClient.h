#pragma once

#include <QtWidgets/QMainWindow>
#include <QTextCodec>
#include "ui_OneRoomClient.h"
#include "UserView.h"
#include "MessageView.h"
#include "define.h"
#include "Socket.h"
#include <qevent.h>
#include "Socket.h"
#include "LoginWindow.h"
#include <qqueue.h>

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
	
	LoginWindow *loginWindow;
private slots:
	void on_sendMsgBtn_clicked();
	void on_sendFileBtn_clicked();
	void on_sendImgBtn_clicked();
	void on_logOutBtn_clicked();
	void on_package_arrived(PackageHead head, char* data);

	void reshow_mainwindow();

public slots:
	void OneRoomClient::getMess(PackageHead head, char *info);

private:
	Ui::OneRoomClientClass ui;
	Socket socket;
	UserInfo currentUser;	// �Լ����û���Ϣ
	QList<UserInfo> userList;	// �����û��б�
	QQueue<Message *> sendMsgQueue;	// ������Ϣ���У����ڷ����ȷ�ϴ���
	void resizeEvent(QResizeEvent *event); // �����¼�����
	bool eventFilter(QObject *obj, QEvent *e);	// testedit���¼�������
	void logout();
};



