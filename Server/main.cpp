#include "server.h"

using namespace std;

int main()
{
    ServerSock * server = new(nothrow) ServerSock;
    if(server == NULL)
    {
        cerr << "Server Initial Error" << endl;
        return -1;
    }

    server->Server_Start();
    cout << "Server has stopped running" << endl;
    delete server;

    return 0;
}
