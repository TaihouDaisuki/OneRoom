#include "Socket.h"

Socket::Socket(QWidget *parent, Qt::WindowFlags f) : QDialog(parent, f)
{
	this->port = 4000;
	this->ip = "119.28.139.244";
	this->tcpSocket = NULL;
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
	if (this->tcpSocket == NULL)
		this->tcpSocket = new QTcpSocket(this);
	//std::cout << "connect to " << this->ip << ":" << this->port << std::endl;
	tcpSocket->connectToHost(ip.c_str(), port);

	connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
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