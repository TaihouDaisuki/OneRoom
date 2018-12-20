#include "ChangePasswordWindow.h"
#include <qmessagebox.h>

ChangePasswordWindow::ChangePasswordWindow(QWidget *parent)
{
	ui.setupUi(this);
}

void ChangePasswordWindow::setOldPassword(QString password)
{
	oldPassword = password;
}

void ChangePasswordWindow::on_confirmButton_clicked()
{
	QString oldpw = ui.oldPwLineEdit->text();
	QString newpw = ui.newPwLineEdit->text();
	
	if (oldpw == "") {
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("请填写原密码"));
		return;
	}
	else if (newpw.length() < MIN_PASSWORD_SIZE) {
		if (newpw == "")
			QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("请填写新密码"));
		else {
			QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("密码长度小于") + QString::number(MIN_PASSWORD_SIZE));
		}
		return;
	}
	else if (newpw == oldPassword) {
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("请勿与原密码相同"));
		return;
	}
	
	emit new_password(newpw);
}
