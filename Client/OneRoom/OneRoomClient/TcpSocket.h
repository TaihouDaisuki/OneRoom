#pragma once

#include <QHostAddress>
#include <QTcpSocket>
#include <QDialog>
#include <string>
#include <iostream>
#include <string.h>
#include "Define.h"


class TcpClient:public QDialog
{
	Q_OBJECT

public:
	TcpClient(QWidget *parent = 0, Qt::WindowFlags f = 0);
	//~TcpClient();
	std::string ip;
	int port;
	QTcpSocket *tcpSocket;
	int OneRoomSendMessage(PackageHead pack, const char * data);
	int GetMessage(PackageHead *pack, char *data);
	int Connect();
	int DisConnect();
private:

public slots:
	void dataReceived();

signals:
	void getNewmessage(int type,char *info,int len);

};



