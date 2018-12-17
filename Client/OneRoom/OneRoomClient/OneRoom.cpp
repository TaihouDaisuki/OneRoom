#include "OneRoom.h"

OneRoom::OneRoom(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	//setAttribute(Qt::WA_TranslucentBackground);//背景透明
    //setWindowOpacity(0.9);
	QImage image;   
	image.load(":/Resources/Resources/img/oneroom.jpg");   
	image = image.scaled(120,120);
	ui.label->clear();    
	ui.label->setPixmap(QPixmap::fromImage(image));
	this->setStyleSheet("background: rgb(33,33,33);border-width:0;border-style:outset");
	//this->setStyleSheet(" background:rgba(51, 51, 51, .6); webkit - filter: blur(10px);-moz - filter: blur(10px);-ms - filter: blur(10px);filter: blur(10px);");
	ui.pushButton->setStyleSheet("background: rgb(117,117,117);border-width:0;border-style:outset");
	ui.lineEdit->setStyleSheet("border:1px solid grey;color:white");
	ui.lineEdit_2->setStyleSheet("border:1px solid grey;color:white");
	ui.lineEdit_2->setEchoMode(QLineEdit::Password);




}


void OneRoom::on_pushButton_clicked() 
{
	tcpclient->Connect();
	QString username = ui.lineEdit->text();
	QString password = ui.lineEdit_2->text();
	if (username.toLocal8Bit().length() > 20|| password.toLocal8Bit().length()>20)
	{
		QMessageBox::critical(0,
			"critical message", QString::fromLocal8Bit("用户名或密码错误，请重新输入"),
			QMessageBox::Ok | QMessageBox::Default,
			QMessageBox::Cancel | QMessageBox::Escape, 0);
		ui.lineEdit->clear();
		ui.lineEdit_2->clear();
		return;
	}

	PackageHead temp = SendTest;
	temp.Type = _SERVER_RETURN_CORRECT;
	temp.DataLen = 40;
	char ch[40];
	memset(ch, 0, sizeof(ch));
	memcpy(ch, username.toLocal8Bit(),username.toLocal8Bit().length());
	memcpy(ch+20, username.toLocal8Bit(), username.toLocal8Bit().length());



	this->tcpclient->OneRoomSendMessage(temp, ch);
	ui.pushButton->setEnabled(false);
	return;
}



void OneRoom::ReceivePack(int value, char *info, int len)
{
	if (value == _MESSAGE_IS_CORRECT)
	{
		this->hide();
		emit sendsignal();
	}
	else if (value == _MESSAGE_IS_ERROR_CON)
	{
		if (info[0] == 0x00)
		{
			;
		}//发送错误但保持连接
	}
	else if (value == _MESSAGE_IS_ERROR_DIS)
	{
		this->tcpclient->DisConnect();;//发送错误且断开连接
		ui.pushButton->setEnabled(true);

	}
}