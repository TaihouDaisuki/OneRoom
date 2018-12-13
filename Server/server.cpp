#include "server.h"

using namespace std;

const char _ip[] = "0.0.0.0";
const int _port = 4347;

//==========ClientInfo==========
ClientInfo::ClientInfo()
{
    sockfd = 0;
    userid = NOTLOGGEDIN;
    bufflen = buffp = 0;
    buff = NULL;
}
ClientInfo::ClientInfo(const int _fd, const char* const _ip, const int _port)
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
    {
        delete buff;
        buff = NULL;
    }
}
void ClientInfo::reset()
{
    userid = NOTLOGGEDIN;
    bufflen = buffp = 0;
    if(buff != NULL)
    {
        delete buff;
        buff = NULL;
    }
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
        return READ_CLOSE;
    else if (_rs < 0)
    {
        if(errno == EWOULDBLOCK || errno == EAGAIN)
            return READ_HUNGUP;
        cerr << "sockfd[" << sockfd "] Read error: " << strerror(errno) << endl;
        cerr << "It's belong to userid = " << userid << endl << endl;
        return READ_ERROR;
    }
    return READ_FINISH;
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
        return WRITE_CLOSE;
    else if(_ws < 0)
    {
        if(errno == EWOULDBLOCK || errno == EAGAIN)
            return WRITE_HANGUP;
        cerr << "sockfd[" << sockfd "] Write error: " << strerror(errno) << endl;
        cerr << "It's belong to userid = " << userid << endl << endl;
        return WRITE_ERROR;
    }
    return WRITE_FINISH;
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
    clientlist.clear();
    clicnt = 0;

    userlist.clear();
}
ServerSock::~ServerSock()
{
    // save data into database
    // close all socket
    clientlist.clear();
        
    return;
}

int ServerSock::Server_Start()
{
    cout << "===================================" << endl;
    cout << "=  Welcome to use OneRoom Server  =" << endl;
    cout << "= The server now is under working =" << endl;
    cout << "===================================" << endl;
    cout << endl << endl;

    fd_set sockfds, readfds;

	FD_ZERO(&sockfds);
	FD_SET(listenfd, &sockfds);

    while(TAIHOUDAISUKI)
    {
        readfds = sockfds;
        int retsel = select(FD_SETSIZE, &readfds, NULL, NULL, NULL);
		if (retsel <= 0)
		{
			cerr << "Server Select error: " << strerror(errno) << endl;
			return SERVER_ERROR;
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

                ClientInfo newclient = (clientfd, ipbuff, cli_port);
                clientlist.push_back(newclient);

                flag = fcntl(clientfd, F_GETFL, 0);
                fcntl(clientfd, F_SETFL, flag | O_NONBLOCK);
                FD_SET(clientfd, &sockfds);
                cout << "Server: Accept connection successfully." << endl << endl;
            }
        }

        // client socket
        list<ClientInfo>::iterator it;
        for(it = clientlist.begin(); it != clientlist.end(); )  // be care of it++
        {
            ClientInfo curclient = *it;
            if(!FD_ISSET(curclient.sockfd, &readfds))
                continue;
            int read_res = curclient.Read_Bitstream();
            if(read_res == READ_HUNGUP)
                continue;
            if(read_res == READ_ERROR || read_res == READ_CLOSE)
            {
                log_out_request(curclient);
                clientlist.erase(it++);
                continue;
            }
            if(read_res == READ_FINISH)
            {
                /*
                    if(request == logout)
                    {
                        log_out_request(curclient);
                        clientlist.erase(it++);
                        continue;
                    }
                */
                // *check buffer kind
                // *deal different kind
            }
        }
    }
}
/*
    read / write error ==> disconnect ==> remove ==> flag = 1
    login / logout ==> flag = 0 & flush (if disconnect ==> flag = 1)
    loop final ==> flag = 1 ==> flush ==> flag = 0 (if disconnect ==> flag = 1)
*/


int ServerSock::log_in_request(ClientInfo &client)
{
    char account[MAXACCLEN + 1]; // get account from packet
    char password[MAXPASSLEN + 1]; // get password from packet
    char pass_MD5[MD5LEN + 1]; // get MD5
    int dbaccres; // count(*) where database.account == account
    int dbMD5res; // count(*) where database.account == account and database.pass_MD5 == pass_MD5
    if(!dbaccres)
    {
        // *reply request account doesn't exist
        return 1;
    }
    if(!dbMD5res)
    {
        // *reply request password error
        return 1;
    }

    // identify success
    char username[MAXNAMELEN]; // get username from database
    int userid; // get userid from database

    map<int, int>::iterator it = userlist.serach(userid);
    if(it != map.end()) // kick off
    {
        ClientInfo preclient = it->second;

        // *send message to preclient for logging out

        cout << "[userid = " << userid << " ] is kick off from [ip = " 
            << preclient.ip << ", port = " << preclient.port << endl;
        preclient.reset();
        userlist.erase(it);
    }

    // log in
    // *reply request successfully

    cout << "[userid = " << userid << " ] log in from [ip = " 
        << client.ip << ", port = " << client.port << endl << endl;
    userlist.insert(make_pair(userid, client.sockfd));
    client.set(userid);

    return 1;
}
int ServerSock::log_out_request(ClientInfo &client)
{
    cout << "[userid = " << client.userid << " ] log out from [ip = " 
        << client.ip << ", port = " << client.port << endl << endl;
    userlist.erase(client.userid);
    client.reset();

    return 1;
}
int ServerSock::get_userlist_request(ClientInfo &client)
{
    map<int, int>::iterator it;
    for(it = userlist.begin(); it != userlist.end(); ++it)
    {
        // *select * where databse.userid == it->userid*
        // *write (account, username) into a struct ?
    }
    // *reply userlist struct

    return 1;
}
int ServerSock::get_setting_request(ClientInfo &client)
{
    // read xml file into a struct
    // reply setting struct 

    return 1;
}
int ServerSock::is_change_setting_request(ClientInfo &client)
{
    // receive setting struct
    // save struct into a xml file
    // reply setting save successfully

    return 1;
}
int ServerSock::transmit_request(ClientInfo &client)
{
    int mess_type; // in _messagetpye
    int snd_type; // in _sendtype
    int rcver_cnt = 0;
    char rcver_acc[MAXCONNNUM][MAXACCLEN];
    int rcver_uid[MAXCONNNUM];
    int rcver_fd[MAXCONNNUM];

    switch (snd_type)
    {
        case P_2_P:
        {
            rcver_acc[rcver_cnt]
            ++rcver_cnt;
            break;
        }
        case P_2_G:
        {
            break;
        }
        case P_2_A:
        {
            map<int, int>::iterator it;
            for(it = userlist.begin(); it != userlist.end(); ++it)

            break;
        }
        default:
        {
            rcver_cnt = 0;
            break;
        }
    }


    switch (mess_type)
    {
        case TEXT_TYPE:
        {
            
            break;
        }
        case GRAPH_TYPE:
        {
            break;
        }
        case FILE_TYPE:
        {
            break;
        }
        default:
        {
            break;
        }
    }
}
