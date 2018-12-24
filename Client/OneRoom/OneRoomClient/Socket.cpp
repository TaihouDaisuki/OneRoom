#include "Socket.h"

Socket::Socket(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	port = 20610;
	ip = "10.60.102.252";
	tcpSocket = NULL;
	sslSocket = NULL;

	headFlag = false;
}

int Socket::Send(PackageHead head, const char * data)
{
	int len = head.dataLen;
	head.isCut = htonl(head.isCut);
	head.seq = htonl(head.seq);
	head.dataLen = htonl(head.dataLen);

	tcpSocket->write((char*)&head, sizeof(PackageHead));
	if (data!=NULL)
		tcpSocket->write(data, len);
	
	return 0;
}

int Socket::Connect()
{
#ifndef SSL
	if (tcpSocket == NULL)
		tcpSocket = new QTcpSocket(this);
	//std::cout << "connect to " << this->ip << ":" << this->port << std::endl;
	tcpSocket->connectToHost(ip.c_str(), port);
	connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
	connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(socket_disconnected()));
	connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError socketError)), this, SLOT(socket_error(QAbstractSocket::SocketError socketError)));
#else
	if (loadSslFiles())
	{
		sslSocket->setCaCertificates(s_caCertificates);
		sslSocket->setPeerVerifyMode(QSslSocket::VerifyPeer);
		sslSocket->setPeerVerifyDepth(1);
		sslSocket->connectToHostEncrypted(ip.c_str(), port);
	}
	else
	{
		QMessageBox::warning(this, "SSL File Error", "Load SSL Files failed.");
	}
	connect(sslSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
	connect(sslSocket, SIGNAL(disconnected()), this, SLOT(socket_disconnected()));
	connect(sslSocket, SIGNAL(error(QAbstractSocket::SocketError socketError)), this, SLOT(socket_error(QAbstractSocket::SocketError socketError)));

#endif
	return 0;
}

int Socket::DisConnect()
{
	tcpSocket->close();
	return 0;
}

void Socket::dataReceived()
{
	while (tcpSocket->bytesAvailable() > 0)
	{
		if (headFlag == false) {
			tcpSocket->read((char*)&head, sizeof(PackageHead));
			head.dataLen = ntohl(head.dataLen);
			head.isCut = ntohl(head.isCut);
			head.seq = ntohl(head.seq);
			headFlag = true;
			if (head.dataLen == 0)
			{
				emit getNewmessage(head, NULL);
			}
		}
		int able = tcpSocket->bytesAvailable();
		if (tcpSocket->bytesAvailable() >= head.dataLen) {
			this->buff = new char[PACKAGE_DATA_MAX_SIZE];
			tcpSocket->read(buff, head.dataLen);
			headFlag = false;
			emit getNewmessage(head, buff);
		}
		else
			;
	}
}

void Socket::socket_disconnected()
{
	emit sock_error_occurred(QString::fromLocal8Bit("连接已经断开"));
}

void Socket::socket_error(QAbstractSocket::SocketError socketError)
{
#ifndef SSL
	emit sock_error_occurred(tcpSocket->errorString());
#else
	emit sock_error_occurred(sslSocket->errorString());
#endif
}

int Socket::loadSslFiles()
{
	bool openOk = false;
	QFile certFile(QDir::currentPath() + QString(":/ssl/server.crt"));
	openOk = certFile.open(QIODevice::ReadOnly);
	s_caCertificate = QSslCertificate(certFile.readAll(), QSsl::Der);
	openOk &= !s_caCertificate.isNull();

	QFile keyFile(QDir::currentPath() + QString(":/ssl/ca.key"));
	openOk &= keyFile.open(QIODevice::ReadOnly);
	s_privateKey = QSslKey(keyFile.readAll(), QSsl::Rsa, QSsl::Pem, QSsl::PrivateKey);
	openOk &= !s_privateKey.isNull();

	//QFile peerFile(QDir::currentPath() + QString("/sslCert/cert.pem"));
	//openOk &= peerFile.open(QIODevice::ReadOnly);
	//QSslCertificate peerCert(peerFile.readAll(), QSsl::Pem);
	//bool peerCertValid = !peerCert.isNull();
	//openOk &= peerCertValid;

	//QList<QSslCertificate> caCerts;
	//caCerts << peerCert;
	//s_caCertificates = caCerts;

	return openOk;

}