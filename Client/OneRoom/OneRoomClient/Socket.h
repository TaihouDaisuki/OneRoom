#pragma once
#include <QHostAddress>
#include <QTcpSocket>
#include <QDialog>
#include <qsslsocket.h>
#include <qsslkey.h>
#include <qfile.h>
#include <qdir.h>
#include <iostream>
#include <string.h>
#include "Define.h"
#include <qmessagebox.h>

class Socket :public QDialog
{
	Q_OBJECT

public:
	Socket(QWidget *parent = 0, Qt::WindowFlags f = 0);
	//~TcpClient();
	std::string ip;
	int port;
	QTcpSocket *tcpSocket;
	QSslSocket *sslSocket;
	int Send(PackageHead head, const char * data);	// 
	int GetMessage(PackageHead *head, char *data);
	int Connect();
	int DisConnect();
private:
	int loadSslFiles();
	/* ssl */
	QSslCertificate s_caCertificate;
	QSslKey s_privateKey;

public slots:
	void dataReceived();
	void socket_disconnected();
	void socket_error(QAbstractSocket::SocketError socketError);

signals:
	void getNewmessage(PackageHead type, char *info);

};