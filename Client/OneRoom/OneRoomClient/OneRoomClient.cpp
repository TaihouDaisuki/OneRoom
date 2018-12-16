#include "OneRoomClient.h"
#include <qdatetime.h>

OneRoomClient::OneRoomClient(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// connect
	connect(ui.sendMsgBtn, SIGNAL(clicked()), this, SLOT(on_sendMsgBtn_clicked));
} 

/* 事件处理函数 */
void OneRoomClient::on_sendMsgBtn_clicked()
{
	QString msg = ui.msgTextEdit->toPlainText();	// 提取输入框信息
	ui.msgTextEdit->setPlainText("");	// 清空输入框
	QString time = QString::number(QDateTime::currentDateTime().toTime_t());	// 获取时间戳

	bool isSending = true;	// 发送状态

	if (ui.msgListWidget->count() % 2) {	// 测试用，交换收发方
		if (isSending) {
			handleMessageTime(time);

			Message* message = new Message(ui.msgListWidget->parentWidget());
			QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
			handleMessage(message, item, msg, time, Message::User_Me);
		}
		else {
			bool isOver = true;
			for (int i = ui.msgListWidget->count() - 1; i > 0; i--) {
				Message* message = (Message*)ui.msgListWidget->itemWidget(ui.msgListWidget->item(i));	// 当前选取的消息
				if (message->text() == msg) {	// 处理刚发送的消息
					isOver = false;
					message->setTextSuccess();
				}
			}
			if (isOver) {
				handleMessageTime(time);

				Message* message = new Message(ui.msgListWidget->parentWidget());
				QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
				handleMessage(message, item, msg, time, Message::User_Me);
				message->setTextSuccess();
			}
		}
	}
	else {
		if (msg != "") {
			handleMessageTime(time);

			Message* message = new Message(ui.msgListWidget->parentWidget());
			QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
			handleMessage(message, item, msg, time, Message::User_He);
		}
	}
	ui.msgListWidget->setCurrentRow(ui.msgListWidget->count() - 1);	// 设置当前行数

}

void OneRoomClient::on_newMsg_come(QString msg, QString sendTime)
{

}

/* User List View */
// 更新用户列表
void OneRoomClient::updateUserList()
{
	UserInfo i;
	foreach(i, userList) {	// 更新用户列表
		UserInfo *info = new UserInfo(ui.userListWidget->parentWidget());
		QListWidgetItem *item = new QListWidgetItem(ui.userListWidget);
		handleUserinfo(info, item, i.nickName(), i.userName(), i.loginTime());
	}
}

// 加载信息并添加到list中
void OneRoomClient::handleUserinfo(UserInfo *userInfo, QListWidgetItem *item, QString nickName, QString userName, QString loginTime)
{
	userInfo->setFixedWidth(ui.userListWidget->width());
	QSize size = userInfo->rectSize();
	item->setSizeHint(size);
	userInfo->setInfo(nickName, userName, loginTime);
	ui.userListWidget->setItemWidget(item, userInfo);
}

/* Message View */
// 设置消息图形属性并加入list
void OneRoomClient::handleMessage(Message *message, QListWidgetItem *item, QString text, QString time, Message::User_Type type)
{
	message->setFixedWidth(ui.msgListWidget->width() - 10);
	QSize size = message->fontRect(text);	// 获取文本框大小
	item->setSizeHint(size);	// 设置尺寸
	message->setText(text, time, size, type);
	ui.msgListWidget->setItemWidget(item, message);	
}

// 处理消息时间
void OneRoomClient::handleMessageTime(QString curMsgTime)
{
	bool isShowTime = false;
	if (ui.msgListWidget->count() > 0) {
		// 如果有item
		QListWidgetItem *lastItem = ui.msgListWidget->item(ui.msgListWidget->count() - 1);	// 获取最后一个Item
		Message *message = (Message *)ui.msgListWidget->itemWidget(lastItem);	// 返回当前item对应的widget
		int lastTime = message->time().toInt();	// 获取最后一条消息的发送时间
		int curTime = curMsgTime.toInt();
		isShowTime = ((curTime - lastTime) > 60);	// 如果距离最后一条消息发送时间超过一分钟以上显示时间
	}
	else {
		isShowTime = true;	// 没有消息的情况显示时间
	}

	if (isShowTime) {
		Message *messageTime = new Message(ui.msgListWidget->parentWidget());	// 时间消息
		QListWidgetItem *itemTime = new QListWidgetItem(ui.msgListWidget);	// 新itme对象，用parent widget初始化
		QSize size = QSize(ui.msgListWidget->width() - 10, 40);
		messageTime->resize(size);	// 设置大小
		itemTime->setSizeHint(size);
		messageTime->setText(curMsgTime, curMsgTime, size, Message::User_Time);	// 设置时间消息
		ui.msgListWidget->setItemWidget(itemTime, messageTime);	// add itme and widget to list
	}

}

void OneRoomClient::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);	// 取消未使用变量警告

	for (int i = 0; i < ui.msgListWidget->count(); i++) {
		Message *message = (Message*)ui.msgListWidget->itemWidget(ui.msgListWidget->item(i));
		QListWidgetItem *item = ui.msgListWidget->item(i);
		handleMessage(message, item, message->text(), message->time(), message->userType());
	}
}
