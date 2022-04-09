#ifndef MY_CLIENT_H_INCLUDED
#define MY_CLIENT_H_INCLUDED
#include <bits/stdc++.h>
#include <winsock2.h>
#include <unistd.h>
#include "My_Packet.h"
#include <queue>

class MyClient
{
public:
    MyClient();
    void set_initialVariables(int port_number, char *IP, std::string filename);
    void run_myClient();
    bool startUP_myClient();
    void send_clientRequest();
    Ack_serverPacket_client receive_ack_fromServer();
    void receive_packetsFromServer(uint32_t numOfPackets);
    void write_inFile(std::string data);
    int remove_fromQueue(int seq);
};


#endif // MY_CLIENT_H_INCLUDED
