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
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));	// ���ñ��ر���
	ui.msgTextEdit->setFontFamily("MicrosoftYaHei");
	ui.msgTextEdit->setFontPointSize(12);
	ui.msgTextEdit->installEventFilter(this);
	this->setStyleSheet("background: rgb(33,33,33);border-width:0;border-style:outset;border:1px solid grey;color:white");
	// test
	userList.append(UserInfo("Megumi", "Kagaya", QString::number(QDateTime::currentDateTime().toTime_t())));
	userList.append(UserInfo(QString::fromLocal8Bit("����"), "test", QString::number(QDateTime::currentDateTime().toTime_t())));
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

/* �¼������� */
void OneRoomClient::on_sendMsgBtn_clicked()
{
	QString msg = ui.msgTextEdit->toPlainText();	// ��ȡ�������Ϣ
	ui.msgTextEdit->setPlainText("");	// ��������
	QString time = QString::number(QDateTime::currentDateTime().toTime_t());	// ��ȡʱ���

	if (msg == "")	// ����Ϣֱ�ӷ���
		return;

	// �ж���Ŀ
	QList<QListWidgetItem *> itemList = ui.userListWidget->selectedItems();	// ����ѡ�е���Ŀ
	int nCount = itemList.count();
	if (nCount < 1) {
		// ��ѡ���û�����ʾ��ѡ���Ͷ���
		QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("��ѡ�����û�"));
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
	int dataSize = ((nCount + 1) * MAX_USERNAME_SIZE) + msgByteArray.length() + 1;	// ���ݲ��ֺ�β��
	// ���ͷ��
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

	// �������
	char* data = new(std::nothrow) char[dataSize];
	if (data == NULL) {
		QMessageBox::warning(this, tr("FBI Warning"), tr("new error"));
		return;
	}
	int length = addTargetUserData(itemList, data, nCount);
	// copy message
	memcpy(&data[length], msgByteArray.data(), msgByteArray.length() + 1);

	// ����package
	bool isSending = true;	// ����״̬

	if (true) {//if (ui.msgListWidget->count() % ) {	// �����ã������շ���
		if (isSending) {
			handleMessageTime(time);

			Message* message = new Message(ui.msgListWidget->parentWidget());
			QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
			handleMessage(message, item, msg, time, Message::User_Me);
			// ����send����
			socket.Send(head, data);
			sendMsgQueue.push_back(message);
		}
		//else {
		//	bool isOver = true;
		//	for (int i = ui.msgListWidget->count() - 1; i > 0; i--) {
		//		Message* message = (Message*)ui.msgListWidget->itemWidget(ui.msgListWidget->item(i));	// ��ǰѡȡ����Ϣ
		//		if (message->text() == msg) {	// ����շ��͵���Ϣ
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

		//�����ʱ��Ŀ�б�
		itemList.clear();
		ui.msgListWidget->setCurrentRow(ui.msgListWidget->count() - 1);	// ���õ�ǰ����
	}

}

void OneRoomClient::on_sendFileBtn_clicked()
{
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

}

void OneRoomClient::on_sendImgBtn_clicked()
{
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

}

void OneRoomClient::on_package_arrived(PackageHead head, char* data)
{
	// ����
	if (head.isData == 1)
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
				emit change_password_success();
				break;
			case SERVER_RETURN_SETTING:
				// ȷ�ϵ�½�ɹ�
				break;
			case SERVER_RETURN_ERROR_C:
				if (data[0] == SEND_MESSAGE_FAIL) {
					// ������Ϣʧ��
					QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("��Ϣ����ʧ�ܣ�ָ���û�������"));
				}
				else if (data[0] == PASSWORD_ERROR) {
					// ������ʧ�ܣ�ԭ�������
					QMessageBox::warning(this, tr("FBI Warning"), QString::fromLocal8Bit("ԭ�������"));
					emit change_password_fail();
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
	// ����
	else
	{

	}

}

// ��itemList�е��û�����ȡ�������ϵ�ǰ�û����û�����ת�����͸�ʽ��dataָ���ַ��ʼ����, �������볤��
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

	// ��ӵ�ǰ�û�����
	userNameByteArray = currentUser.userName().toLocal8Bit();
	memcpy(&data[nCount * MAX_USERNAME_SIZE], userNameByteArray.data(), MAX_USERNAME_SIZE);
	length += MAX_USERNAME_SIZE;

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
void OneRoomClient::handleMessage(Message *message, QListWidgetItem *item, QString text, QString time, Message::User_Type type)
{
	message->setFixedWidth(ui.msgListWidget->width() - 10);
	QSize size = message->fontRect(text);	// ��ȡ�ı����С
	item->setSizeHint(size);	// ���óߴ�
	message->setText(text, time, size, type);
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
		handleMessage(message, item, message->text(), message->time(), message->userType());
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