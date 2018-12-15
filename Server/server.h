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

#define MAXACCLEN               30
#define MAXPASSLEN              30
#define MAXNAMELEN              30
#define MD5LEN                  16


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
        if(rhs.buff == NULL)
            buff = NULL;
        else
        {
            buff = new(nothrow) char[bufflen];
            if(buff == NULL)
                bufflen = buffp = 0;
            else
                memcpy(buff, rhs.buff, bufflen);
        }   
        return *this;         
    }

    void set(const int userid);
    void reset();

    int Read_Bitstream();
    int Write_Bitstream();
    int is_Logged_in() const;

    int sockfd;
    int userid;
    char ip[INET_ADDRSTRLEN + 1];
    int port;
private:
    int bufflen;
    int buffp;
    char *buff;
}

class ServerSock: private ClientInfo
{
public:
    ServerSock();
    ~ServerSock();

    int Server_Start();
private:
    enum _requesttype
    {
        DEFAULT_REQUEST, LOG_IN, LOG_OUT, CHANGE_PASSWORD, GET_SETTINGS, CHANGE_SETTINGS, TRANSMIT_MSG
    };
    enum _messagetpye
    {
        DEFAULT_MESSAGE, TEXT_TYPE, GRAPH_TYPE, FILE_TYPE
    };
    enum _sendtype
    {
        DEFAULT_RECEIVER, P_2_P, P_2_G, P_2_A
    };

    int log_in_request(ClientInfo *client);
    int log_out_request(ClientInfo *client, list<ClientInfo>::iterator &it);
    int get_setting_request(ClientInfo *client);
    int is_change_setting_request(ClientInfo *client);
    int transmit_request(ClientInfo *client);

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
