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
	ui.msgListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));	// 设置本地编码
	ui.msgTextEdit->setFontFamily("MicrosoftYaHei");
	ui.msgTextEdit->setFontPointSize(12);
	ui.msgTextEdit->installEventFilter(this);
	setStyleSheet("background: rgb(33,33,33);border-width:0;border-style:outset;border:1px solid grey;color:white");


	// 初始化子窗口
	loginWindow = new LoginWindow(this);
	loginWindow->tcpclient = &this->socket;
	loginWindow->show();
	settingBoard = new SettingBoard(this);
	
	// connect
	connect(this->loginWindow->tcpclient, &Socket::sock_error_occurred, this->loginWindow, &LoginWindow::handle_socket_error);
	connect(this->loginWindow, &LoginWindow::sendsignal, this, &OneRoomClient::reshow_mainwindow);
	connect(this->settingBoard->changePasswordWindow, &ChangePasswordWindow::new_password, this->loginWindow, &LoginWindow::handle_new_password);
	connect(&this->socket, &Socket::getNewmessage, this->loginWindow, &LoginWindow::ReceivePack);

	connect(this, &OneRoomClient::change_password_result, this->settingBoard->changePasswordWindow, &ChangePasswordWindow::handle_password_result);
	connect(this->settingBoard, &SettingBoard::historyList_num, this, &OneRoomClient::send_history_num_setting);

	setWindowOpacity(0.9);

}


void OneRoomClient::on_userListWidget_itemDoubleClicked(QListWidgetItem *item)
{
	UserInfo *user = (UserInfo *)ui.userListWidget->itemWidget(item);
	QString fromname = user->userName();
	QString toname = currentUser.userName();

	int count = ui.msgListWidget->count();
	int last_time;
	for (int i = 0; i < count; i++)
	{
		Message *message = (Message *)ui.msgListWidget->itemWidget(ui.msgListWidget->item(i));	// 返回当前item对应的widget
		if (message->userType() == Message::User_Type::User_Time)
		{
			last_time = i;
			ui.msgListWidget->setItemHidden(ui.msgListWidget->item(i), true);
		}
		if (message->userType() == Message::User_Type::User_Me)
		{
			int count = message->m_toUserNameList.count();
			int flag = 0;
			for (int j = 0; j < count; j++)
				if (message->m_toUserNameList.at(j) == fromname)
					flag = 1;
			if(flag==0)
				ui.msgListWidget->setItemHidden(ui.msgListWidget->item(i), true);
			else
			{
				ui.msgListWidget->setItemHidden(ui.msgListWidget->item(last_time), false);
				ui.msgListWidget->setItemHidden(ui.msgListWidget->item(i), false);
			}
		}
		if (message->userType() == Message::User_Type::User_He)
		{
			if (message->fromUserName() != fromname)
				ui.msgListWidget->setItemHidden(ui.msgListWidget->item(i), true);
			else
			{
				ui.msgListWidget->setItemHidden(ui.msgListWidget->item(last_time), false);
				ui.msgListWidget->setItemHidden(ui.msgListWidget->item(i), false);
			}
		}
	}

}
/* 事件处理函数 */
void OneRoomClient::on_sendMsgBtn_clicked()
{
	setButtonDisable();
	QString msg = ui.msgTextEdit->toPlainText();	// 提取输入框信息
	ui.msgTextEdit->setPlainText("");	// 清空输入框
	QString time = QString::number(QDateTime::currentDateTime().toTime_t());	// 获取时间戳

	if (msg == "")	// 空消息直接返回
	{
		setButtonEnable();
		return;
	}

	// 判断数目
	QList<QListWidgetItem *> itemList = ui.userListWidget->selectedItems();	// 所有选中的项目
	int nCount = itemList.count();
	if (nCount < 1) {
		// 无选择用户，提示需选择发送对象
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("请选择发送用户"));
		setButtonEnable();
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
	// 添加头部
	PackageHead head;
	char* data = NULL;
	int dataSize = 0;

	switch (sendType) {
	case DATA_TYPE_SINGLE: {
		memcpy(&head, &SingleHead, sizeof(PackageHead));
		dataSize = MAX_USERNAME_SIZE + msgByteArray.length() + 1;	// 数据部分含尾零
		data = new(std::nothrow) char[dataSize];
		if (data == NULL) {
			QMessageBox::warning(this, tr("FBI Warning"), tr("new error"));
			return;
		}
		// 单发目的用户名
		UserInfo *user;
		QByteArray userNameByteArray;
		user = (UserInfo *)ui.userListWidget->itemWidget(itemList[0]);
		userNameByteArray = user->userName().toLocal8Bit();
		memcpy(data, userNameByteArray.data(), userNameByteArray.length() + 1);
		// copy message
		memcpy(&data[MAX_USERNAME_SIZE], msgByteArray.data(), msgByteArray.length() + 1);
		break;
	}
	case DATA_TYPE_GROUP: {
		memcpy(&head, &GroupHead, sizeof(PackageHead));
		// 添加数据
		dataSize = 1 + (nCount * MAX_USERNAME_SIZE) + msgByteArray.length() + 1;	// 数据部分含尾零
		data = new(std::nothrow) char[dataSize];
		if (data == NULL) {
			QMessageBox::warning(this, tr("FBI Warning"), tr("new error"));
			return;
		}
		int length = addTargetUserData(itemList, data, nCount);
		// copy message
		memcpy(&data[length], msgByteArray.data(), msgByteArray.length() + 1);
		break;
	}
	case DATA_TYPE_ALL: {
		memcpy(&head, &AllHead, sizeof(PackageHead));
		dataSize = MAX_USERNAME_SIZE + msgByteArray.length() + 1;	// 数据部分含尾零
		data = new(std::nothrow) char[dataSize];
		if (data == NULL) {
			QMessageBox::warning(this, tr("FBI Warning"), tr("new error"));
			return;
		}
		// copy message
		memcpy(&data[MAX_USERNAME_SIZE], msgByteArray.data(), msgByteArray.length() + 1);
		break;
	}
	default:
		QMessageBox::warning(this, tr("FBI Warning"), tr("add message head error"));
		return;
	}
	head.dataLen = dataSize;

	// 发送package
	handleMessageTime(time);
	Message* message = new Message(ui.msgListWidget->parentWidget());
	message->setUserName(currentUser.userName());
	if (sendType == DATA_TYPE_SINGLE)
	{
		char temp[20];
		memcpy(temp, data, 20);
		message->m_toUserNameList.append(temp);
	}
	else if (sendType == DATA_TYPE_GROUP)
	{
		unsigned char count = data[0];
		for (unsigned char i = 0; i < count; i++)
		{
			char temp[20];
			memcpy(temp, data+1+20*i, 20);
			message->m_toUserNameList.append(temp);
		}
	}
	else if (sendType == DATA_TYPE_ALL)
	{
		unsigned char count = ui.userListWidget->count();
		for (unsigned char i = 0; i < count; i++)
		{
			char temp[20];
			UserInfo *user = (UserInfo *)ui.userListWidget->itemWidget(ui.userListWidget->item(i));
			message->m_toUserNameList.append(user->userName());
		}
	}
	else
		;



	QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
	handleMessage(message, item, msg, time, Message::User_Me);
	// 调用send函数
	socket.Send(head, data);
	delete data;
	sendMsgQueue.push_back(message);

	//清空临时条目列表
	itemList.clear();
	ui.msgListWidget->setCurrentRow(ui.msgListWidget->count() - 1);	// 设置当前行数
	setButtonEnable();


	on_userListWidget_itemDoubleClicked(ui.userListWidget->selectedItems().at(0));
}

void OneRoomClient::on_sendFileBtn_clicked()
{
	QList<QListWidgetItem *> itemList = ui.userListWidget->selectedItems();	// 所有选中的项目
	int nCount = itemList.count();
	if (nCount < 1) {
		// 无选择用户，提示需选择发送对象
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("请选择发送用户"));
		setButtonEnable();
		return;
	}
	if (nCount > 1) {
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("请选择单个用户发送文件"));
		setButtonEnable();
		return;
	}


	setButtonDisable();
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
	// send
	QString msg = QString::fromLocal8Bit("[文件]");

	if (fileNames.count() < 0)
	{
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("未选择文件"));
		setButtonEnable();
		return;
	}
	QString filename_temp = fileNames.at(0);



	setButtonEnable();
}

void OneRoomClient::on_sendImgBtn_clicked()
{
	setButtonDisable();
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
	// send
	QString msg = QString::fromLocal8Bit("[图片]");
	QString time = QString::number(QDateTime::currentDateTime().toTime_t());	// 获取时间戳

	// 判断数目
	QList<QListWidgetItem *> itemList = ui.userListWidget->selectedItems();	// 所有选中的项目
	int nCount = itemList.count();
	if (nCount < 1) {
		// 无选择用户，提示需选择发送对象
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("请选择发送用户"));
		setButtonEnable();
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
	// 添加头部
	PackageHead head;
	char* data = NULL;
	int dataSize = 0;

	switch (sendType) {
	case DATA_TYPE_SINGLE: {
		memcpy(&head, &SingleHead, sizeof(PackageHead));
		dataSize = MAX_USERNAME_SIZE + msgByteArray.length() + 1;	// 数据部分含尾零
		data = new(std::nothrow) char[dataSize];
		if (data == NULL) {
			QMessageBox::warning(this, tr("FBI Warning"), tr("new error"));
			return;
		}
		// 单发目的用户名
		UserInfo *user;
		QByteArray userNameByteArray;
		user = (UserInfo *)ui.userListWidget->itemWidget(itemList[0]);
		userNameByteArray = user->userName().toLocal8Bit();
		memcpy(data, userNameByteArray.data(), userNameByteArray.length() + 1);
		
		// copy data
		memcpy(&data[MAX_USERNAME_SIZE], msgByteArray.data(), msgByteArray.length() + 1);
		break;
	}
	case DATA_TYPE_GROUP: {
		memcpy(&head, &GroupHead, sizeof(PackageHead));
		// 添加数据
		dataSize = 1 + (nCount * MAX_USERNAME_SIZE) + msgByteArray.length() + 1;	// 数据部分含尾零
		data = new(std::nothrow) char[dataSize];
		if (data == NULL) {
			QMessageBox::warning(this, tr("FBI Warning"), tr("new error"));
			return;
		}
		int length = addTargetUserData(itemList, data, nCount);
		
		// copy data
		memcpy(&data[length], msgByteArray.data(), msgByteArray.length() + 1);
		break;
	}
	case DATA_TYPE_ALL: {
		memcpy(&head, &AllHead, sizeof(PackageHead));
		dataSize = MAX_USERNAME_SIZE + msgByteArray.length() + 1;	// 数据部分含尾零
		data = new(std::nothrow) char[dataSize];
		if (data == NULL) {
			QMessageBox::warning(this, tr("FBI Warning"), tr("new error"));
			return;
		}
		// copy data
		memcpy(&data[MAX_USERNAME_SIZE], msgByteArray.data(), msgByteArray.length() + 1);
		break;
	}
	default:
		QMessageBox::warning(this, tr("FBI Warning"), tr("add message head error"));
		return;
	}
	head.dataLen = dataSize;

	// 发送package
	handleMessageTime(time);

	Message* message = new Message(ui.msgListWidget->parentWidget());
	QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
	handleMessage(message, item, msg, time, Message::User_Me, Message::Msg_Img, fileNames[0]);
	// 调用send函数
	//socket.Send(head, data);
	delete data;
	sendMsgQueue.push_back(message);

	//清空临时条目列表
	itemList.clear();
	ui.msgListWidget->setCurrentRow(ui.msgListWidget->count() - 1);	// 设置当前行数

	setButtonEnable();
}

void OneRoomClient::on_settingBtn_clicked()
{
	settingBoard->show();
}

void OneRoomClient::on_package_arrived(PackageHead head, char* const data)
{
	// 数据
	if (head.isData == 1)
	{
		switch (head.type & 0xf0) {
		case DATA_TYPE_TEXT: {
			QString time = QString::number(QDateTime::currentDateTime().toTime_t());	// 获取时间戳
			handleMessageTime(time);
			QString msg = QString::fromLocal8Bit(data+20);	

			Message* message = new Message(ui.msgListWidget->parentWidget());

			char temp_name[20];
			memcpy(temp_name,data,20);

		 	message->setUserName(QString::fromLocal8Bit(temp_name));
			message->m_toUserNameList.append(currentUser.userName());

			QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
			handleMessage(message, item, msg, time, Message::User_He);
			break;
		}
		case DATA_TYPE_PICTURE: {
			QString time = QString::number(QDateTime::currentDateTime().toTime_t());	// 获取时间戳
			handleMessageTime(time);
			QString msg = QString::fromLocal8Bit(data + 20);

			Message* message = new Message(ui.msgListWidget->parentWidget());
			QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
			handleMessage(message, item, msg, time, Message::User_He);
			break;
		}
		case DATA_TYPE_FILE: {
			QString time = QString::number(QDateTime::currentDateTime().toTime_t());	// 获取时间戳
			handleMessageTime(time);
			QString msg = QString::fromLocal8Bit(data + 20);

			Message* message = new Message(ui.msgListWidget->parentWidget());

			char temp_name[20];
			memcpy(temp_name, data, 20);

			message->setUserName(QString::fromLocal8Bit(temp_name));
			message->m_toUserNameList.append(currentUser.userName());

			QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
			handleMessage(message, item, msg, time, Message::User_He);


			break;
		}
			default:
				QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("server return error"));
		}
	}
	// 控制
	else
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
			emit change_password_result(OK);
			break;
		case SERVER_RETURN_SETTING:
			// 确认登陆成功
			break;
		case SERVER_RETURN_ERROR_C:
			if (data[0] == SEND_MESSAGE_FAIL) {
				// 发送消息失败
				QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("消息发送失败"));
			}
			else if (data[0] == PASSWORD_ERROR) {
				// 改密码失败，原密码错误
				emit change_password_result(ERROR);
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
			int userDataLen = 2 * MAX_USERNAME_SIZE;
			int num = head.dataLen / userDataLen;
			userList.clear();
			UserInfo user;
			for (int i = 0; i < num; i++) {
				user.setInfo(QString::fromLocal8Bit(data + (i * userDataLen) + MAX_USERNAME_SIZE), QString::fromLocal8Bit(data + (i * userDataLen)));
				userList.append(user);
			}
			updateUserList();
			break;
		}
		default:
			QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("server return error"));
		}
	}

	delete data;
}

void OneRoomClient::on_logOutBtn_clicked() 
{
	socket.DisConnect();
	logout();
}

void OneRoomClient::on_msgHistoryBtn_clicked()
{

}

void OneRoomClient::handle_socket_error(QString errorMsg)
{
	QMessageBox::warning(this, tr("FBI Warning"), errorMsg);
}

void OneRoomClient::send_history_num_setting(int num)
{
	int len = sizeof(int);
	PackageHead head;
	char* data = new char[len];
	head.isData = 1;
	head.dataLen = len;
	head.isCut = 0;
	head.type = CLIENT_CHANGE_SETTING;

	*(int*)data = htonl(num);

	socket.Send(head, data);

	delete data;
}

void OneRoomClient::send_password_setting(QString password)
{
	QByteArray pwByteArray;
	PackageHead head;
	char* data = new char[MAX_PASSWORD_SIZE * 2];
	head.isData = 1;
	head.dataLen = MAX_PASSWORD_SIZE * 2;
	head.isCut = 0;
	head.type = CLIENT_CHANGE_PASSWORD;

	pwByteArray = currentUser.password().toLocal8Bit();
	memcpy(data, pwByteArray.data(), MAX_PASSWORD_SIZE);

	pwByteArray = password.toLocal8Bit();
	memcpy(data + MAX_PASSWORD_SIZE, pwByteArray.data(), MAX_PASSWORD_SIZE);

	socket.Send(head, data);

	delete data;
}

// 将itemList中的用户名提取出来加上当前用户的用户名后转按发送格式从data指向地址开始填入, 返回填入长度
int OneRoomClient::addTargetUserData(QList<QListWidgetItem *> &itemList, char* const data, int nCount)
{
	//char* data = new char[(nCount + 1) * USERNAME_BUFF_SIZE];
	UserInfo *user;
	QByteArray userNameByteArray;
	int length = 1;
	
	// 首字节为发送人数
	data[0] = (char)nCount;
	for (int i = 0; i < nCount; i++) {
		user = (UserInfo *)ui.userListWidget->itemWidget(itemList[i]);
		// QString to GBK char*
		userNameByteArray = user->userName().toLocal8Bit();
		memcpy(&data[i * MAX_USERNAME_SIZE + 1], userNameByteArray.data(), MAX_USERNAME_SIZE);
		length += MAX_USERNAME_SIZE;
	}

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
void OneRoomClient::handleMessage(Message *message, QListWidgetItem *item, QString text, QString time, Message::User_Type type, Message::Msg_Type msgType, QString imgPath)
{
	message->setFixedWidth(ui.msgListWidget->width() - 10);
	if (msgType == Message::Msg_Img) {
		message->setMsgType(msgType);
		message->setImgPath(imgPath);
	}
	QSize size = message->fontRect(text);	// 获取气泡大小
	item->setSizeHint(size);	// 设置尺寸
	message->setText(text, time, size, type, msgType, imgPath);
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
		handleMessage(message, item, message->text(), message->time(), message->userType(), message->msgType(), message->imgPath());
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

void OneRoomClient::reshow_mainwindow(QString userName, QString password, int histroyListNum)
{
	// 初始值设置
	currentUser.setInfo("", userName, QString::number(QDateTime::currentDateTime().toTime_t()), password);

	this->show();
	this->setFocus();
	disconnect(&this->socket, &Socket::getNewmessage, this->loginWindow, &LoginWindow::ReceivePack);
	disconnect(this->settingBoard->changePasswordWindow, &ChangePasswordWindow::new_password, this->loginWindow, &LoginWindow::handle_new_password);
	disconnect(this->loginWindow, &LoginWindow::change_password_result, this->settingBoard->changePasswordWindow, &ChangePasswordWindow::handle_password_result);
	disconnect(this->loginWindow->tcpclient, &Socket::sock_error_occurred, this->loginWindow, &LoginWindow::handle_socket_error);

	connect(&this->socket, &Socket::getNewmessage, this, &OneRoomClient::on_package_arrived);
	connect(this->settingBoard->changePasswordWindow, &ChangePasswordWindow::new_password, this, &OneRoomClient::send_password_setting);
	connect(this, &OneRoomClient::change_password_result, this->settingBoard->changePasswordWindow, &ChangePasswordWindow::handle_password_result);
	connect(&this->socket, &Socket::sock_error_occurred, this, &OneRoomClient::handle_socket_error);
	
	QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("登陆成功！"));
}

void OneRoomClient::setButtonDisable()
{
	ui.logOutBtn->setDisabled(true);
	ui.msgHistoryBtn->setDisabled(true);
	ui.sendFileBtn->setDisabled(true);
	ui.sendImgBtn->setDisabled(true);
	ui.sendMsgBtn->setDisabled(true);
	ui.settingBtn->setDisabled(true);
}

void OneRoomClient::setButtonEnable()
{
	ui.logOutBtn->setEnabled(true);
	ui.msgHistoryBtn->setEnabled(true);
	ui.sendFileBtn->setEnabled(true);
	ui.sendImgBtn->setEnabled(true);
	ui.sendMsgBtn->setEnabled(true);
	ui.settingBtn->setEnabled(true);
}

void OneRoomClient::logout()
{
	this->hide();
	loginWindow->show();
	loginWindow->setFocus();
	disconnect(&this->socket, &Socket::getNewmessage, this, &OneRoomClient::on_package_arrived);
	disconnect(this->settingBoard->changePasswordWindow, &ChangePasswordWindow::new_password, this, &OneRoomClient::send_password_setting);
	disconnect(this, &OneRoomClient::change_password_result, this->settingBoard->changePasswordWindow, &ChangePasswordWindow::handle_password_result);
	disconnect(&this->socket, &Socket::sock_error_occurred, this, &OneRoomClient::handle_socket_error);

	connect(&this->socket, &Socket::getNewmessage, this->loginWindow, &LoginWindow::ReceivePack);
	connect(this->settingBoard->changePasswordWindow, &ChangePasswordWindow::new_password, this->loginWindow, &LoginWindow::handle_new_password);
	connect(this->loginWindow, &LoginWindow::change_password_result, this->settingBoard->changePasswordWindow, &ChangePasswordWindow::handle_password_result);
	connect(this->loginWindow->tcpclient, &Socket::sock_error_occurred, this->loginWindow, &LoginWindow::handle_socket_error);
}
