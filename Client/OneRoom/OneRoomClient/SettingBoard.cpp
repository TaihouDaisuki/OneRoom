#include <SettingBoard.h>

SettingBoard::SettingBoard(QWidget *parent)
{
	ui.setupUi(this);
	//setStyleSheet("background: rgb(33,33,33);border-width:0;border-style:outset;color:white");
	setWindowOpacity(0.8);
	changePasswordWindow = new ChangePasswordWindow(this);
}

void SettingBoard::on_changePasswordBtn_clicked()
{
	changePasswordWindow->show();
}

void SettingBoard::on_cancelButton_clicked()
{
	ui.spinBox->setValue(ui.spinBox->minimum());
	this->hide();
}

void SettingBoard::on_changeButton_clicked()
{
	emit historyList_num(ui.spinBox->value());
}