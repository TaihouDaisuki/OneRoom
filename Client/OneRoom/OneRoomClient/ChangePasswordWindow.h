#pragma once
#include <QtWidgets/QMainWindow>
#include "ui_ChangePasswordWindow.h"
#include <qstring.h>
#include "Define.h"

class ChangePasswordWindow : public QMainWindow
{
	Q_OBJECT

public:
	ChangePasswordWindow(QWidget *parent = Q_NULLPTR);
	void setOldPassword(QString password);
private slots:
	void on_confirmButton_clicked();

signals:
	void new_password(QString password);

private:
	Ui::changePasswordWindow ui;
	QString oldPassword;
};