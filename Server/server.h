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
#define PACKETLEN               1508
#define MAXBUFFERLEN            2000

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
        DEFAULT_MESSAGE, TEXT_TYPE, GRAPH_TYPE, FILE_TYPE
    };
    enum _sendtype
    {
        DEFAULT_RECEIVER, P_2_P, P_2_G, P_2_A
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

    sockaddr_in serveraddr;
    int listenfd;

    list<ClientInfo> clientlist;
    int clicnt;
    map<int, ClientInfo*> userlist; // first-userid, second-sockfd

    int userreq; // 0-noneed 1-resend
};

#endif
