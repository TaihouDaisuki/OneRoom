#include "TcpSocket.h"

TcpClient::TcpClient(QWidget *parent, Qt::WindowFlags f)
	: QDialog(parent, f)
{
	this->port = 4000;
	this->ip = "119.28.139.244";
}

int TcpClient::OneRoomSendMessage(Package pack, const char * data)
{
	char buf[1500];
	memcpy(buf, &pack, 64);
	memcpy(buf + 64, data, pack.DataLen);
	std::cout << tcpSocket->write(buf, pack.DataLen + 64) << endl;
	return 0;
}



int TcpClient::GetMessage(Package *pack, char *data)
{
	return 0;
}


int TcpClient::Connect()
{
	this->tcpSocket = new QTcpSocket(this);
	std::cout << "connect to " << this->ip << ":" << this->port << std::endl;
	tcpSocket->connectToHost(ip.c_str(), port);

	// connect(tcpSocket,SIGNAL(connected()),this,SLOT(slotConnected()));
	// connect(tcpSocket,SIGNAL(disconnected()),this,SLOT(slotDisconnected()));
	connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(dataReceived()));
	return 0;
}


int TcpClient::DisConnect()
{
	tcpSocket->close();
	return 0;
}


void TcpClient::dataReceived()
{
	while (tcpSocket->bytesAvailable()>0)
	{
		std::cout << tcpSocket->bytesAvailable() << std::endl;
		char pack[64];
		tcpSocket->read(pack, 64);
		Package temp;
		memcpy(&temp, pack, 64);
		std::cout << "IsData:" << (int)temp.IsData << std::endl;
		std::cout << "Type:" << (int)temp.Type << std::endl;
		std::cout << "IsCut:" << (int)temp.IsCut << std::endl;
		std::cout << "Seq:" << (int)temp.Seq << std::endl;
		std::cout << "Datalen:" << (int)temp.DataLen << std::endl;
		char temp_buf[1500];
		tcpSocket->read(temp_buf, temp.DataLen);
		std::cout << "getNewmessage" << std::endl;
		emit getNewmessage(1, temp_buf, temp.DataLen);
	}
}

