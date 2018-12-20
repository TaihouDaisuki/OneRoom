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
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("����дԭ����"));
		return;
	}
	else if (newpw.length() < MIN_PASSWORD_SIZE) {
		if (newpw == "")
			QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("����д������"));
		else {
			QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("���볤��С��") + QString::number(MIN_PASSWORD_SIZE));
		}
		return;
	}
	else if (newpw == oldPassword) {
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("������ԭ������ͬ"));
		return;
	}
	
	emit new_password(newpw);
}
