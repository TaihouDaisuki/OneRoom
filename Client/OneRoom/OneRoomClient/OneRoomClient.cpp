#include "OneRoomClient.h"
#include <qdatetime.h>

OneRoomClient::OneRoomClient(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	// connect
	connect(ui.sendMsgBtn, SIGNAL(clicked()), this, SLOT(on_sendMsgBtn_clicked));
} 

/* �¼������� */
void OneRoomClient::on_sendMsgBtn_clicked()
{
	QString msg = ui.msgTextEdit->toPlainText();	// ��ȡ�������Ϣ
	ui.msgTextEdit->setPlainText("");	// ��������
	QString time = QString::number(QDateTime::currentDateTime().toTime_t());	// ��ȡʱ���

	bool isSending = true;	// ����״̬

	if (ui.msgListWidget->count() % 2) {	// �����ã������շ���
		if (isSending) {
			handleMessageTime(time);

			Message* message = new Message(ui.msgListWidget->parentWidget());
			QListWidgetItem* item = new QListWidgetItem(ui.msgListWidget);
			handleMessage(message, item, msg, time, Message::User_Me);
		}
		else {
			bool isOver = true;
			for (int i = ui.msgListWidget->count() - 1; i > 0; i--) {
				Message* message = (Message*)ui.msgListWidget->itemWidget(ui.msgListWidget->item(i));	// ��ǰѡȡ����Ϣ
				if (message->text() == msg) {	// ����շ��͵���Ϣ
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
	ui.msgListWidget->setCurrentRow(ui.msgListWidget->count() - 1);	// ���õ�ǰ����

}

void OneRoomClient::on_newMsg_come(QString msg, QString sendTime)
{

}

/* User List View */
// �����û��б�
void OneRoomClient::updateUserList()
{
	UserInfo i;
	foreach(i, userList) {	// �����û��б�
		UserInfo *info = new UserInfo(ui.userListWidget->parentWidget());
		QListWidgetItem *item = new QListWidgetItem(ui.userListWidget);
		handleUserinfo(info, item, i.nickName(), i.userName(), i.loginTime());
	}
}

// ������Ϣ����ӵ�list��
void OneRoomClient::handleUserinfo(UserInfo *userInfo, QListWidgetItem *item, QString nickName, QString userName, QString loginTime)
{
	userInfo->setFixedWidth(ui.userListWidget->width());
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
