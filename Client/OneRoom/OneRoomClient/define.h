#pragma once
#include <winsock2.h>
#pragma comment(lib, "ws2_32")

#define MAX_USERNAME_SIZE 20	// 最长用户名大小(含\0)
#define PACKAGE_HEAD_SIZE 8		
#define PACKAGE_DATA_MAX_SIZE 2048	// 最长包大小
#define MIN_PASSWORD_SIZE 10	// 最短密码长度
#define MAX_PASSWORD_SIZE 20	// 含\0
//#define SSL

// 消息类型
#define DATA_TYPE_SINGLE 0x00
#define DATA_TYPE_GROUP 0x02
#define DATA_TYPE_ALL 0x01
#define DATA_TYPE_TEXT 0x00
#define DATA_TYPE_PICTURE 0x10
#define DATA_TYPE_FILE 0x20

// 控制类型
#define CLIENT_REQUIRE_LOGIN 0x00
#define CLIENT_REQUIRE_LOGOUT 0x03
#define CLIENT_CHANGE_PASSWORD 0x07
#define CLIENT_CHANGE_SETTING 0x08

// 服务端返回事件
#define SERVER_ACK_MESSAGE 0x01
#define SERVER_RETURN_SETTING 0x09
#define SERVER_RETURN_ERROR_C 0x0A	// 错误保持连接
#define SERVER_RETUEN_ERROR_D 0x0B	// 错误断开连接
#define SERVER_RETURN_USERLIST 0x0C
#define SERVER_ACK_CHANGE_PASSWORD 0x0D

// 错误代码
#define NO_SUCH_USER 0x00	// 不存在的用户
#define ENFORCE_CHANGE_PASSWORD 0x01 // 要求更改密码
#define PASSWORD_ERROR 0x02 // 密码错误
#define ENFORCE_OFFLINE 0x03 // 强制下线
#define SEND_MESSAGE_FAIL 0x04 // 发送失败

// 客户端用
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
