#pragma once
#include <ui_SettingBoard.h>
#include <QtWidgets/QMainWindow>
#include "ChangePasswordWindow.h"

class SettingBoard : public QMainWindow
{
	Q_OBJECT

public:
	SettingBoard(QWidget *parent = Q_NULLPTR);
	ChangePasswordWindow *changePasswordWindow;
	Ui::settingBoard ui;

private slots:
	void on_changePasswordBtn_clicked();
	void on_cancelButton_clicked();
	void on_changeButton_clicked();

signals:
	void historyList_num(int num);

};