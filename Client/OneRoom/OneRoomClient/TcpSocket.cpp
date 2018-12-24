#include "TcpSocket.h"


TcpClient::TcpClient(QWidget *parent, Qt::WindowFlags f)
	: QDialog(parent, f)
{
	this->port = 4000;
	this->ip = "119.28.139.244";
	this->tcpSocket = NULL;
}

int TcpClient::OneRoomSendMessage(PackageHead pack, const char * data)
{
	char buf[1500];
	memcpy(buf, &pack, 8);
	memcpy(buf + 8, data, pack.DataLen);
	tcpSocket->write(buf, pack.DataLen + 8);
	return 0;
}



int TcpClient::GetMessage(PackageHead *pack, char *data)
{
	return 0;
}


int TcpClient::Connect()
{
	if(this->tcpSocket==NULL)
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
		char pack[8];
		tcpSocket->read(pack, 8);
		PackageHead temp;
		memcpy(&temp, pack, 8);
		std::cout << "IsData:" << (int)temp.IsData << std::endl;
		std::cout << "Type:" << (int)temp.Type << std::endl;
		std::cout << "IsCut:" << (int)temp.IsCut << std::endl;
		std::cout << "Seq:" << (int)temp.Seq << std::endl;
		std::cout << "Datalen:" << (int)temp.DataLen << std::endl;
		char temp_buf[1500];
		tcpSocket->read(temp_buf, temp.DataLen);
		std::cout << "getNewmessage" << std::endl;

		if (temp.IsData == 1)
		{
			switch (temp.Type)
			{
			case	_MESSAGE_IS_SINGLE_TEXT:
				emit getNewmessage(SINGLE_TEXT, temp_buf, temp.DataLen);
				break;
			default:
				emit getNewmessage(SINGLE_TEXT, temp_buf, temp.DataLen);
				break;
			}
		}
		else
		{
			switch (temp.Type)
			{
			case _SERVER_RETURN_CORRECT:
				emit getNewmessage(_MESSAGE_IS_CORRECT, temp_buf, temp.DataLen);
				break;
			case _SERVER_ERROR_CONNECT:
				emit getNewmessage(_MESSAGE_IS_ERROR_CON, temp_buf, temp.DataLen);
				break;
			case _SERVER_ERROR_DISCONNECT:
				emit getNewmessage(_MESSAGE_IS_ERROR_DIS, temp_buf, temp.DataLen);
				break;
			default:
				emit getNewmessage(_MESSAGE_IS_CORRECT, temp_buf, temp.DataLen);
				break;
			}
		}
	}
}

