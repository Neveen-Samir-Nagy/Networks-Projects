#ifndef HTTP_SERVER_H_INCLUDED
#define HTTP_SERVER_H_INCLUDED
#include <bits/stdc++.h>
#include <winsock2.h>
#include <thread>
#include <mutex>
#include "My_Packet.h"

class HTTPServer
{
public:
    HTTPServer(int port_number, int random, float p);
    void run_server();
private:
    bool startUP_server();
    void my_client(int client);
    void GET_request(std::string client_request, int client);
    void POST_request(std::string client_request, int client);
    bool check_data(int client);
    void send_ackServerPacket(Packet packet, int client);
};

#endif // HTTP_SERVER_H_INCLUDED
