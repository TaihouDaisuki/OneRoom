#pragma once
#include <QHostAddress>
#include <QTcpSocket>
#include <QDialog>
#include <string.h>
#include "define.h"

class Socket :public QDialog
{
	Q_OBJECT

public:
	Socket(QWidget *parent = 0, Qt::WindowFlags f = 0);
	//~TcpClient();
	std::string ip;
	int port;
	QTcpSocket *tcpSocket;
	int SendMessage(PackageHead &head, const char * data);	// 
	int GetMessage(PackageHead *head, char *data);
	int Connect();
	int DisConnect();
private:

public slots:
	void dataReceived();

signals:
	void getNewmessage(int type, char *info, int len);

};