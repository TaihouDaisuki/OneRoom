#include "server.h"

using namespace std;

const char _ip[] = "0.0.0.0";
const int _port = 4347;

//==========ClientInfo==========
ClientInfo::ClientInfo()
{
    sockfd = ERRSOCKET;
    userid = NOTLOGGEDIN;
    bufflen[0] = buffp[0] = 0;
    bufflen[1] = buffp[1] = 0;
}
ClientInfo::ClientInfo(const int _fd, const char* const _ip, const int _port)
{
    strcpy(ip, _ip);
    port = _port;

    sockfd = _fd;
    userid = NOTLOGGEDIN;
    bufflen[0] = buffp[0] = 0;
    bufflen[1] = buffp[1] = 0;
}
ClientInfo::~ClientInfo()
{
    close(sockfd);
}

void ClientInfo::set(const int _userid)
{
    userid = _userid;
    bufflen[0] = buffp[0] = 0;
    bufflen[1] = buffp[1] = 0;
}
void ClientInfo::reset()
{
    userid = NOTLOGGEDIN;
    bufflen[0] = buffp[0] = 0;
    bufflen[1] = buffp[1] = 0;
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
        _rs = read(sockfd, buff[0] + bufferp[0], bufflen[0] - buffp[0]);
        buffp[0] += (_rs > 0 ? _rs : 0);
        if(buffp[0] == bufflen[0])
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
        _ws = write(sockfd, buffer[1] + buffer_p[1], bufflen[1] - buffer_p[1]);
        buffp[1] += (_ws > 0 ? _ws : 0);
        if(buffp[1] == bufflen[1])
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

void ClientInfo::Load_Buffer(void * const dst, const int len)
{
    memcpy(dst, buff[0], len);

    return 1;
}
void ClientInfo::reset_read_buff(const int len)
{
    bufflen[0] = len;
    buffp[0] = 0;
}
void ClientInfo::Save_Buffer(void * const src, const int len)
{
    memcpy(buff[1], src, len);
    buffp[1] = 0;
    bufflen[1] = len;

    return 1;
}

int ClientInfo::Rcv(void *const dst, const int len)
{
    reset_read_buff(len);
    int _rs = Read_Bitstream();
    while (_rs == WRITE_HUNGUP)
    {
        sleep(1); // avoid occupy CPU
        _rs = Write_Bitstream();
    }
    if(_rs == READ_FINISH)
        Load_Buffer(dst, len);
    return _rs;
}
int ClientInfo::Snd(void *const src, const int len)
{
    Save_Buffer(src, len);
    int _ws = Write_Bitstream();
    while (_ws == WRITE_HUNGUP)
    {
        sleep(1); // avoid occupy CPU
        _ws = Write_Bitstream();
    }
    return _ws;
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

                ClientInfo newclient(clientfd, ipbuff, cli_port);
                newclient.reset_read_buff(CTRLPACKLEN); // set read buff len
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
            if(curclient.sockfd == ERRSOCKET || !FD_ISSET(curclient.sockfd, &readfds))
            {
                ++it;
                continue;
            }
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
                continue;
            }
            if(read_res == READ_FINISH)  // be care of it++
            {
                Packet packet;
                curclient.Load_Buffer(&packet);
                if(!curclient.is_Logged_in() && (packet.isData || packet.Type != LOG_IN_REQ))
                {
                    // *reply request error(has not log in)
                    curclient.reset_read_buff(CTRLPACKLEN);
                    ++it;
                    continue;
                }

                int res = 0;
                switch(packet.isData)
                {
                    case 0:
                    {
                        switch(packet.Type)
                        {
                            case LOG_IN_REQ:
                            {
                                res = log_in_request(curclient);
                                break;
                            }
                            case LOG_OUT_REQ:
                            {
                                res = log_out_request(curclient);
                                break;
                            }
                            case CHG_PSSW_REQ:
                            {
                                res = change_password_request(curclient);
                                break;
                            }
                            case CHG_SET_REQ:
                            {
                                res = change_setting_request(curclient);
                                break;
                            }
                        }
                        break;
                    }
                    case 1:
                    {
                        res = transmit_request(curclient, &packet);
                        break;
                    }
                    default:
                    {
                        // *reply request error
                        break;
                    }
                }
                if(res == -1) // socket ERROR
                {
                    clientlist.erase(it++);
                    continue;
                }
                if(res == 0) // command error
                    //send command error to client
                    ;
                curclient.reset_read_buff(CTRLPACKLEN); // next round reading
                ++it;
            }
        }

        if(!userreq)
            continue;
        // some one disconnect when sending userlist, resend it
        userreq = 0;
        userlist_request_to_all();
        for(it = clientlist.begin(); it != clientlist.end(); )
        {
            if(it->sockfd == ERRSOCKET)
                clientlist.erase(it++);
            else
                ++it;
        }
    }
}
/*
    read / write error ==> disconnect ==> remove ==> flag = 1
    login / logout ==> flag = 0 & flush (if disconnect ==> flag = 1)
    loop final ==> flag = 1 ==> flush ==> flag = 0 (if disconnect ==> flag = 1)
*/

int ServerSock::log_in_request(ClientInfo *client, const char* const data) // wating for adding limit max log in number
{
    char account[MAXACCLEN + 1]; // get account from packet
    char password[MAXPASSLEN + 1]; // get password from packet
    char pass_MD5[MD5LEN + 1]; // get MD5
    int dbaccres; // count(*) where database.account == account
    int dbMD5res; // count(*) where database.account == account and database.pass_MD5 == pass_MD5
    int fsttime; // select fsttime where database.account == account

    int _rs = client->Rcv(account, MAXACCLEN);
    if (_rs == READ_CLOSE || READ_ERROR)
    {
        log_out_unexpected(client);
        return -1;
    }
    _rs = client->Rcv(password, MAXPASSLEN);
    if (_rs == READ_CLOSE || READ_ERROR)
    {
        log_out_unexpected(client);
        return -1;
    }

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
    // *read ini file into a struct
    // *reply setting struct 
    // *reply request successfully

    cout << "[userid = " << userid << " ] log in from [ip = " 
        << client->ip << ", port = " << client->port << endl << endl;
    userlist.insert(make_pair(userid, client));
    client->set(userid);

    userlist_request_to_all();
    return 1;
}
int ServerSock::log_out_request(ClientInfo *client)
{
    cout << "[userid = " << client->userid << " ] log out from [ip = " 
        << client->ip << ", port = " << client->port << endl << endl;

    userlist.erase(client->userid);

    userlist_request_to_all();
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

    int _rs;
    _rs = client->Rcv(account, MAXACCLEN);
    if (_rs == READ_CLOSE || READ_ERROR)
    {
        log_out_unexpected(client);
        return -1;
    }
    _rs = client->Rcv(oldpassword, MAXPASSLEN);
    if (_rs == READ_CLOSE || READ_ERROR)
    {
        log_out_unexpected(client);
        return -1;
    }
    _rs = client->Rcv(newpassword, MAXPASSLEN);
    if (_rs == READ_CLOSE || READ_ERROR)
    {
        log_out_unexpected(client);
        return -1;
    }

    if(!strcmp(oldpassword, newpassword)) // 
    {
        // *reply password hasn't changed
        return 1;
    }
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
int ServerSock::change_setting_request(ClientInfo &client)
{
    // receive setting struct
    // save struct into a ini file
    // reply setting save successfully

    return 1;
}
int ServerSock::transmit_request(ClientInfo &client, CtrlPack *pack)
{
    int snd_type = pack->type & 0xF;
    int mess_type = (pack->type >> 4) & 0xF;
    int rcver_cnt = 0;
    char rcver_acc[MAXCONNNUM][MAXACCLEN];
    int rcver_uid[MAXCONNNUM];
    ClientInfo* rcver_client[MAXCONNNUM];
    int snd_succ[MAXCONNNUM];
    map<int, ClientInfo*>::iterator it;

    char *buffer = new(nothrow) char[MAXDATALEN * (pack->isCut + 1)];
    if(buffer == NULL)
    {
        cerr << "Server Run Out of Memory" << endl;
        exit(EXIT_FAILURE);
    }
    int bufferlen = 0;
    while(TAIHOUDAISUKI)
    {
        _rs = client->Rcv(buffer + bufferlen, pack->Datalen);
        if (_rs == READ_CLOSE || READ_ERROR)
        {
            log_out_unexpected(client);
            delete[] buffer;
            return -1;
        }

        bufferp += ntohl(pack->Datalen);  // net to host !!!
        if(pack->Seq == pack->isCut)
            break;
            
        _rs = client->Rcv(packet, CTRLPACKLEN);
        if (_rs == READ_CLOSE || READ_ERROR)
        {
            log_out_unexpected(client);
            delete[] buffer;
            return -1;
        }
    }

    int bufferp = 0;
    switch (snd_type)
    {
        case P_2_P:
        {
            memcpy(rcver_acc[rcver_cnt], buffer, MAXACCLEN);
            bufferp += MAXACCLEN;
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
            unsigned char rcver_cnt = buffer[bufferp++];
            for(int i = 0; i < rcver_cnt; ++i)
            {
                memcpy(rcver_acc[i], buffer + bufferp, MAXACCLEN);
                bufferp += MAXACCLEN;
                // rcver_uid[i] = select uid from database where database.account == rcver_acc[i]
                it = userlist.find(rcver_uid[i]);
                if (it == userlist.end())
                    snd_succ[i] = 0;
                else
                {
                    snd_succ[i] = 1;
                    rcver_client[i] = it->second();
                }
            }
            break;
        }
        case P_2_A:
        {
            for(it = userlist.begin(); it != userlist.end(); ++it)
            {
                if(it->first == client.userid) // don't send to the sender
                    continue;
                snd_succ[rcver_cnt] = 1;
                rcver_uid[rcver_cnt] = it->first;
                rcver_client[rcver_cnt] = it->second;
                ++rcver_cnt
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

    // file buffer from buffer+bufferp to buffer+bufferlen-1, length = bufferlen-bufferp
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

    unsigned char _isData = 1;
    unsigned char _Type = (mess_type << 4) | snd_type;
    unsigned char _isCut = (bufferlen - bufferp) / MAXBUFFERLEN - 1 + !((bufferlen - bufferp) % MAXBUFFERLEN);
    // CTRLPACKLEN
    for(int _Sep = 0; _Seq <= _isCut; ++_Seq)
    {
        int curlen = (_Seq == _isCut) ? (bufferlen - bufferp) : MAXDATALEN;
        int _Datalen = hton(curlen);
        CtrlPack _Packet(_isData, _Type, _isCut, _Seq, _Datalen);
        for (int i = 0; i < rcver_cnt; ++i)
        {
            if (!snd_succ[i]) // socket has close / user has log out
                continue;

            ClientInfo *receiver = rcver_client[i];
            if (receiver->sockfd == ERRSOCKET) // socket has error above
                continue;                   // but we check userlist first, so this might not work, for secure, take it

            
            int _ws = receiver->Snd(_Packet, CTRLPACKLEN);
            if (_ws == WRITE_CLOSE || WRITE_ERROR)
            {
                log_out_unexpected(receiver);
                snd_succ[i] = 0;
                continue;
            }
            int _ws = receiver->Snd(buffer + bufferp, curlen);
            if (_ws == WRITE_CLOSE || WRITE_ERROR)
            {
                log_out_unexpected(receiver);
                snd_succ[i] = 0;
                continue;
            }
            else if (_ws == WRITE_FINISH)
                continue;
        }
        bufferp += curlen;
    }

    // *write user_acc[i] and snd_succ[i] a pair into file
    // *write file into buffer
    // *send file back to client
    delete[] buffer;
    return 1;
}
int ServerSock::userlist_request_to_all()
{
    map<int, ClientInfo *>::iterator mapit;
    for(mapit = userlist.begin(); mapit != userlist.end(); ++it)
        ; // *write userid in userlist into file (e.g. xml)
    
    list<ClientInfo>::iterator listit;
    for(listit = clientlist.begin(); listit != clientlist.end(); ++listit)
    {
        ClientInfo client = *listit;
        if(client.sockfd == ERRSOCKET)
            continue;
        // *write userlist(xml file) into buffer

        /*int _ws = client.Snd(buffer, len);
        if(_ws == WRITE_CLOSE || WRITE_ERROR)
        {
            log_out_unexpected(&curclient);
            continue;
        }
        else if(_ws == WRITE_FINISH)
            continue;
        
        //WRITE_DEFAULT should not be run if it run normally 
        cerr << "WRITE_DEFAULT has been run, some bug may exist" << endl << endl << endl; */
    }
    return 1;
}

inline int Server::log_out_unexpected(ClientInfo *client)
{
    cout << "[userid = " << client->userid << " ] log out from [ip = " 
        << client->ip << ", port = " << client->port << " unexpected." << endl << endl;

    userlist.erase(client->userid);
    close(client->sockfd);
    client->sockfd = ERRSOCKET;

    userreq = 1;
    return 1;
}
