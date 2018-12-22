#include "Socket.h"

Socket::Socket(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	port = 20610;
	ip = "10.60.102.252";
	tcpSocket = NULL;
	sslSocket = NULL;

	// connect
#ifndef SSL
	connect(tcpSocket, SIGNAL(QAbstractSocket::readyRead()), this, SLOT(dataReceived()));
	connect(tcpSocket, SIGNAL(QAbstractSocket::disconnected()), this, SLOT(socket_disconnected()));
	connect(tcpSocket, SIGNAL(QAbstractSocket::error(QAbstractSocket::SocketError socketError)), this, SLOT(socket_error(QAbstractSocket::SocketError socketError)));
#else
	connect(sslSocket, SIGNAL(QAbstractSocket::readyRead()), this, SLOT(dataReceived()));
	connect(sslSocket, SIGNAL(QAbstractSocket::disconnected()), this, SLOT(socket_disconnected()));
	connect(sslSocket, SIGNAL(QAbstractSocket::error(QAbstractSocket::SocketError socketError)), this, SLOT(socket_error(QAbstractSocket::SocketError socketError)));
#endif
}

int Socket::Send(PackageHead head, const char * data)
{
	/*char* buf;
	memcpy(buf, &head, sizeof(PackageHead));
	
	if (data)
		memcpy(buf + sizeof(PackageHead), data, head.dataLen);*/
	
	tcpSocket->write((char*)&head, sizeof(PackageHead));
	if (data!=NULL)
		tcpSocket->write(data, head.dataLen);
	
	return 0;
}

int Socket::GetMessage(PackageHead *head, char *data)
{
	return 0;
}

int Socket::Connect()
{
#ifndef SSL
	if (tcpSocket == NULL)
		tcpSocket = new QTcpSocket(this);
	//std::cout << "connect to " << this->ip << ":" << this->port << std::endl;
	tcpSocket->connectToHost(ip.c_str(), port);
#else
	if (loadSslFiles())
	{
		sslSocket->setLocalCertificate(s_certificate);
		sslSocket->setPrivateKey(s_privateKey);
		sslSocket->setCaCertificates(s_caCertificates);
		sslSocket->setPeerVerifyMode(QSslSocket::VerifyPeer);
		sslSocket->setPeerVerifyDepth(1);
		sslSocket->connectToHostEncrypted(ip.c_str(), port);
	}
	else
	{
		QMessageBox::warning(this, "SSL File Error", "Load SSL Files failed.");
	}
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
		//std::cout << tcpSocket->bytesAvailable() << std::endl;
		char* head = new char[PACKAGE_HEAD_SIZE];
		tcpSocket->read(head, 8);
		PackageHead temp;
		memcpy(&temp, head, 8);
		
		char* temp_buf = NULL;

		if (temp.dataLen == 0)
			temp_buf = NULL;
		else {
			temp_buf = new char[PACKAGE_DATA_MAX_SIZE];
			tcpSocket->read(temp_buf, temp.dataLen);
		}
		//std::cout << "getNewmessage" << std::endl;
		emit getNewmessage(temp, temp_buf);
	}
}

void Socket::socket_disconnected()
{
	QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("连接已经断开"));
}

void Socket::socket_error(QAbstractSocket::SocketError socketError)
{
#ifndef SSL
	QMessageBox::warning(this, tr("FBI Warning"), tcpSocket->errorString());
#else
	QMessageBox::warning(this, tr("FBI Warning"), sslSocket->errorString());
#endif
}

int Socket::loadSslFiles()
{
	bool openOk = false;
	QFile certFile(QDir::currentPath() + QString(":/ssl/server.crt"));
	openOk = certFile.open(QIODevice::ReadOnly);
	s_certificate = QSslCertificate(certFile.readAll(), QSsl::Der);
	openOk &= !s_certificate.isNull();

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