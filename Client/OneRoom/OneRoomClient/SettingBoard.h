#pragma once
#include <ui_SettingBoard.h>
#include <QtWidgets/QMainWindow>
#include "ChangePasswordWindow.h"

class SettingBoard : public QMainWindow
{
	Q_OBJECT

public:
	SettingBoard(QWidget *parent = Q_NULLPTR);

private slots:
	void on_changePasswordBtn_clicked();
	void on_cancelButton_clicked();
	void on_changeButton_clicked();

signals:
	void new_settings();

private:
	Ui::settingBoard ui;
	ChangePasswordWindow *changePasswordWindow;
};