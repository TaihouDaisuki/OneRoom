#include "server.h"

using namespace std;

//==========ClientInfo==========
ClientInfo::ClientInfo(const int _fd = 0, const char* const _ip, const int _port)
{
    strcpy(ip, _ip);
    port = _port;

    sockfd = _fd;
    userid = NOTLOGGEDIN;
    bufflen = buffp = 0;
    buff = NULL;
}
ClientInfo::~ClientInfo()
{
    close(sockfd);
    if(buff != NULL)
        delete buff;
}

void ClientInfo::set(const int _userid)
{
    userid = _userid;
    bufflen = buffp = 0;
    if(buff != NULL)
        delete buff;
}
void ClientInfo::reset()
{
    userid = NOTLOGGEDIN;
    bufflen = buffp = 0;
    if(buff != NULL)
        delete buff;
}

int ClientInfo::is_Logged_in() const
{
    return userid != NOTLOGGEDIN;
}

int ClientInfo::Read_Bitstream()
{
    int _rs;
    do
    {
        errno = 0;
        _rs = read(sockfd, buff + bufferp, bufflen - buffp);
        buffp += (_rs > 0 ? _rs : 0);
        if(buffp == bufflen)
            break;
    } while (_rs < 0 && errno == EINTR);

    if(_rs == 0)
        return READCLOSE;
    else if (_rs < 0)
    {
        if(errno == EWOULDBLOCK || errno == EAGAIN)
            return READHUNGUP;
        cerr << "sockfd[" << sockfd "] Read error: " << strerror(errno) << endl;
        cerr << "It's belong to userid = " << userid << endl << endl;
        return READERROR;
    }
    return READFINISH;
}
int ClientInfo::Write_Bitstream()
{
    int _ws;
    do
    {
        errno = 0;
        _ws = write(sockfd, buffer + buffer_p, Len - buffer_p);
        buffp += (_ws > 0 ? _ws : 0);
        if(buffp == bufflen)
            break;
    } while(_ws < 0 && errno == EINTR);

    if(_ws == 0)
        return WRITECLOSE;
    else if(_ws < 0)
    {
        if(errno == EWOULDBLOCK || errno == EAGAIN)
            return WRITEHANGUP;
        cerr << "sockfd[" << sockfd "] Write error: " << strerror(errno) << endl;
        cerr << "It's belong to userid = " << userid << endl << endl;
        return WRITEERROR;
    }
    return WRITEFINISH;
}

//==========ServerSock==========
ServerSock::ServerSock()
{
    //==========sock==========
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		cerr << "Create socket error: "<< strerror(errno) << "(errno: "<< errno <<")" << endl;
		exit(EXIT_FAILURE);
	}

	int flag = fcntl(listenfd, F_GETFL, 0);
	fcntl(listenfd, F_SETFL, flag | O_NONBLOCK);

	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&opt, sizeof(opt));
	
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	if (inet_pton(AF_INET, _ip, &serveraddr.sin_addr) <= 0) 
	{
		cerr << "inet_pton error for " << _ip << endl;
		exit(EXIT_FAILURE);
	}
	serveraddr.sin_port = htons(_port);

	//==========bind==========
	if(bind(listenfd, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) 
    {
        cerr << "bind socket error: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    //==========listen==========
    if(listen(sock, MAXCONNNUM) < 0)
	{
		cerr << "listen socket error: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
	}

    //==========reset==========
    clisockp.clear();
    clicnt = 0;

    userlist.clear();
}
ServerSock::~ServerSock()
{
    // save data into database
    // close all socket
    list<ClientInfo *>::iterator it;
    for(it = clientp.begin(); it != clientp.end(); ++it)
    {
        delete *it;
        list.erase(it);
    }
        
    return;
}

int ServerSock::Server_Start()
{
    cout << "===================================" << endl;
    cout << "=  Welcome to use OneRoom Server  =" << endl;
    cout << "= The server now is under working =" << endl;
    cout << "===================================" << endl;
    cout << endl << endl;

    fd_set sockfds, readfds, writefds;

	FD_ZERO(&sockfds);
	FD_SET(listenfd, &sockfds);

    while(TAIHOUDAISUKI)
    {
        readfds = sockfds;
        writefds = sockfds;
        int retsel = select(FD_SETSIZE, &readfds, &writefds, NULL, NULL);
		if (retsel <= 0)
		{
			cerr << "Server Select error: " << strerror(errno) << endl;
			return;
		}

        // listen socket
        if(FD_ISSET(fd, &readfds))
        {
            int clientfd = 0, flag;
            struct sockaddr_in cliaddr;
            socklen_t clilen = sizeof(cliaddr);
            while (clientfd >= 0)
            {
                if ((clientfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
                    break;

                //解析客户端地址
                char ipbuff[INET_ADDRSTRLEN + 1] = {0};
                inet_ntop(AF_INET, &cliaddr.sin_addr, ipbuff, INET_ADDRSTRLEN);
                uint16_t cli_port = ntohs(cliaddr.sin_port);
                cout << "Server: Detect a connection from " << ipbuff << ", port " << cli_port << endl;

                ClientInfo* _clientp = new(nothrow) ClientInfo(clientfd, ipbuff, cli_port);
                if(_clientp == NULL)
                {
                    cerr << "Server: No more memory for this client" << endl << endl;
                    close(clientfd);
                    continue;
                }
                clientp.push_back(_clientp);

                fdList[i] = clientfd;
                flag = fcntl(clientfd, F_GETFL, 0);
                fcntl(clientfd, F_SETFL, flag | O_NONBLOCK);
                FD_SET(clientfd, &sockfds);
                cout << "Server: Accept connection successfully." << endl << endl;
            }
        }

        // client socket
        if(clientp.empty())
            continue;

        iterator<>
        list<ClientInfo *>::iterator it;
        for(it = clientp.begin(); it != clientp.end(); ++it)
        {
            ClientInfo* curclip = *it;
            if(FD_ISSET(curclip->sockfd, &readfds))
            {
                int read_res = curclip->Read_Bitstream();
                if(read_res == READHUNGUP)
                    continue;
                if(read_res == READERROR || read_res == READCLOSE)
                {
                    Log_out(curclip);
                    delete *curclip;
                    --
                    continue;
                }
                if(read_res == READFINISH)
                {
                    // save into database?
                    // exchange information?
                    // log in?
                    // log out?
                    // ......
                }
            }
            if(FD_ISSET(curclip->sockfd, &writefds) && /*(***)*/)
            {
                int write_res = curclip->Write_Bitstream();
                if(read_res == WRITEHUNGUP)
                    continue;
                if(read_res == WRITEERROR || read_res == WRITECLOSE)
                {
                    if(curclip->is_Logged_in())
                        Log_out(curclip->userid);
                    delete *curclip;
                    continue;
                }
                if(read_res == READFINISH)
                {
                    // save into database?
                    // exchange information?
                    // log in?
                    // log out?
                    // ......
                }
            }
        }
    }
}

void ServerSock::Log_in(const char* const username, ClientInfo * const curclip)
{
    int userid;
    // get userid from database by username
    map<int, list<ClientInfo*>::iterator>::iterator it = userlist.serach(userid);
    if(it != map.end()) // has logged in
    {
        ClientInfo* loggedin = it->second;
        // send message let it log out
        cout << "user: " << username << "[id = " << userid << " ] is kick off from [ip = " 
            << loggedin->ip << ", port = " << loggedin->port << endl;
        loggedin->reset();
        userlist.erase(it);
    }
    cout << "user: " << username << "[id = " << userid << " ] log in from [ip = " 
        << curclip->ip << ", port = " << curclip->port << endl << endl;
    curclip->set(userid);
    userlist.insert(make_pair(userid, curclip));

}
void ServerSock::Log_out(ClientInfo * const curclip)
{
    char *username;
    // get username from database by userid
    if(!curclip->is_Logged_in())
        return;
    cout << "user: " << username << "[id = " << userid << "] log out from [ip = " 
        << curclip->ip << ", port = " << curclip->port << endl << endl;
    curclip->reset();
    userlist.erase(curclip->userid);
}
