#include <SettingBoard.h>

SettingBoard::SettingBoard(QWidget *parent = Q_NULLPTR)
{
	ui.setupUi(this);
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
	emit new_settings();
}