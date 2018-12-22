#include "server.h"
#include "tinystr.h"
#include "tinyxml.h"

using namespace std;

const char _ip[] = "0.0.0.0";
const int _port = 20610;

//==========ClientInfo==========
ClientInfo::ClientInfo()
{
    sockfd = ERRSOCKET;
    userid = NOTLOGGEDIN;
    bufflen[0] = buffp[0] = 0;
    bufflen[1] = buffp[1] = 0;
}
ClientInfo::ClientInfo(const int _fd, const char *const _ip, const int _port)
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
    if(sockfd != ERRSOCKET)
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
        _rs = read(sockfd, buff[0] + buffp[0], bufflen[0] - buffp[0]);
        buffp[0] += (_rs > 0 ? _rs : 0);
        if (buffp[0] == bufflen[0])
            break;
    } while (_rs < 0 && errno == EINTR);

    if (_rs == 0)
        return READ_CLOSE;
    else if (_rs < 0)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return READ_HUNGUP;
        cerr << "sockfd[" << sockfd << "] Read error: " << strerror(errno) << endl;
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
        _ws = write(sockfd, buff[1] + buffp[1], bufflen[1] - buffp[1]);
        buffp[1] += (_ws > 0 ? _ws : 0);
        if (buffp[1] == bufflen[1])
            break;
    } while (_ws < 0 && errno == EINTR);

    if (_ws == 0)
        return WRITE_CLOSE;
    else if (_ws < 0)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return WRITE_HUNGUP;
        cerr << "sockfd[" << sockfd << "] Write error: " << strerror(errno) << endl;
        cerr << "It's belong to userid = " << userid << endl;
        return WRITE_ERROR;
    }
    return WRITE_FINISH;
}

void ClientInfo::Load_Buffer(void *const dst, const int len)
{
    memcpy(dst, buff[0], len);
}
void ClientInfo::reset_read_buff(const int len)
{
    bufflen[0] = len;
    buffp[0] = 0;
}
void ClientInfo::Save_Buffer(void *const src, const int len)
{
    memcpy(buff[1], src, len);
    buffp[1] = 0;
    bufflen[1] = len;
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
    if (_rs == READ_FINISH)
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
    cout<<len<<endl;
    return _ws;
}

//==========ServerSock==========
ServerSock::ServerSock()
{
    //==========sock==========
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        cerr << "Create socket error: " << strerror(errno) << "(errno: " << errno << ")" << endl;
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
    if (bind(listenfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        cerr << "bind socket error: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    //==========listen==========
    if (listen(listenfd, MAXCONNNUM) < 0)
    {
        cerr << "listen socket error: " << strerror(errno) << endl;
        exit(EXIT_FAILURE);
    }

    //==========reset==========
    clientlist.clear();
    clicnt = 0;

    userlist.clear();
    userreq = 0;

    //==========database==========
    if ((mysql = mysql_init(NULL)) == NULL)
    {
        cerr << "mysql_init failed" << endl;
        exit(EXIT_FAILURE);
    }

    if (mysql_real_connect(mysql, "localhost", "u1650254", "u1650254", "db1650254", 0, NULL, 0) == NULL)
    {
        cerr << "mysql_real_connect failed(" << mysql_error(mysql) << ")" << endl;
        exit(EXIT_FAILURE);
    }
    mysql_set_character_set(mysql, "gbk");
}
ServerSock::~ServerSock()
{
    // save data into database
    // close all socket
    clientlist.clear();
    mysql_close(mysql);

    return;
}

int ServerSock::updatebfds(fd_set fds)
{
    int i;
    int res_maxfd = 0;
    for (i = 0; i < FD_SETSIZE; ++i)
        if (FD_ISSET(i, &fds) && i > res_maxfd)
            res_maxfd = i;
    return res_maxfd;
}

int ServerSock::Server_Start()
{
    cout << "===================================" << endl;
    cout << "=  Welcome to use OneRoom Server  =" << endl;
    cout << "= The server now is under working =" << endl;
    cout << "===================================" << endl;
    cout << endl
         << endl;

    FD_ZERO(&sockfds);
    FD_ZERO(&readfds);

    FD_SET(listenfd, &sockfds);


    while (TAIHOUDAISUKI)
    {
        struct timeval timeout;
        timeout.tv_sec = 1;
        timeout.tv_usec = 0;
        readfds = sockfds;
        int count = updatebfds(readfds);
       // cout << count << endl;
        errno = 0;
        int retsel = select(count+1, &readfds, NULL, NULL, &timeout);
        if (retsel < 0)
        {
            if (errno == EINTR || errno == EWOULDBLOCK || errno == EAGAIN)
                continue;
            else
            {
                cout << retsel << " " << errno << endl;
                cerr << "Server Select error: " << strerror(errno) << endl;
                return SERVER_ERROR;
            }
        }
        else if (retsel == 0)
        {
            continue;
        }

        // listen socket
        if (FD_ISSET(listenfd, &readfds))
        {
            int clientfd = 0, flag;
            struct sockaddr_in cliaddr;
            socklen_t clilen = sizeof(cliaddr);
            while (clientfd >= 0)
            {
                if ((clientfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen)) < 0)
                    break;

                char ipbuff[INET_ADDRSTRLEN + 1] = {0};
                inet_ntop(AF_INET, &cliaddr.sin_addr, ipbuff, INET_ADDRSTRLEN);
                uint16_t cli_port = ntohs(cliaddr.sin_port);
                cout << "Server: Detect a connection from " << ipbuff << ", port " << cli_port << ", using fd " << clientfd << endl;

                ClientInfo newclient(clientfd, ipbuff, cli_port);
                newclient.reset_read_buff(CTRLPACKLEN); // set read buff len
                clientlist.push_back(newclient);
                newclient.sockfd = ERRSOCKET;

                flag = fcntl(clientfd, F_GETFL, 0);
                fcntl(clientfd, F_SETFL, flag | O_NONBLOCK);
                FD_SET(clientfd, &sockfds);
            }
            cout << "Server: Accept connection successfully." << endl
                 << endl;
        }

        // client socket
        list<ClientInfo>::iterator it;
        for (it = clientlist.begin(); it != clientlist.end();)
        {
            ClientInfo &curclient = *it;
            if (curclient.sockfd == ERRSOCKET || !FD_ISSET(curclient.sockfd, &readfds))
            {
                ++it;
                cout << "test" << endl;
                continue;
            }
            int read_res = curclient.Read_Bitstream();
            cout<<"read"<<endl;
            if (read_res == READ_HUNGUP)
            {
                ++it; // be care of it++
                continue;
            }
            if (read_res == READ_ERROR || read_res == READ_CLOSE)
            {
                log_out_unexpected(&curclient);
                clientlist.erase(it++);
                continue;
            }
            if (read_res == READ_FINISH) // be care of it++
            {
                CtrlPack packet;
                curclient.Load_Buffer(&packet, CTRLPACKLEN);

                int res = 0;
                switch (packet.isData)
                {
                case 0:
                {
                    switch (packet.Type)
                    {
                    case LOG_IN_REQ:
                    {
                        res = log_in_request(&curclient);
                        break;
                    }
                    case LOG_OUT_REQ:
                    {
                        res = log_out_request(&curclient);
                        break;
                    }
                    case CHG_PSSW_REQ:
                    {
                        res = change_password_request(&curclient);
                        break;
                    }
                    case CHG_SET_REQ:
                    {
                        res = change_setting_request(&curclient);
                        break;
                    }
                    case GET_HISTORY:
                    {
                        
                    }
                    }
                    break;
                }
                case 1:
                {
                    res = transmit_request(&curclient, &packet);
                    break;
                }
                default:
                {
                    // *reply request error
                    break;
                }
                }
                if (res == -1) // socket ERROR
                {
                    clientlist.erase(it++);
                    continue;
                }
                curclient.reset_read_buff(CTRLPACKLEN); // next round reading
                ++it;
            }
        }

        if (!userreq)
            continue;
        cout<<"gg"<<endl;
        // some one disconnect when sending userlist, resend it
        userreq = 0;
        for (it = clientlist.begin(); it != clientlist.end();)
        {
            if (it->sockfd == ERRSOCKET)
                clientlist.erase(it++);
            else
                ++it;
        }
        userlist_request_to_all();
    }

    return 1;
}
/*
    read / write error ==> disconnect ==> remove ==> flag = 1
    login / logout ==> flag = 0 & flush (if disconnect ==> flag = 1)
    loop final ==> flag = 1 ==> flush ==> flag = 0 (if disconnect ==> flag = 1)
*/

int ServerSock::log_in_request(ClientInfo *client) // wating for adding limit max log in number
{
    char account[MAXACCLEN + 1];   // get account from packet
    char password[MAXPASSLEN + 1]; // get password from packet
    int userid;
    int dbpassres; // count(*) where database.account == account and database.pass_MD5 == pass_MD5
    bool fsttime;  // select fsttime where database.account == account
    CtrlPack _Packet = {0, REQ_ERR_DISC, 0, 0, htonl(sizeof(char))};

    int _rs = client->Rcv(account, MAXACCLEN);
    if (_rs == READ_CLOSE || _rs == READ_ERROR)
    {
        log_out_unexpected(client);
        return -1;
    }
    _rs = client->Rcv(password, MAXPASSLEN);
    if (_rs == READ_CLOSE || _rs == READ_ERROR)
    {
        log_out_unexpected(client);
        return -1;
    }

    cout<<account<<endl<<password<<endl;

    char buffer[MAXBUFFERLEN];
    // get account
    userid = mysql_get_uid(account);
    cout<<"userid="<<userid<<endl;
    if (userid == -1)
    {
        // reply request account doesn't exist
        char error_number = ACC_NOT_EXIST;
        memcpy(buffer, &_Packet, CTRLPACKLEN);
        memcpy(buffer + CTRLPACKLEN, &error_number, sizeof(char));

        client->Snd(buffer, CTRLPACKLEN + sizeof(char));
        log_out_unexpected(client);
        return -1;
    }

    // search password
    dbpassres = mysql_compare_password(userid, password);
    cout<<dbpassres<<endl;
    if (!dbpassres)
    {
        // reply request password error
        char error_number = PASS_ERR;
        memcpy(buffer, &_Packet, CTRLPACKLEN);
        memcpy(buffer + CTRLPACKLEN, &error_number, sizeof(char));

        client->Snd(buffer, CTRLPACKLEN + sizeof(char));
        log_out_unexpected(client);
        return -1;
    }

    // check first time
    client->set(userid);
    fsttime = mysql_check_first_time(userid);
    cout<<fsttime<<endl;
    if (fsttime)
    {
        // reply to change default password
        char error_number = CHG_PASS;
        _Packet.Type = REQ_ERR_CONN;
        memcpy(buffer, &_Packet, CTRLPACKLEN);
        memcpy(buffer + CTRLPACKLEN, &error_number, sizeof(char));

        int _ws = client->Snd(buffer, CTRLPACKLEN + sizeof(char));
        if (_ws == WRITE_CLOSE || _ws == WRITE_ERROR)
        {
            log_out_unexpected(client);
            return -1;
        }

        return 1;
    }

    // identify success
    char username[MAXNAMELEN]; // get username from database
    mysql_get_username(userid, username);

    map<int, ClientInfo *>::iterator it = userlist.find(userid);
    if (it != userlist.end()) // kick off
    {
        ClientInfo *preclient = it->second;

        // send message to preclient for logging out
        char error_number = KICK_OFF;
        memcpy(buffer, &_Packet, CTRLPACKLEN);
        memcpy(buffer + CTRLPACKLEN, &error_number, sizeof(char));

        int _ws = preclient->Snd(buffer, CTRLPACKLEN);
        if (_ws == WRITE_CLOSE || _ws == WRITE_ERROR)
            log_out_unexpected(preclient);

        cout << "[userid = " << userid << " ] is kick off from [ip = "
             << preclient->ip << ", port = " << preclient->port << endl;
        preclient->reset();
        userlist.erase(it);
    }

    // log in
    // *read ini file into a struct
    // *reply setting struct
    // *reply request successfully
    _Packet.Type = REQ_SET;
    _Packet.Datalen = 0; // ************************************************** the buffer length of ini file, here just testing
    int _ws = client->Snd(&_Packet, CTRLPACKLEN);
    if (_ws == WRITE_CLOSE || _ws == WRITE_ERROR)
        log_out_unexpected(client);

    cout << "[userid = " << userid << " ] log in from [ip = "
         << client->ip << ", port = " << client->port << endl
         << endl;
    userlist.insert(make_pair(userid, client));

    userlist_request_to_all();
    return 1;
}
int ServerSock::log_out_request(ClientInfo *client)
{
    cout << "[userid = " << client->userid << " ] log out from [ip = "
         << client->ip << ", port = " << client->port << endl
         << endl;

    FD_CLR(client->sockfd, &sockfds);
    close(client->sockfd);
    userlist.erase(client->userid);

    userlist_request_to_all();
    return 1;
}
int ServerSock::change_password_request(ClientInfo *client)
{
    char oldpassword[MAXPASSLEN + 1]; // get password from packet
    char newpassword[MAXPASSLEN + 1]; // get password from packet
    int dbpasswordres;                // count(*) where database.account == account and database.pass_MD5 == pass_MD5
    char uid_c[10];
    CtrlPack _Packet = {0, REQ_ERR_CONN, 0, 0, htonl(sizeof(char))};

    int _rs;
    _rs = client->Rcv(oldpassword, MAXPASSLEN);
    if (_rs == READ_CLOSE || _rs == READ_ERROR)
    {
        log_out_unexpected(client);
        return -1;
    }
    _rs = client->Rcv(newpassword, MAXPASSLEN);
    if (_rs == READ_CLOSE || _rs == READ_ERROR)
    {
        log_out_unexpected(client);
        return -1;
    }
    cout<<oldpassword<<endl<<newpassword<<endl;
    sprintf(uid_c, "%d", client->userid);
    // search password
    char buffer[MAXBUFFERLEN];
    dbpasswordres = mysql_compare_password(client->userid, oldpassword);
    cout<<dbpasswordres<<endl;
    if (!dbpasswordres)
    {
        // reply request password error
        char error_number = PASS_ERR;
        memcpy(buffer, &_Packet, CTRLPACKLEN);
        memcpy(buffer + CTRLPACKLEN, &error_number, sizeof(char));

        int _ws = client->Snd(buffer, CTRLPACKLEN + sizeof(char));
        if (_ws == WRITE_CLOSE || _ws == WRITE_ERROR)
        {
            log_out_unexpected(client);
            return -1;
        }

        return -1;
    }

    // change password in database and set first_time_log_in to 0
    // reply password change successfully
    string sqlqry;
    sqlqry = "update security set password_MD5 = md5(\'";
    sqlqry.append(newpassword).append("\') where uid = ").append(uid_c).append(";");
    mysql_query(mysql, sqlqry.c_str());
    sqlqry = "update user set First_Time_Log_In = 0";
    sqlqry.append(" where uid = ").append(uid_c).append(";");
    mysql_query(mysql, sqlqry.c_str());

    _Packet.Type = PASS_CHG_SUCC;
    _Packet.Datalen = 0;
    client->Snd(&_Packet, CTRLPACKLEN);
    log_out_unexpected(client);
    return -1;
}
int ServerSock::change_setting_request(ClientInfo *client)
{
    // receive setting struct
    // save struct into a ini file
    // reply setting save successfully

    return 1;
}
int ServerSock::transmit_request(ClientInfo *client, CtrlPack *pack)
{
    int snd_type = pack->Type & 0xF;
    int mess_type = (pack->Type >> 4) & 0xF;
    int rcver_cnt = 0;
    char rcver_acc[MAXCONNNUM][MAXACCLEN];
    int rcver_uid[MAXCONNNUM];
    ClientInfo *rcver_client[MAXCONNNUM];
    int snd_succ[MAXCONNNUM];
    map<int, ClientInfo *>::iterator it;
    
    char *databuffer = new (nothrow) char[MAXDATALEN * (pack->isCut + 1)];
    if (databuffer == NULL)
    {
        cerr << "Server Run Out of Memory" << endl;
        exit(EXIT_FAILURE);
    }
    int bufferlen = 0;
    while (TAIHOUDAISUKI)
    {
        pack->Datalen = ntohl(pack->Datalen);
        int _rs = client->Rcv(databuffer + bufferlen, pack->Datalen);
        if (_rs == READ_CLOSE || _rs == READ_ERROR)
        {
            log_out_unexpected(client);
            delete[] databuffer;
            return -1;
        }

        bufferlen += pack->Datalen; // net to host !!!
        if (pack->Seq == pack->isCut)
            break;

        _rs = client->Rcv(pack, CTRLPACKLEN);
        if (_rs == READ_CLOSE || _rs == READ_ERROR)
        {
            log_out_unexpected(client);
            delete[] databuffer;
            return -1;
        }
    }

    int bufferp = 0;
    switch (snd_type)
    {
    case P_2_P:
    {
        memcpy(rcver_acc[rcver_cnt], databuffer, MAXACCLEN);
        rcver_uid[rcver_cnt] = mysql_get_uid(rcver_acc[rcver_cnt]);
        bufferp += MAXACCLEN;
        it = userlist.find(rcver_uid[rcver_cnt]);
        if (it == userlist.end())
            snd_succ[rcver_cnt] = 0;
        else
        {
            snd_succ[rcver_cnt] = 1;
            rcver_client[rcver_cnt] = it->second;
        }
        ++rcver_cnt;
        break;
    }
    case P_2_G:
    {
        rcver_cnt = int(databuffer[bufferp++]);
        for (int i = 0; i < rcver_cnt; ++i)
        {
            memcpy(rcver_acc[i], databuffer + bufferp, MAXACCLEN);
            rcver_uid[i] = mysql_get_uid(rcver_acc[i]);
            bufferp += MAXACCLEN;
            it = userlist.find(rcver_uid[i]);
            if (it == userlist.end())
                snd_succ[i] = 0;
            else
            {
                snd_succ[i] = 1;
                rcver_client[i] = it->second;
            }
        }
        cout << databuffer + bufferp << endl;
        break;
    }
    case P_2_A:
    {
        bufferp += MAXACCLEN;
        for (it = userlist.begin(); it != userlist.end(); ++it)
        {
            if (it->first == client->userid) // don't send to the sender
                continue;
            snd_succ[rcver_cnt] = 1;
            rcver_uid[rcver_cnt] = it->first;
            rcver_client[rcver_cnt] = it->second;
            mysql_get_username(rcver_uid[rcver_cnt], rcver_acc[rcver_cnt]);
            cout<<rcver_acc[rcver_cnt]<<endl;
            ++rcver_cnt;
        }

        break;
    }
    default:
    {
        rcver_cnt = 0;
        break;
    }
    }

    char sender[MAXACCLEN + 1];
    mysql_get_account(client->userid, sender);

    cout<<"begin to send from "<<sender<<endl;

    char* messagep = databuffer + bufferp;
    bufferp -= MAXACCLEN;
    memcpy(databuffer + bufferp, sender, MAXACCLEN);

    unsigned char _isData = 1;
    unsigned char _Type = (mess_type << 4) | snd_type;

    unsigned char _isCut = (bufferlen - bufferp) / MAXDATALEN - 1 + !!((bufferlen - bufferp) % MAXDATALEN);

    char buffer[MAXBUFFERLEN];
    for (unsigned char _Seq = 0; _Seq <= _isCut; ++_Seq)
    {
        unsigned int curlen = (_Seq == _isCut) ? (bufferlen - bufferp) : MAXDATALEN;
        unsigned int _Datalen = htonl(curlen);
        CtrlPack _Packet = {_isData, _Type, _isCut, _Seq, _Datalen};
        memcpy(buffer, &_Packet, CTRLPACKLEN);
        memcpy(buffer + CTRLPACKLEN, databuffer + bufferp, curlen);

        for (int i = 0; i < rcver_cnt; ++i)
        {
            if (!snd_succ[i]) // socket has close / user has log out
                continue;

            ClientInfo *receiver = rcver_client[i];
            if (receiver->sockfd == ERRSOCKET) // socket has error above
                continue;                      // but we check userlist first, so this might not work, for secure, take it

            int _ws = receiver->Snd(buffer, CTRLPACKLEN + curlen);
            if (_ws == WRITE_CLOSE || _ws == WRITE_ERROR)
            {
                log_out_unexpected(receiver);
                snd_succ[i] = 0;
                continue;
            }
        }
        bufferp += curlen;
    }
    cout<<"end to send"<<endl;
    delete[] databuffer;

    int succ_cnt = 0;
    for (int i = 0; i < rcver_cnt; ++i)
        if (snd_succ[i])
            ++succ_cnt;

    bufferlen = MAXACCLEN * succ_cnt;
    bufferp = 0;
    databuffer = new (nothrow) char[bufferlen];
    if (databuffer == NULL)
    {
        cerr << "Server Run Out of Memory" << endl;
        exit(EXIT_FAILURE);
    }
    _isCut = bufferlen / MAXDATALEN - 1 + !!(bufferlen % MAXDATALEN);
    cout<<bufferlen<<" "<<bufferp<<endl;
    cout<<(int)_isCut<<endl;
    CtrlPack _Packet = {0, REQ_SUCC, _isCut, 0, 0};
    if (!succ_cnt)
    {
        bufferlen = sizeof(char);
        _isCut = 0;
        _Packet.Type = REQ_ERR_CONN;
        databuffer[0] = SND_ERR;
    }
    else
    {
        int offset = 0;
        for (int i = 0; i < rcver_cnt; ++i)
            if (snd_succ[i])
            {
                memcpy(snd_succ + offset, rcver_acc[i], MAXACCLEN);
                offset += MAXACCLEN;
            }

        // file databuffer from databuffer+bufferp to databuffer+bufferlen-1, length = bufferlen-bufferp
        switch (mess_type)
        {
        case TEXT_TYPE:
        {
            break;
        }
        case GRAPH_TYPE:
        {
            char _message[] = "[图片]";
            messagep = _message;
            break;
        }
        case FILE_TYPE:
        {
            char _message[] = "[文件]";
            messagep = _message;
            break;
        }
        default:
        {
            break;
        }
        }
        for(int i = 0; i < rcver_cnt; ++i)
        {
            if (!snd_succ[i])
                continue;
            mysql_insert_message(rcver_uid[i], sender, messagep);
        }
        mysql_insert_message(client->userid, sender, messagep);
    }

    for (unsigned char _Seq = 0; _Seq <= _isCut; ++_Seq)
    {
        unsigned int curlen = (_Seq == _isCut) ? (bufferlen - bufferp) : MAXDATALEN;
        unsigned int _Datalen = htonl(curlen);
        _Packet.Seq = _Seq;
        _Packet.Datalen = _Datalen;

        memcpy(buffer, &_Packet, CTRLPACKLEN);
        memcpy(buffer + CTRLPACKLEN, databuffer + bufferp, curlen);
        int _ws = client->Snd(buffer, CTRLPACKLEN);
        if (_ws == WRITE_CLOSE || _ws == WRITE_ERROR)
        {
            log_out_unexpected(client);
            delete[] databuffer;
            return -1;
        }

        bufferp += curlen;
    }
    delete[] databuffer;

    return 1;
}
int ServerSock::userlist_request_to_all()
{
    if(!userlist.size())
        return 1;

    int uid[MAXCONNNUM];
    int alivecnt = 0;
    map<int, ClientInfo *>::iterator mapit;
    for (mapit = userlist.begin(); mapit != userlist.end(); ++mapit)
        uid[alivecnt++] = mapit->first;

    char account[MAXACCLEN];
    char username[MAXNAMELEN];
    int datalen = (MAXACCLEN + MAXNAMELEN) * alivecnt;
    char *data = new (nothrow) char[datalen];
    int bufferp = 0;
    if (data == NULL)
    {
        cerr << "Server Run Out of Memory" << endl;
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < alivecnt; ++i)
    {
        mysql_get_account(uid[i], account);
        mysql_get_username(uid[i], username);
        memcpy(data + bufferp, account, MAXACCLEN);
        bufferp += MAXACCLEN;
        memcpy(data + bufferp, username, MAXNAMELEN);
        bufferp += MAXNAMELEN;
    }

    unsigned char _isCut = datalen / MAXDATALEN - 1 + !!(datalen % MAXDATALEN);
    CtrlPack _Packet = {0, REQ_USER, _isCut, 0, 0};
    char buffer[MAXBUFFERLEN];
    bufferp = 0;
    list<ClientInfo>::iterator listit;
    for (unsigned char _Seq = 0; _Seq <= _isCut; ++_Seq)
    {
        unsigned int curlen = (_Seq == _isCut) ? (datalen - bufferp) : MAXDATALEN;
        unsigned int _Datalen = htonl(curlen);
        _Packet.Seq = _Seq;
        _Packet.Datalen = _Datalen;
        memcpy(buffer, &_Packet, CTRLPACKLEN);
        memcpy(buffer + CTRLPACKLEN, data + bufferp, curlen);

        for (listit = clientlist.begin(); listit != clientlist.end(); ++listit)
        {
            ClientInfo &client = *listit;
            if (client.sockfd == ERRSOCKET || client.userid == NOTLOGGEDIN)
                continue;

            int _ws = client.Snd(buffer, CTRLPACKLEN + curlen);
            if (_ws == WRITE_CLOSE || _ws == WRITE_ERROR)
            {
                log_out_unexpected(&client);
                continue;
            }
        }
        bufferp += curlen;
    }
    delete[] data;

    return 1;
}

int ServerSock::log_out_unexpected(ClientInfo *client)
{
    cout << "[userid = " << client->userid << " ] log out from [ip = "
         << client->ip << ", port = " << client->port << " unexpected." << endl
         << endl;

    userlist.erase(client->userid);
    close(client->sockfd);
    FD_CLR(client->sockfd, &sockfds);
    client->sockfd = ERRSOCKET;

    userreq = 1;
    return 1;
}

int ServerSock::mysql_get_uid(const char *const account)
{
    string sqlqry;

    sqlqry = "select uid from user where account = \'";
    sqlqry.append(account).append("\';");

    mysql_query(mysql, sqlqry.c_str());
    result = mysql_store_result(mysql);
    row = mysql_fetch_row(result);
    int res = row == NULL ? -1 : atoi(row[0]);

    mysql_free_result(result);

    return res;
}
void ServerSock::mysql_get_account(const int uid, char *const account)
{
    string sqlqry;
    char uid_c[10];
    sprintf(uid_c, "%d", uid);

    sqlqry = "select account from user where uid = ";
    sqlqry.append(uid_c).append(";");

    mysql_query(mysql, sqlqry.c_str());
    result = mysql_store_result(mysql);
    row = mysql_fetch_row(result);

    strcpy(account, row[0]);
    mysql_free_result(result);
}
void ServerSock::mysql_get_username(const int uid, char *const username)
{
    string sqlqry;
    char uid_c[10];
    sprintf(uid_c, "%d", uid);

    sqlqry = "select username from user where uid = ";
    sqlqry.append(uid_c).append(";");

    mysql_query(mysql, sqlqry.c_str());
    result = mysql_store_result(mysql);
    row = mysql_fetch_row(result);

    strcpy(username, row[0]);
    mysql_free_result(result);
}
int ServerSock::mysql_compare_password(const int uid, const char *const password)
{
    string sqlqry;
    char uid_c[10];
    sprintf(uid_c, "%d", uid);

    sqlqry = "select count(*) from security where uid = ";
    sqlqry.append(uid_c).append(" and password_MD5 = md5(\'").append(password).append("\');");

    mysql_query(mysql, sqlqry.c_str());
    result = mysql_store_result(mysql);
    row = mysql_fetch_row(result);
    int res = atoi(row[0]) ? 1 : 0;

    mysql_free_result(result);

    return res;
}
bool ServerSock::mysql_check_first_time(const int uid)
{
    string sqlqry;
    char uid_c[10];
    sprintf(uid_c, "%d", uid);

    sqlqry = "select First_Time_Log_In from user where uid = ";
    sqlqry.append(uid_c).append(";");

    mysql_query(mysql, sqlqry.c_str());
    result = mysql_store_result(mysql);
    row = mysql_fetch_row(result);
    bool res = atoi(row[0]) ? 1 : 0;

    mysql_free_result(result);

    return res;
}
void ServerSock::mysql_insert_message(const int uid, const char* const account, const char* const message)
{
    string sqlqry;
    string curtime;

    sqlqry = "FROM_UNIXTIME(unix_timestamp(now()), \'%Y%m%d%H%i%S\')";
    mysql_query(mysql, sqlqry.c_str());
    result = mysql_store_result(mysql);
    row = mysql_fetch_row(result);
    curtime = row[0];

    char uid_c[10];
    sprintf(uid_c, "%d", uid);

    sqlqry = "insert into message";
    sqlqry.append(uid_c).append("(sender, time, message) values(\'").append(account).append("\', ");
    sqlqry.append("\'").append(curtime).append("\', ");
    sqlqry.append("\'").append(message).append("\');");
    mysql_query(mysql, sqlqry.c_str());
}
