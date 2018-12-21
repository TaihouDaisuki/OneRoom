#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_LoginWindow.h"
#include "ui_ChangePasswordWindow.h"
//#include <QMouseEvent>
//#include <windows.h> 
//#include <windowsx.h> 
//#include <dwmapi.h> 
//#include <objidl.h> // Fixes error C2504: 'IUnknown' : base class undefined 
//#include <gdiplus.h> 
//#include <GdiPlusColor.h> 
//#include <QWidget>
//#include <QGraphicsBlurEffect>
//#include <QGraphicsPixmapItem>
//#include <QPaintEvent>
//#include <QPainter>
//#include <QTimer>
//#include <QDebug>
//#include <QApplication>
//#include <QDesktopWidget>
//#include <QEvent>
//#include <QMouseEvent>
#include <QMessageBox>
//#include <QtWinExtras>
#include "Define.h"
//#pragma comment (lib,"Dwmapi.lib") // Adds missing library, fixes error LNK2019: unresolved external symbol __imp__DwmExtendFrameIntoClientArea 
//#pragma comment (lib,"user32.lib")
#include "Socket.h"
#include "ChangePasswordWindow.h"

class LoginWindow : public QMainWindow
{
	Q_OBJECT

public:
	LoginWindow(QWidget *parent = Q_NULLPTR);
	Socket *tcpclient;

private:
	Ui::LoginWindow ui;
	ChangePasswordWindow *changePwWin;
	QString init_password;

private slots:
	void on_pushButton_clicked();

public slots:
	void ReceivePack(PackageHead value, char *info);
	void handle_new_password(QString new_password);

signals:
	void sendsignal(QString userName, QString password, int histroyListNum);
	void change_password_result(int result);
};
