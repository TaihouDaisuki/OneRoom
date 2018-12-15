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


using namespace std;

const char _ip[] = "0.0.0.0";
const int _port = 4347;

enum _serverres
{
    SERVERDEFAULT, SERVERERROR, SERVERFINISH
}

class ClientInfo
{
public:
    enum _readres
    {
        READDEFAULT, READFINISH, READHUNGUP, READCLOSE, READERROR
    };
    enum _writeres
    {
        WRITEDEFAULT, WRITEFINISH, WRITEHUNGUP, WRITECLOSE, WRITEERROR
    };

    ClientInfo(const int fd = 0, const char* const ip, const int port);
    ~ClientInfo();

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
    void Log_in(const char* const username, ClientInfo * const curclip);
    void Log_out(ClientInfo * const curclip);

    sockaddr_in serveraddr;
    int listenfd;

    list<ClientInfo*> clientp;
    int clicnt;
    map<int, ClientInfo*> userlist;
};

#endif
