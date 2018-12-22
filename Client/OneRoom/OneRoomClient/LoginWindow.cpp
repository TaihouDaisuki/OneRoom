#include "LoginWindow.h"

LoginWindow::LoginWindow(QWidget *parent)
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

	// child window
	changePwWin = new ChangePasswordWindow(this);
	
	// connect
	connect(changePwWin, &ChangePasswordWindow::new_password, this, &LoginWindow::handle_new_password);
}

void LoginWindow::on_pushButton_clicked()
{
	tcpclient->Connect();
	QString username = ui.lineEdit->text();
	QString password = ui.lineEdit_2->text();
	if (username.toLocal8Bit().length() > MAX_USERNAME_SIZE || password.toLocal8Bit().length()>20)
	{
		QMessageBox::critical(0,
			"critical message", QString::fromLocal8Bit("用户名或密码超出长度限制，请重新输入"),
			QMessageBox::Ok | QMessageBox::Default,
			QMessageBox::Cancel | QMessageBox::Escape, 0);
		ui.lineEdit->clear();
		ui.lineEdit_2->clear();
		return;
	}

	PackageHead temp = SendTest;
	temp.isData = 0;
	temp.type = CLIENT_REQUIRE_LOGIN;
	temp.dataLen = 40;
	char ch[40];
	memset(ch, 0, sizeof(ch));
	memcpy(ch, username.toLocal8Bit(), username.toLocal8Bit().length());
	memcpy(ch+20, password.toLocal8Bit(), password.toLocal8Bit().length());

	this->tcpclient->Send(temp, ch);
	ui.pushButton->setDisabled(true);
	return;
}

void LoginWindow::handle_new_password(QString new_password)
{
	// 请求修改密码
	PackageHead temp;
	temp.isData = 0;
	temp.type = CLIENT_CHANGE_PASSWORD;
	temp.isCut = 0;
	temp.dataLen = 40;
	char ch[40];
	memset(ch, 0, sizeof(ch));
	memcpy(ch, init_password.toLocal8Bit(), init_password.toLocal8Bit().length());
	memcpy(ch + 20, new_password.toLocal8Bit(), new_password.toLocal8Bit().length());

	this->tcpclient->Send(temp, ch);
}

void LoginWindow::ReceivePack(PackageHead head, char *info)
{
	if (head.isData == 0) 
	{
		switch (head.type) {
			case SERVER_RETUEN_ERROR_D: {
				tcpclient->DisConnect();
				if (info[0] == NO_SUCH_USER) {
					QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("不存在的用户名"));
					ui.lineEdit->clear();
					ui.lineEdit_2->clear();
				}
				else if (info[0] == PASSWORD_ERROR) {
					QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("密码错误"));
					ui.lineEdit->clear();
					ui.lineEdit_2->clear();
				}
				break;
			}
			case SERVER_RETURN_ERROR_C: {
				if (info[0] == ENFORCE_CHANGE_PASSWORD) {
					init_password = ui.lineEdit_2->text();	// 要求强制修改密码代表当前用户输入的就是初始密码
					changePwWin->show();
					changePwWin->setFocus();
					QMessageBox::warning(changePwWin, tr("FBI Warning"), QString::fromLocal8Bit("首次登陆请修改密码"));
				}
				else if (info[0] == PASSWORD_ERROR)
					emit change_password_result(ERROR);
				else
					QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("SERVER_RETURN_ERROR_C ERROR CODE ERROR"));
				break;
			}
			case SERVER_ACK_CHANGE_PASSWORD: {
				emit change_password_result(OK);	// 不代表登陆成功，需重新登陆
				changePwWin->hide();
				ui.lineEdit_2->clear();	// 清空密码
				break;
			}
			case SERVER_RETURN_SETTING: {
				this->hide();
				if(info)
					emit sendsignal(ui.lineEdit->text(), ui.lineEdit_2->text(), ntohl(*(int*)info));	// 参数需发送设置信息
				else
					emit sendsignal(ui.lineEdit->text(), ui.lineEdit_2->text(), 10);	// 参数需发送设置信息
				break;
			}
			default:
				break;
		}
	}
	else
	{

	}
	ui.pushButton->setEnabled(true);
	return;
}

void LoginWindow::handle_socket_error(QString errorMsg)
{
	QMessageBox::warning(this, tr("FBI Warning"), errorMsg);
}
