#include "OneRoomClient.h"
#include <qdatetime.h>
#include <qtextcodec.h>
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qdebug.h>

OneRoomClient::OneRoomClient(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.userListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);	// 设置多选
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));	// 设置本地编码
	ui.msgTextEdit->setFontFamily("MicrosoftYaHei");
	ui.msgTextEdit->setFontPointSize(12);
	ui.msgTextEdit->installEventFilter(this);
	this->setStyleSheet("background: rgb(33,33,33);border-width:0;border-style:outset;border:1px solid grey;color:white");
	// test
	userList.append(UserInfo("Megumi", "Kagaya", QString::number(QDateTime::currentDateTime().toTime_t())));
	userList.append(UserInfo(QString::fromLocal8Bit("测试"), "test", QString::number(QDateTime::currentDateTime().toTime_t())));
	userList.append(UserInfo("1234567890", "number", QString::number(QDateTime::currentDateTime().toTime_t())));

	updateUserList();
	// connect
	//connect(ui.sendMsgBtn, SIGNAL(clicked()), this, SLOT(on_sendMsgBtn_clicked));
	//this->tcpclient = new Socket;
	//connect(this->tcpclient, &TcpClient::getNewmessage, this, &OneRoomClient::getMess);

	this->loginWindow = new LoginWindow;
	this->loginWindow->tcpclient = &this->socket;
	this->loginWindow->show();

	connect(this->loginWindow, &LoginWindow::sendsignal, this, &OneRoomClient::reshow_mainwindow);
	connect(&this->socket, &Socket::getNewmessage, this->loginWindow, &LoginWindow::ReceivePack);

	setWindowOpacity(0.9);
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
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("请选择发送用户"));
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
	QByteArray msgByteArray = msg.toLocal8Bit();	// 转为编码格式
	int dataSize = ((nCount + 1) * MAX_USERNAME_SIZE) + msgByteArray.length() + 1;	// 数据部分含尾零
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
	head.dataLen = dataSize;

	// 添加数据
	char* data = new(std::nothrow) char[dataSize];
	if (data == NULL) {
		QMessageBox::warning(this, tr("FBI Warning"), tr("new error"));
		return;
	}
	int length = addTargetUserData(itemList, data, nCount);
	// copy message
	memcpy(&data[length], msgByteArray.data(), msgByteArray.length() + 1);

	// 发送package
	bool isSending = true;	// 发送状态

	if (true) {//if (ui.msgListWidget->count() % ) {	// 测试用，交换收发方
		if (isSending) {
			handleMessageTime(time);

			Message* message = new Message(ui.msgListWidget->parentWidget());
			QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
			handleMessage(message, item, msg, time, Message::User_Me);
			// 调用send函数
			socket.Send(head, data);
			sendMsgQueue.push_back(message);
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

}

void OneRoomClient::on_sendFileBtn_clicked()
{
	//定义文件对话框类
	QFileDialog *fileDialog = new QFileDialog(this);
	//定义文件对话框标题
	fileDialog->setWindowTitle(tr("选择发送文件"));
	//设置默认文件路径
	fileDialog->setDirectory(".");
	//设置文件过滤器
	fileDialog->setNameFilter(tr("*.*"));
	//设置视图模式
	fileDialog->setViewMode(QFileDialog::Detail);
	//选择的文件的路径
	QStringList fileNames;
	if (fileDialog->exec())
	{
		fileNames = fileDialog->selectedFiles();
	}

}

void OneRoomClient::on_sendImgBtn_clicked()
{
	//定义文件对话框类
	QFileDialog *fileDialog = new QFileDialog(this);
	//定义文件对话框标题
	fileDialog->setWindowTitle(tr("选择图片"));
	//设置默认文件路径
	fileDialog->setDirectory(".");
	//设置文件过滤器
	fileDialog->setNameFilter(tr("Images(*.png *.jpg *.jpeg *.bmp)"));
	//设置视图模式
	fileDialog->setViewMode(QFileDialog::Detail);
	//打印所有选择的图片的路径
	QStringList fileNames;
	if (fileDialog->exec())
	{
		fileNames = fileDialog->selectedFiles();
	}

}

void OneRoomClient::on_package_arrived(PackageHead head, char* data)
{
	// 数据
	if (head.isData == 1)
	{
		switch (head.type) {
			case SERVER_ACK_MESSAGE: {
				// 收到消息确认包，确认消息发送成功
				Message *message;
				message = sendMsgQueue.front();
				sendMsgQueue.pop_front();
				message->setTextSuccess();
				break;
			}
			case SERVER_ACK_CHANGE_PASSWORD:
				emit change_password_success();
				break;
			case SERVER_RETURN_SETTING:
				// 确认登陆成功
				break;
			case SERVER_RETURN_ERROR_C:
				if (data[0] == SEND_MESSAGE_FAIL) {
					// 发送消息失败
					QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("消息发送失败，指定用户不存在"));
				}
				else if (data[0] == PASSWORD_ERROR) {
					// 改密码失败，原密码错误
					QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("原密码错误"));
					emit change_password_fail();
				}
				else {
					// nothing
				}
				break;
			case SERVER_RETUEN_ERROR_D:
				socket.disconnect();	// 断开连接
				if (data[0] == ENFORCE_OFFLINE) {
					// 强制下线
					logout();	// 登出
				}
				else {
					// nothing
				}
				break;
			case SERVER_RETURN_USERLIST: {
				// 更新用户列表
				int num = head.dataLen / MAX_USERNAME_SIZE;
				userList.clear();
				UserInfo userInfo;
				for (int i = 0; i < num; i++) {
					//user.setInfo();
					userList.append(userInfo);
				}
				updateUserList();
				break;
			}
			default:
				QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("server return error"));
				return;
		}
	}
	// 控制
	else
	{

	}

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
		memcpy(&data[i * MAX_USERNAME_SIZE], userNameByteArray.data(), MAX_USERNAME_SIZE);
		length += MAX_USERNAME_SIZE;
	}

	// 添加当前用户名称
	userNameByteArray = currentUser.userName().toLocal8Bit();
	memcpy(&data[nCount * MAX_USERNAME_SIZE], userNameByteArray.data(), MAX_USERNAME_SIZE);
	length += MAX_USERNAME_SIZE;

	return length;
}

/* User List View */
// 更新用户列表
void OneRoomClient::updateUserList()
{
	UserInfo i;
	ui.userListWidget->clear();	// 清空列表
	foreach(i, userList) {	// 更新用户列表
		UserInfo *info = new UserInfo(ui.userListWidget->parentWidget());
		QListWidgetItem *item = new QListWidgetItem(ui.userListWidget);
		handleUserinfo(info, item, i.nickName(), i.userName(), i.loginTime());
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

/* reload event function */
void OneRoomClient::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);	// 取消未使用变量警告

	for (int i = 0; i < ui.msgListWidget->count(); i++) {
		Message *message = (Message*)ui.msgListWidget->itemWidget(ui.msgListWidget->item(i));
		QListWidgetItem *item = ui.msgListWidget->item(i);
		handleMessage(message, item, message->text(), message->time(), message->userType());
	}
}

bool OneRoomClient::eventFilter(QObject *obj, QEvent *e)
{
	Q_ASSERT(obj == ui.msgTextEdit);	// 保证obj为msgTextEdit
	if (e->type() == QEvent::KeyPress)
	{
		QKeyEvent *event = static_cast<QKeyEvent*>(e);
		if (event->key() == Qt::Key_Return)	// 回车发送
		{
			on_sendMsgBtn_clicked(); //发送消息的槽
			return true;
		}
	}
	return false;
}

void OneRoomClient::on_logOutBtn_clicked() {
	this->socket.DisConnect();

}

void OneRoomClient::reshow_mainwindow()
{
	this->show();
	disconnect(&this->socket, &Socket::getNewmessage, this->loginWindow, &LoginWindow::ReceivePack);
	connect(&this->socket, &Socket::getNewmessage, this, &OneRoomClient::on_package_arrived);
}

void OneRoomClient::logout()
{
	this->hide();
	loginWindow->show();
	disconnect(&this->socket, &Socket::getNewmessage, this, &OneRoomClient::on_package_arrived);
	connect(&this->socket, &Socket::getNewmessage, this->loginWindow, &LoginWindow::ReceivePack);
}