#pragma once
#define _MESSAGE_IS_CORRECT   -1
#define _MESSAGE_IS_ERROR_CON -2
#define _MESSAGE_IS_ERROR_DIS -3


#define SINGLE_TEXT 1





#define _MESSAGE_IS_SINGLE_TEXT 0x00





#define _SERVER_RETURN_CORRECT 0x01
//0x01 服务器返回正确，若前一步操作为登陆，则返回的数据段中包含服务端保存的用户配置
#define _SERVER_SEND_SETTINGS 0x09
//0x09 服务端发送用户现在设置
#define _SERVER_ERROR_CONNECT 0x0A
//0x0A 服务端返回错误，不断开连接
#define _SERVER_ERROR_DISCONNECT 0x0B
//0x0B 服务端返回错误，要求断开连接
#define _SERVER_SEND_LIST 0x0C
//0x0C 服务端发送用户列表






struct PackageHead {
	unsigned char IsData;
	unsigned char Type;
	unsigned char IsCut;
	unsigned char Seq;
	int DataLen;
};


const PackageHead SingleText = {
	1,0x00,0,0,0
};

const PackageHead AllText = {
	1,0x01,0,0,0
};

const PackageHead GroupText = {
	1,0x02,0,0,0
};

const PackageHead SendPassWord = {
	0,0x00,0,0,0
};

const PackageHead SendTest = {
	0,0x0B,0,0,0
};