#include "OneRoomClient.h"
#include <qdatetime.h>
#include <qtextcodec.h>
#include <qmessagebox.h>
#include "Socket.h"

OneRoomClient::OneRoomClient(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.userListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);	// 设置多选
	// 设置编码
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));

	// test
	userList.append(new UserInfo("Megumi", "Kagaya", QString::number(QDateTime::currentDateTime().toTime_t())));
	userList.append(new UserInfo(QString::fromLocal8Bit("测试"), "test", QString::number(QDateTime::currentDateTime().toTime_t())));
	userList.append(new UserInfo("1234567890", "number", QString::number(QDateTime::currentDateTime().toTime_t())));

	updateUserList();
	// connect
	connect(ui.sendMsgBtn, SIGNAL(clicked()), this, SLOT(on_sendMsgBtn_clicked));
} 

/* 事件处理函数 */
void OneRoomClient::on_sendMsgBtn_clicked()
{
	QString msg = ui.msgTextEdit->toPlainText();	// 提取输入框信息
	ui.msgTextEdit->setPlainText("");	// 清空输入框
	QString time = QString::number(QDateTime::currentDateTime().toTime_t());	// 获取时间戳
	
	if (msg == "")	// 空消息直接返回
		return;

	// 判断数目
	QList<QListWidgetItem *> itemList = ui.userListWidget->selectedItems();	// 所有选中的项目
	int nCount = itemList.count();
	if (nCount < 1) {
		// 无选择用户，提示需选择发送对象
		QMessageBox::warning(this, tr("FBI Warning"), tr("请选择发送用户"));
		return;
	}
	
	// 判断消息发送方式
	unsigned char sendType = 0;
	if (nCount == ui.userListWidget->count())
		sendType = DATA_TYPE_ALL;
	else if (nCount == 1)
		sendType = DATA_TYPE_SINGLE;
	else
		sendType = DATA_TYPE_GROUP;

	// 构成数据包
	// 添加头部
	PackageHead head;
	switch (sendType) {
	case DATA_TYPE_SINGLE:
		memcpy(&head, &SingleHead, sizeof(PackageHead));
		break;
	case DATA_TYPE_GROUP:
		memcpy(&head, &GroupHead, sizeof(PackageHead));
		break;
	case DATA_TYPE_ALL:
		memcpy(&head, &AllHead, sizeof(PackageHead));
		break;
	default:
		QMessageBox::warning(this, tr("FBI Warning"), tr("add message head error"));
		return;
	}

	// 添加数据
	QByteArray msgByteArray = msg.toLocal8Bit();	// 转为编码格式
	int dataSize = ((nCount + 1) * USERNAME_BUFF_SIZE) + msgByteArray.length() + 1;	// 数据部分含尾零
	char* data = new(std::nothrow) char[dataSize];
	if (data == NULL) {
		QMessageBox::warning(this, tr("FBI Warning"), tr("new error"));
		return;
	}
	int length = addTargetUserData(itemList, data, nCount);
	

	// 发送package
	bool isSending = true;	// 发送状态

	if (isSending) {
		handleMessageTime(time);

		Message* message = new Message(ui.msgListWidget->parentWidget());
		QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
		handleMessage(message, item, msg, time, Message::User_Me);
		// 调用send函数
		int ret = socket.SendMessage(head, data);
		if(ret)	// 设置发送成功
			message->setTextSuccess();
	}
	//else {
	//	bool isOver = true;
	//	for (int i = ui.msgListWidget->count() - 1; i > 0; i--) {
	//		Message* message = (Message*)ui.msgListWidget->itemWidget(ui.msgListWidget->item(i));	// 当前选取的消息
	//		if (message->text() == msg) {	// 处理刚发送的消息
	//			isOver = false;
	//			message->setTextSuccess();
	//		}
	//	}
	//	if (isOver) {	// 
	//		handleMessageTime(time);

	//		Message* message = new Message(ui.msgListWidget->parentWidget());
	//		QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
	//		handleMessage(message, item, msg, time, Message::User_Me);
	//	}
	//}
	
	//清空临时条目列表
	itemList.clear();
	ui.msgListWidget->setCurrentRow(ui.msgListWidget->count() - 1);	// 设置当前行数
}

// 将itemList中的用户名提取出来加上当前用户的用户名后转按发送格式从data指向地址开始填入, 返回填入长度
int OneRoomClient::addTargetUserData(QList<QListWidgetItem *> &itemList, char* data, int nCount)
{
	//char* data = new char[(nCount + 1) * USERNAME_BUFF_SIZE];
	UserInfo *user;
	QByteArray userNameByteArray;
	int length = 0;

	for (int i = 0; i < nCount; i++) {
		user = (UserInfo *)ui.userListWidget->itemWidget(itemList[i]);
		// QString to GBK char*
		userNameByteArray = user->userName().toLocal8Bit();
		memcpy(&data[i * USERNAME_BUFF_SIZE], userNameByteArray.data(), USERNAME_BUFF_SIZE);
		length += USERNAME_BUFF_SIZE;
	}

	// 添加当前用户名称
	userNameByteArray = currentUser.userName().toLocal8Bit();
	memcpy(&data[nCount * USERNAME_BUFF_SIZE], userNameByteArray.data(), USERNAME_BUFF_SIZE);
	length += USERNAME_BUFF_SIZE;

	return length;
}


void OneRoomClient::on_newMsg_come(QString msg, QString sendTime)
{

}

/* User List View */
// 更新用户列表
void OneRoomClient::updateUserList()
{
	UserInfo *i;
	foreach(i, userList) {	// 更新用户列表
		UserInfo *info = new UserInfo(ui.userListWidget->parentWidget());
		QListWidgetItem *item = new QListWidgetItem(ui.userListWidget);
		handleUserinfo(info, item, i->nickName(), i->userName(), i->loginTime());
	}
}

// 加载信息并添加到list中
void OneRoomClient::handleUserinfo(UserInfo *userInfo, QListWidgetItem *item, QString nickName, QString userName, QString loginTime)
{
	int width = 250;
	userInfo->setFixedWidth(width);
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
