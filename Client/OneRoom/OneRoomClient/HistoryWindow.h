#pragma once
#include "ui_HistoryWindow.h"
#include <QtWidgets/QMainWindow>

class HistoryWindow : public QMainWindow
{
	Q_OBJECT
	
public:
	HistoryWindow(QWidget *parent = Q_NULLPTR);

//private:
	Ui::HistoryWindow ui;
};