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
	ui.userListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);	// ���ö�ѡ
	ui.msgListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));	// ���ñ��ر���
	ui.msgTextEdit->setFontFamily("MicrosoftYaHei");
	ui.msgTextEdit->setFontPointSize(12);
	ui.msgTextEdit->installEventFilter(this);
	setStyleSheet("background: rgb(33,33,33);border-width:0;border-style:outset;border:1px solid grey;color:white");


	// ��ʼ���Ӵ���
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
		Message *message = (Message *)ui.msgListWidget->itemWidget(ui.msgListWidget->item(i));	// ���ص�ǰitem��Ӧ��widget
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
/* �¼������� */
void OneRoomClient::on_sendMsgBtn_clicked()
{
	setButtonDisable();
	QString msg = ui.msgTextEdit->toPlainText();	// ��ȡ�������Ϣ
	ui.msgTextEdit->setPlainText("");	// ��������
	QString time = QString::number(QDateTime::currentDateTime().toTime_t());	// ��ȡʱ���

	if (msg == "")	// ����Ϣֱ�ӷ���
	{
		setButtonEnable();
		return;
	}

	// �ж���Ŀ
	QList<QListWidgetItem *> itemList = ui.userListWidget->selectedItems();	// ����ѡ�е���Ŀ
	int nCount = itemList.count();
	if (nCount < 1) {
		// ��ѡ���û�����ʾ��ѡ���Ͷ���
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("��ѡ�����û�"));
		setButtonEnable();
		return;
	}

	// �ж���Ϣ���ͷ�ʽ
	unsigned char sendType = 0;
	if (nCount == ui.userListWidget->count())
		sendType = DATA_TYPE_ALL;
	else if (nCount == 1)
		sendType = DATA_TYPE_SINGLE;
	else
		sendType = DATA_TYPE_GROUP;

	// �������ݰ�
	QByteArray msgByteArray = msg.toLocal8Bit();	// תΪ�����ʽ
	// ���ͷ��
	PackageHead head;
	char* data = NULL;
	int dataSize = 0;

	switch (sendType) {
	case DATA_TYPE_SINGLE: {
		memcpy(&head, &SingleHead, sizeof(PackageHead));
		dataSize = MAX_USERNAME_SIZE + msgByteArray.length() + 1;	// ���ݲ��ֺ�β��
		data = new(std::nothrow) char[dataSize];
		if (data == NULL) {
			QMessageBox::warning(this, tr("FBI Warning"), tr("new error"));
			return;
		}
		// ����Ŀ���û���
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
		// �������
		dataSize = 1 + (nCount * MAX_USERNAME_SIZE) + msgByteArray.length() + 1;	// ���ݲ��ֺ�β��
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
		dataSize = MAX_USERNAME_SIZE + msgByteArray.length() + 1;	// ���ݲ��ֺ�β��
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

	// ����package
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
	// ����send����
	socket.Send(head, data);
	delete data;
	sendMsgQueue.push_back(message);

	//�����ʱ��Ŀ�б�
	itemList.clear();
	ui.msgListWidget->setCurrentRow(ui.msgListWidget->count() - 1);	// ���õ�ǰ����
	setButtonEnable();


	on_userListWidget_itemDoubleClicked(ui.userListWidget->selectedItems().at(0));
}

void OneRoomClient::on_sendFileBtn_clicked()
{
	QList<QListWidgetItem *> itemList = ui.userListWidget->selectedItems();	// ����ѡ�е���Ŀ
	int nCount = itemList.count();
	if (nCount < 1) {
		// ��ѡ���û�����ʾ��ѡ���Ͷ���
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("��ѡ�����û�"));
		setButtonEnable();
		return;
	}
	if (nCount > 1) {
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("��ѡ�񵥸��û������ļ�"));
		setButtonEnable();
		return;
	}


	setButtonDisable();
	//�����ļ��Ի�����
	QFileDialog *fileDialog = new QFileDialog(this);
	//�����ļ��Ի������
	fileDialog->setWindowTitle(tr("ѡ�����ļ�"));
	//����Ĭ���ļ�·��
	fileDialog->setDirectory(".");
	//�����ļ�������
	fileDialog->setNameFilter(tr("*.*"));
	//������ͼģʽ
	fileDialog->setViewMode(QFileDialog::Detail);
	//ѡ����ļ���·��
	QStringList fileNames;
	if (fileDialog->exec())
	{
		fileNames = fileDialog->selectedFiles();
	}
	// send
	QString msg = QString::fromLocal8Bit("[�ļ�]");

	if (fileNames.count() < 0)
	{
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("δѡ���ļ�"));
		setButtonEnable();
		return;
	}
	QString filename_temp = fileNames.at(0);



	setButtonEnable();
}

void OneRoomClient::on_sendImgBtn_clicked()
{
	setButtonDisable();
	//�����ļ��Ի�����
	QFileDialog *fileDialog = new QFileDialog(this);
	//�����ļ��Ի������
	fileDialog->setWindowTitle(tr("ѡ��ͼƬ"));
	//����Ĭ���ļ�·��
	fileDialog->setDirectory(".");
	//�����ļ�������
	fileDialog->setNameFilter(tr("Images(*.png *.jpg *.jpeg *.bmp)"));
	//������ͼģʽ
	fileDialog->setViewMode(QFileDialog::Detail);
	//��ӡ����ѡ���ͼƬ��·��
	QStringList fileNames;
	if (fileDialog->exec())
	{
		fileNames = fileDialog->selectedFiles();
	}
	// send
	QString msg = QString::fromLocal8Bit("[ͼƬ]");
	QString time = QString::number(QDateTime::currentDateTime().toTime_t());	// ��ȡʱ���

	// �ж���Ŀ
	QList<QListWidgetItem *> itemList = ui.userListWidget->selectedItems();	// ����ѡ�е���Ŀ
	int nCount = itemList.count();
	if (nCount < 1) {
		// ��ѡ���û�����ʾ��ѡ���Ͷ���
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("��ѡ�����û�"));
		setButtonEnable();
		return;
	}

	// �ж���Ϣ���ͷ�ʽ
	unsigned char sendType = 0;
	if (nCount == ui.userListWidget->count())
		sendType = DATA_TYPE_ALL;
	else if (nCount == 1)
		sendType = DATA_TYPE_SINGLE;
	else
		sendType = DATA_TYPE_GROUP;

	// �������ݰ�
	QByteArray msgByteArray = msg.toLocal8Bit();	// תΪ�����ʽ
	// ���ͷ��
	PackageHead head;
	char* data = NULL;
	int dataSize = 0;

	switch (sendType) {
	case DATA_TYPE_SINGLE: {
		memcpy(&head, &SingleHead, sizeof(PackageHead));
		dataSize = MAX_USERNAME_SIZE + msgByteArray.length() + 1;	// ���ݲ��ֺ�β��
		data = new(std::nothrow) char[dataSize];
		if (data == NULL) {
			QMessageBox::warning(this, tr("FBI Warning"), tr("new error"));
			return;
		}
		// ����Ŀ���û���
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
		// �������
		dataSize = 1 + (nCount * MAX_USERNAME_SIZE) + msgByteArray.length() + 1;	// ���ݲ��ֺ�β��
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
		dataSize = MAX_USERNAME_SIZE + msgByteArray.length() + 1;	// ���ݲ��ֺ�β��
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

	// ����package
	handleMessageTime(time);

	Message* message = new Message(ui.msgListWidget->parentWidget());
	QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
	handleMessage(message, item, msg, time, Message::User_Me, Message::Msg_Img, fileNames[0]);
	// ����send����
	//socket.Send(head, data);
	delete data;
	sendMsgQueue.push_back(message);

	//�����ʱ��Ŀ�б�
	itemList.clear();
	ui.msgListWidget->setCurrentRow(ui.msgListWidget->count() - 1);	// ���õ�ǰ����

	setButtonEnable();
}

void OneRoomClient::on_settingBtn_clicked()
{
	settingBoard->show();
}

void OneRoomClient::on_package_arrived(PackageHead head, char* const data)
{
	// ����
	if (head.isData == 1)
	{
		switch (head.type & 0xf0) {
		case DATA_TYPE_TEXT: {
			QString time = QString::number(QDateTime::currentDateTime().toTime_t());	// ��ȡʱ���
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
			QString time = QString::number(QDateTime::currentDateTime().toTime_t());	// ��ȡʱ���
			handleMessageTime(time);
			QString msg = QString::fromLocal8Bit(data + 20);

			Message* message = new Message(ui.msgListWidget->parentWidget());
			QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
			handleMessage(message, item, msg, time, Message::User_He);
			break;
		}
		case DATA_TYPE_FILE: {
			QString time = QString::number(QDateTime::currentDateTime().toTime_t());	// ��ȡʱ���
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
	// ����
	else
	{
		switch (head.type) {
		case SERVER_ACK_MESSAGE: {
			// �յ���Ϣȷ�ϰ���ȷ����Ϣ���ͳɹ�
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
			// ȷ�ϵ�½�ɹ�
			break;
		case SERVER_RETURN_ERROR_C:
			if (data[0] == SEND_MESSAGE_FAIL) {
				// ������Ϣʧ��
				QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("��Ϣ����ʧ��"));
			}
			else if (data[0] == PASSWORD_ERROR) {
				// ������ʧ�ܣ�ԭ�������
				emit change_password_result(ERROR);
			}
			else {
				// nothing
			}
			break;
		case SERVER_RETUEN_ERROR_D:
			socket.disconnect();	// �Ͽ�����
			if (data[0] == ENFORCE_OFFLINE) {
				// ǿ������
				logout();	// �ǳ�
			}
			else {
				// nothing
			}
			break;
		case SERVER_RETURN_USERLIST: {
			// �����û��б�
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

// ��itemList�е��û�����ȡ�������ϵ�ǰ�û����û�����ת�����͸�ʽ��dataָ���ַ��ʼ����, �������볤��
int OneRoomClient::addTargetUserData(QList<QListWidgetItem *> &itemList, char* const data, int nCount)
{
	//char* data = new char[(nCount + 1) * USERNAME_BUFF_SIZE];
	UserInfo *user;
	QByteArray userNameByteArray;
	int length = 1;
	
	// ���ֽ�Ϊ��������
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
// �����û��б�
void OneRoomClient::updateUserList()
{
	UserInfo i;
	ui.userListWidget->clear();	// ����б�
	foreach(i, userList) {	// �����û��б�
		UserInfo *info = new UserInfo(ui.userListWidget->parentWidget());
		QListWidgetItem *item = new QListWidgetItem(ui.userListWidget);
		handleUserinfo(info, item, i.nickName(), i.userName(), i.loginTime());
	}
}

// ������Ϣ����ӵ�list��
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
// ������Ϣͼ�����Բ�����list
void OneRoomClient::handleMessage(Message *message, QListWidgetItem *item, QString text, QString time, Message::User_Type type, Message::Msg_Type msgType, QString imgPath)
{
	message->setFixedWidth(ui.msgListWidget->width() - 10);
	if (msgType == Message::Msg_Img) {
		message->setMsgType(msgType);
		message->setImgPath(imgPath);
	}
	QSize size = message->fontRect(text);	// ��ȡ���ݴ�С
	item->setSizeHint(size);	// ���óߴ�
	message->setText(text, time, size, type, msgType, imgPath);
	ui.msgListWidget->setItemWidget(item, message);
}

// ������Ϣʱ��
void OneRoomClient::handleMessageTime(QString curMsgTime)
{
	bool isShowTime = false;
	if (ui.msgListWidget->count() > 0) {
		// �����item
		QListWidgetItem *lastItem = ui.msgListWidget->item(ui.msgListWidget->count() - 1);	// ��ȡ���һ��Item
		Message *message = (Message *)ui.msgListWidget->itemWidget(lastItem);	// ���ص�ǰitem��Ӧ��widget
		int lastTime = message->time().toInt();	// ��ȡ���һ����Ϣ�ķ���ʱ��
		int curTime = curMsgTime.toInt();
		isShowTime = ((curTime - lastTime) > 60);	// ����������һ����Ϣ����ʱ�䳬��һ����������ʾʱ��
	}
	else {
		isShowTime = true;	// û����Ϣ�������ʾʱ��
	}

	if (isShowTime) {
		Message *messageTime = new Message(ui.msgListWidget->parentWidget());	// ʱ����Ϣ
		QListWidgetItem *itemTime = new QListWidgetItem(ui.msgListWidget);	// ��itme������parent widget��ʼ��
		QSize size = QSize(ui.msgListWidget->width() - 10, 40);
		messageTime->resize(size);	// ���ô�С
		itemTime->setSizeHint(size);
		messageTime->setText(curMsgTime, curMsgTime, size, Message::User_Time);	// ����ʱ����Ϣ
		ui.msgListWidget->setItemWidget(itemTime, messageTime);	// add itme and widget to list
	}

}

/* reload event function */
void OneRoomClient::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);	// ȡ��δʹ�ñ�������

	for (int i = 0; i < ui.msgListWidget->count(); i++) {
		Message *message = (Message*)ui.msgListWidget->itemWidget(ui.msgListWidget->item(i));
		QListWidgetItem *item = ui.msgListWidget->item(i);
		handleMessage(message, item, message->text(), message->time(), message->userType(), message->msgType(), message->imgPath());
	}
}

bool OneRoomClient::eventFilter(QObject *obj, QEvent *e)
{
	Q_ASSERT(obj == ui.msgTextEdit);	// ��֤objΪmsgTextEdit
	if (e->type() == QEvent::KeyPress)
	{
		QKeyEvent *event = static_cast<QKeyEvent*>(e);
		if (event->key() == Qt::Key_Return)	// �س�����
		{
			on_sendMsgBtn_clicked(); //������Ϣ�Ĳ�
			return true;
		}
	}
	return false;
}

void OneRoomClient::reshow_mainwindow(QString userName, QString password, int histroyListNum)
{
	// ��ʼֵ����
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
	
	QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("��½�ɹ���"));
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
