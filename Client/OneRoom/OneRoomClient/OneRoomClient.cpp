#include "OneRoomClient.h"
#include <qdatetime.h>
#include <qtextcodec.h>
#include <qmessagebox.h>
#include "Socket.h"

OneRoomClient::OneRoomClient(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	ui.userListWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);	// ���ö�ѡ
	// ���ñ���
	QTextCodec::setCodecForLocale(QTextCodec::codecForName("GBK"));

	// test
	userList.append(new UserInfo("Megumi", "Kagaya", QString::number(QDateTime::currentDateTime().toTime_t())));
	userList.append(new UserInfo(QString::fromLocal8Bit("����"), "test", QString::number(QDateTime::currentDateTime().toTime_t())));
	userList.append(new UserInfo("1234567890", "number", QString::number(QDateTime::currentDateTime().toTime_t())));

	updateUserList();
	// connect
	connect(ui.sendMsgBtn, SIGNAL(clicked()), this, SLOT(on_sendMsgBtn_clicked));
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
		QMessageBox::warning(this, tr("FBI Warning"), tr("��ѡ�����û�"));
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

	// �������
	QByteArray msgByteArray = msg.toLocal8Bit();	// תΪ�����ʽ
	int dataSize = ((nCount + 1) * USERNAME_BUFF_SIZE) + msgByteArray.length() + 1;	// ���ݲ��ֺ�β��
	char* data = new(std::nothrow) char[dataSize];
	if (data == NULL) {
		QMessageBox::warning(this, tr("FBI Warning"), tr("new error"));
		return;
	}
	int length = addTargetUserData(itemList, data, nCount);
	

	// ����package
	bool isSending = true;	// ����״̬

	if (isSending) {
		handleMessageTime(time);

		Message* message = new Message(ui.msgListWidget->parentWidget());
		QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
		handleMessage(message, item, msg, time, Message::User_Me);
		// ����send����
		int ret = socket.SendMessage(head, data);
		if(ret)	// ���÷��ͳɹ�
			message->setTextSuccess();
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
		memcpy(&data[i * USERNAME_BUFF_SIZE], userNameByteArray.data(), USERNAME_BUFF_SIZE);
		length += USERNAME_BUFF_SIZE;
	}

	// ��ӵ�ǰ�û�����
	userNameByteArray = currentUser.userName().toLocal8Bit();
	memcpy(&data[nCount * USERNAME_BUFF_SIZE], userNameByteArray.data(), USERNAME_BUFF_SIZE);
	length += USERNAME_BUFF_SIZE;

	return length;
}


void OneRoomClient::on_newMsg_come(QString msg, QString sendTime)
{

}

/* User List View */
// �����û��б�
void OneRoomClient::updateUserList()
{
	UserInfo *i;
	foreach(i, userList) {	// �����û��б�
		UserInfo *info = new UserInfo(ui.userListWidget->parentWidget());
		QListWidgetItem *item = new QListWidgetItem(ui.userListWidget);
		handleUserinfo(info, item, i->nickName(), i->userName(), i->loginTime());
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

void OneRoomClient::resizeEvent(QResizeEvent *event)
{
	Q_UNUSED(event);	// ȡ��δʹ�ñ�������

	for (int i = 0; i < ui.msgListWidget->count(); i++) {
		Message *message = (Message*)ui.msgListWidget->itemWidget(ui.msgListWidget->item(i));
		QListWidgetItem *item = ui.msgListWidget->item(i);
		handleMessage(message, item, message->text(), message->time(), message->userType());
	}
}
