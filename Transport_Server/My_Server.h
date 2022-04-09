#ifndef MY_SERVER_H_INCLUDED
#define MY_SERVER_H_INCLUDED
#include <bits/stdc++.h>
#include <winsock2.h>
#include "My_Packet.h"
#include <thread>
#include <mutex>
#include <Windows.h>
#include <unistd.h>
#include <cstdlib>
#include <queue>
#include <ctime>
#include <set>
#include <vector>
#include <chrono>
#include<bits/stdc++.h>

class MyServer
{
public:
    MyServer();
    void set_initialVariables(int port_number, int random, float p);
    bool startUP();
    void run_myServer();
    void my_client(int client);
    void send_ackServerPacket(Packet packet, int client);
    void send_packetsToClient(FILE *file, uint32_t num_packets, int client);
    void choose_lossPackets(int numOfPackets, float P);
    void remove_fromQueue(int seq_number);
    int resend_packets(int base, int client);
};

#endif // MY_SERVER_H_INCLUDED
