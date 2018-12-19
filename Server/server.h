#ifndef __SERVER_H__
#define __SERVER_H__

#include <iostream>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <list>
#include <map>
#include <utility>

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <dirent.h>

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>

#include <stdbool.h>
#include <mysql.h>


#define TAIHOUDAISUKI           1
#define MAXCONNNUM              64
#define NOTLOGGEDIN             -1
#define ERRSOCKET               -1

#define MAXACCLEN               20
#define MAXPASSLEN              20
#define MAXNAMELEN              20
#define MD5LEN                  16
#define MAXDATALEN              1500
#define CTRLPACKLEN             8
#define MAXBUFFERLEN            1520

// from client
#define LOG_IN_REQ              0x00
#define LOG_OUT_REQ             0x03
#define CHG_PSSW_REQ            0x07
#define CHG_SET_REQ             0x08
// from server
#define REQ_SUCC                0x01
#define REQ_SET                 0x09
#define REQ_ERR_CONN            0x0A
#define REQ_ERR_DISC            0x0B
#define REQ_USER                0x0C
#define PASS_CHG_SUCC           0x0D
// error number
#define ACC_NOT_EXIST           0x0
#define CHG_PASS                0x1
#define PASS_ERR                0x2
#define KICK_OFF                0x3
#define SND_ERR                 0x4


using namespace std;

enum _serverres
{
    SERVER_DEFAULT, SERVER_ERROR, SERVER_FINISH
}

class ClientInfo
{
public:
    enum _readres
    {
        READ_DEFAULT, READ_FINISH, READ_HUNGUP, READ_CLOSE, READ_ERROR
    };
    enum _writeres
    {
        WRITE_DEFAULT, WRITE_FINISH, WRITE_HUNGUP, WRITE_CLOSE, WRITE_ERROR
    };

    ClientInfo();
    ClientInfo(const int fd, const char* const ip, const int port);
    ~ClientInfo();

    bool operator ==(const ClientInfo &rhs) const
    {
        return fd == rhs.fd;
    }
    ClientInfo& operator =(const ClientInfo &rhs)
    {
        sockfd = rhs.sockfd;
        userid = rhs.userid;
        memcpy(ip, rhs.ip, INET_ADDRSTRLEN + 1);
        port = rhs.port;

        bufflen = rhs.bufflen;
        buffp = rhs.buffp;
        memcpy(buff, rhs.buff, bufflen);
        return *this;         
    }

    void set(const int userid);
    void reset();

    int Read_Bitstream();
    int Write_Bitstream();

    void Load_Buffer(void * const dst, const int len);
    void reset_read_buff(const int len);
    void Save_Buffer(void * const src, const int len);

    int Rcv(void *const dst, const int len);
    int Snd(void *const src, const int len);

    int is_Logged_in() const;

    int sockfd;
    int userid;
    char ip[INET_ADDRSTRLEN + 1];
    int port;
    int bufflen[2]; // read-0 write-1

private:
    int buffp[2]; // read-0 write-1
    char buff[2][MAXBUFFERLEN]; // read-0 write-1
}

class ServerSock: private ClientInfo
{
public:
    ServerSock();
    ~ServerSock();

    int Server_Start();

private:
    enum _messagetpye
    {
        TEXT_TYPE, GRAPH_TYPE, FILE_TYPE
    };
    enum _sendtype
    {
        P_2_P, P_2_A, P_2_G
    };

    struct CtrlPack
    {
        unsigned char isData;
        unsigned char Type;
        unsigned char isCut;
        unsigned char Seq;
        int Datalen;
    };

    int log_in_request(ClientInfo *client);
    int log_out_request(ClientInfo *client);
    int change_password_request(ClientInfo *client)
    int change_setting_request(ClientInfo *client);
    int transmit_request(ClientInfo *client, Packet *pack);

    int userlist_request_to_all();
    int log_out_unexpected(ClientInfo *client);

    int mysql_get_uid(const char* const account);
    void mysql_get_account(const int uid, char* const account)
    void mysql_get_username(const int uid, char* const username);
    int mysql_compare_password(const char* const pass_MD5);
    bool mysql_check_first_time(const int uid);

    sockaddr_in serveraddr;
    int listenfd;

    list<ClientInfo> clientlist;
    int clicnt;
    map<int, ClientInfo*> userlist; // first-userid, second-sockfd

    int userreq; // 0-noneed 1-resend

    MYSQL     *mysql;   
    MYSQL_RES *result; 
    MYSQL_ROW  row;
};

#endif
