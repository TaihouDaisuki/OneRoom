#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_OneRoom.h"
#include <QMouseEvent>
#include <windows.h> 
#include <WinUser.h>
#include <windowsx.h> 
#include <dwmapi.h> 
#include <objidl.h> // Fixes error C2504: 'IUnknown' : base class undefined 
#include <gdiplus.h> 
#include <GdiPlusColor.h> 
#include <QWidget>
#include <QGraphicsBlurEffect>
#include <QGraphicsPixmapItem>
#include <QPaintEvent>
#include <QPainter>
#include <QTimer>
#include <QDebug>
#include <QApplication>
#include <QDesktopWidget>
#include <QEvent>
#include <QMouseEvent>
#include <QMessageBox>
#include <QtWinExtras>
#include "Define.h"
#pragma comment (lib,"Dwmapi.lib") // Adds missing library, fixes error LNK2019: unresolved external symbol __imp__DwmExtendFrameIntoClientArea 
#pragma comment (lib,"user32.lib")
#include "TcpSocket.h"


class OneRoom : public QMainWindow
{
	Q_OBJECT

public:
	OneRoom(QWidget *parent = Q_NULLPTR);
	TcpClient *tcpclient;
private:
	
	Ui::OneRoomClass ui;
private slots:
	void on_pushButton_clicked();
public slots:
	void ReceivePack(int value, char *info, int len);

signals:
	void sendsignal();
};
