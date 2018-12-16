#include "OneRoom.h"

OneRoom::OneRoom(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	//setAttribute(Qt::WA_TranslucentBackground);//±³¾°Í¸Ã÷
	//setWindowOpacity(0.8);
	QImage image;   
	image.load(":/Resources/Resources/img/oneroom.jpg");   
	image = image.scaled(120,120);
	ui.label->clear();    
	ui.label->setPixmap(QPixmap::fromImage(image));
	this->setStyleSheet("background: rgb(33,33,33);border-width:0;border-style:outset");
	ui.pushButton->setStyleSheet("background: rgb(117,117,117);border-width:0;border-style:outset");
	ui.lineEdit->setStyleSheet("border:1px solid grey;color:white");
	ui.lineEdit_2->setStyleSheet("border:1px solid grey;color:white");
}


void OneRoom::on_pushButton_clicked() 
{
	this->hide();
	emit sendsignal();
}