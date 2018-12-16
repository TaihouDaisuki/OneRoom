#pragma once

#include <QHostAddress>
#include <QTcpSocket>
#include <QDialog>
#include <string>
#include <iostream>
#include <string.h>

struct Package {
	unsigned char IsData;
	unsigned char Type;
	unsigned char IsCut;
	unsigned char Seq;
	int DataLen;
};

const Package SingleText = {
	1,0x00,0,0,0
};

const Package AllText = {
	1,0x01,0,0,0
};

const Package GroupText = {
	1,0x02,0,0,0
};


class TcpClient:public QDialog
{
	Q_OBJECT

public:
	TcpClient(QWidget *parent = 0, Qt::WindowFlags f = 0);
	//~TcpClient();
	std::string ip;
	int port;
	QTcpSocket *tcpSocket;
	int OneRoomSendMessage(Package pack, const char * data);
	int GetMessage(Package *pack, char *data);
	int Connect();
	int DisConnect();
private:

public slots:
	void dataReceived();

signals:
	void getNewmessage(int type,char *info,int len);

};



