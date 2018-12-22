#pragma once
#include <winsock2.h>
#pragma comment(lib, "ws2_32")

#define MAX_USERNAME_SIZE 20	// ��û�����С(��\0)
#define PACKAGE_HEAD_SIZE 8		
#define PACKAGE_DATA_MAX_SIZE 2048	// �����С
#define MIN_PASSWORD_SIZE 10	// ������볤��
#define MAX_PASSWORD_SIZE 20	// ��\0
//#define SSL

// ��Ϣ����
#define DATA_TYPE_SINGLE 0x00
#define DATA_TYPE_GROUP 0x02
#define DATA_TYPE_ALL 0x01
#define DATA_TYPE_TEXT 0x00
#define DATA_TYPE_PICTURE 0x10
#define DATA_TYPE_FILE 0x20

// ��������
#define CLIENT_REQUIRE_LOGIN 0x00
#define CLIENT_REQUIRE_LOGOUT 0x03
#define CLIENT_CHANGE_PASSWORD 0x07
#define CLIENT_CHANGE_SETTING 0x08

// ����˷����¼�
#define SERVER_ACK_MESSAGE 0x01
#define SERVER_RETURN_SETTING 0x09
#define SERVER_RETURN_ERROR_C 0x0A	// ���󱣳�����
#define SERVER_RETUEN_ERROR_D 0x0B	// ����Ͽ�����
#define SERVER_RETURN_USERLIST 0x0C
#define SERVER_ACK_CHANGE_PASSWORD 0x0D

// �������
#define NO_SUCH_USER 0x00	// �����ڵ��û�
#define ENFORCE_CHANGE_PASSWORD 0x01 // Ҫ���������
#define PASSWORD_ERROR 0x02 // �������
#define ENFORCE_OFFLINE 0x03 // ǿ������
#define SEND_MESSAGE_FAIL 0x04 // ����ʧ��

// �ͻ�����
#define ERROR 0
#define OK 1

struct PackageHead {
	unsigned char isData;
	unsigned char type;
	unsigned char isCut;
	unsigned char seq;
	int dataLen;
};

const PackageHead SingleHead = {
	1,DATA_TYPE_SINGLE,0,0,0
};

const PackageHead AllHead = {
	1,DATA_TYPE_ALL,0,0,0
};

const PackageHead GroupHead = {
	1,DATA_TYPE_GROUP,0,0,0
};

const PackageHead SendTest = {
	0,0x00,0,0,0
};
