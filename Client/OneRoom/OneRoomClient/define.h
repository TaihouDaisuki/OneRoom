#pragma once
#define USERNAME_BUFF_SIZE 20
#define PACKAGE_HEAD_SIZE 8
#define PACKAGE_MAX_SIZE 2048

// 消息类型
#define DATA_TYPE_SINGLE 0x00
#define DATA_TYPE_GROUP 0x01
#define DATA_TYPE_ALL 0x02
#define DATA_TYPE_TEXT 0x00
#define DATA_TYPE_PICTURE 0x10
#define DATA_TYPE_FILE 0x20

// 控制类型
#define CONTROL_TYPE_CLIENT_LOGIN 0x00
#define CONTROL_TYPE_CLIENT_LOGOUT 0x03
#define CONTROL_TYPE_CLIENT_CHANGE 0x00


struct PackageHead {
	unsigned char isData;
	unsigned char type;
	unsigned char isCut;
	unsigned char seq;
	int dataLen;
};

const PackageHead SingleHead = {
	1,0x00,0,0,0
};

const PackageHead AllHead = {
	1,0x01,0,0,0
};

const PackageHead GroupHead = {
	1,0x02,0,0,0
};
