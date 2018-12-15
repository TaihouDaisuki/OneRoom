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
        cerr << "It's belong to userid = " << userid << endl;
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
        cerr << "It's belong to userid = " << userid << endl;
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
    userreq = 0;
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
        for(it = clientlist.begin(); it != clientlist.end(); ) 
        {
            ClientInfo curclient = *it;
            if(!FD_ISSET(curclient.sockfd, &readfds))
                continue;
            int read_res = curclient.Read_Bitstream();
            if(read_res == READ_HUNGUP)
            {
                ++it;  // be care of it++
                continue;
            }
            if(read_res == READ_ERROR || read_res == READ_CLOSE)
            {
                log_out_unexpected(curclient);
                clientlist.erase(it++);
                userreq = 1;
                continue;
            }
            if(read_res == READ_FINISH)  // be care of it++
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

        if(!userreq)
            continue;
        // some one disconnect when sending userlist, resend it
        userreq = userlist_request_to_all();
    }
}
/*
    read / write error ==> disconnect ==> remove ==> flag = 1
    login / logout ==> flag = 0 & flush (if disconnect ==> flag = 1)
    loop final ==> flag = 1 ==> flush ==> flag = 0 (if disconnect ==> flag = 1)
*/

int ServerSock::log_in_request(ClientInfo *client) // wating for adding limit max log in number
{
    char account[MAXACCLEN + 1]; // get account from packet
    char password[MAXPASSLEN + 1]; // get password from packet
    char pass_MD5[MD5LEN + 1]; // get MD5
    int dbaccres; // count(*) where database.account == account
    int dbMD5res; // count(*) where database.account == account and database.pass_MD5 == pass_MD5
    int fsttime; // select fsttime where database.account == account
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
    if(fsttime)
    {
        // *reply to change default password
        return 1;
    }

    // identify success
    char username[MAXNAMELEN]; // get username from database
    int userid; // get userid from database

    map<int, ClientInfo*>::iterator it = userlist.serach(userid);
    if(it != map.end()) // kick off
    {
        ClientInfo* preclient = it->second;

        // *send message to preclient for logging out

        cout << "[userid = " << userid << " ] is kick off from [ip = " 
            << preclient->ip << ", port = " << preclient->port << endl;
        preclient->reset();
        userlist.erase(it);
    }

    // log in
    // *reply request successfully

    cout << "[userid = " << userid << " ] log in from [ip = " 
        << client->ip << ", port = " << client->port << endl << endl;
    userlist.insert(make_pair(userid, client));
    client->set(userid);

    userreq = userlist_request_to_all();
    return 1;
}
int ServerSock::log_out_request(ClientInfo *client, list<ClientInfo>::iterator &it)
{
    cout << "[userid = " << client->userid << " ] log out from [ip = " 
        << client->ip << ", port = " << client->port << endl << endl;

    userlist.erase(client->userid);
    clientlist.erase(it++);

    userreq = userlist_request_to_all();
    return 1;
}
int ServerSock::change_password_request(ClientInfo *client)
{
    char account[MAXACCLEN + 1]; // get account from packet
    char oldpassword[MAXPASSLEN + 1]; // get password from packet
    char oldpass_MD5[MD5LEN + 1]; // get MD5
    char newpassword[MAXPASSLEN + 1]; // get password from packet
    char newpass_MD5[MD5LEN + 1]; // get MD5
    int dbaccres; // count(*) where database.account == account
    int dbMD5res; // count(*) where database.account == account and database.pass_MD5 == pass_MD5

    if(!dbaccres)
    {
        // *reply request account doesn't exist
        return 1;
    }
    if(!dbMD5res)
    {
        // *reply request old password error
        return 1;
    }

    // *change password in database and set first_time_log_in to 0
    // *reply password change successfully
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
    ClientInfo* rcver_client[MAXCONNNUM];
    int snd_succ[MAXCONNNUM];
    map<int, int>::iterator it;

    switch (snd_type)
    {
        case P_2_P:
        {
            // *get rcver_acc[rcver_cnt] from data pack
            // rcver_uid[rcver_cnt] = select uid from database where database.account == rcver_acc[rcver_cnt]
            it = userlist.find(rcver_uid[rcver_cnt]);
            if(it == userlist.end())
                snd_succ[rcver_cnt] = 0;
            else
            {
                snd_succ[rcver_cnt] = 1;
                rcver_client[rcver_cnt] = it->second();
            }
            ++rcver_cnt;
            break;
        }
        case P_2_G:
        {
            // * for ----> get rcver_acc[rcver_cnt] from data pack
            // * other like P_2_P, a loop end with ++rcver_cnt for each loop
            break;
        }
        case P_2_A:
        {
            map<int, int>::iterator it;
            for(it = userlist.begin(); it != userlist.end(); ++it, ++rcver_cnt)
            {
                snd_succ[rcver_cnt] = 1;
                rcver_uid[rcver_cnt] = it->first;
                rcver_client[rcver_cnt] = it->second;
                // rcver_acc[rcver_cnt] = select account from database where database.uid == rcver_uid[rcver_cnt]
            }

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
            // *write text into common_buffer
            break;
        }
        case GRAPH_TYPE:
        {
            // *save graph into file
            // *write graph into common_buffer (if file is large maybe need to send serval times)
            break;
        }
        case FILE_TYPE:
        {
            // *save file
            // *write file into common_buffer (if file is large maybe need to send serval times)
            break;
        }
        default:
        {
            break;
        }
    }

    for(int i = 0; i < rcver_cnt; ++i)
    {
        if(!snd_succ[i]) // socket has close / user has log out
            continue;

        ClientInfo* client = rcver_client[i];
        int _ws = client->Write_Bitstream();
        while(_ws == WRITE_HUNGUP)
        {
            sleep(1); // avoid occupy CPU
            _ws = client->Write_Bitstream();
        }
        if(_ws == WRITE_CLOSE || WRITE_ERROR)
        {
            res = 1;
            log_out_unexpected(client);
            list<ClientInfo>::iterator it = find(clientlist.begin(), clientlist.end(), *client);
            if(it != clientlist.end())
                list.erase(it);
            continue;
        }
        else if(_ws == WRITE_FINISH)
            continue;
    }

    // *write user_acc[i] and snd_succ[i] a pair into file
    // *write file into buffer
    // *send file back to client
}
int ServerSock::userlist_request_to_all()
{
    map<int, ClientInfo *>::iterator mapit;
    for(mapit = userlist.begin(); mapit != userlist.end(); ++it)
        ; // *write userid in userlist into file (e.g. xml)
    
    int res = 0;
    
    list<ClientInfo>::iterator listit;
    for(listit = clientlist.begin(); listit != clientlist.end(); )
    {
        ClientInfo client = *listit;
        // *write userlist(xml file) into buffer
        int _ws = client.Write_Bitstream();
        while(_ws == WRITE_HUNGUP)
        {
            sleep(1); // avoid occupy CPU
            _ws = client.Write_Bitstream();
        }
        if(_ws == WRITE_CLOSE || WRITE_ERROR)
        {
            res = 1;
            log_out_unexpected(&curclient);
            clientlist.erase(listit++);
            continue;
        }
        else if(_ws == WRITE_FINISH)
            continue;
        
        //WRITE_DEFAULT should not be run if it run normally 
        cerr << "WRITE_DEFAULT has been run, some bug may exist" << endl << endl << endl;
    }

    return res;
}

inline int Server::log_out_unexpected(ClientInfo *client)
{
    cout << "[userid = " << client->userid << " ] log out from [ip = " 
        << client->ip << ", port = " << client->port << " unexpected." << endl << endl;

    userlist.erase(client->userid);
    return 1;
}
