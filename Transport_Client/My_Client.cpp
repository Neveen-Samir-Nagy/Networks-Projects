#include"My_Client.h"
#include "My_Packet.h"
#include <string.h>
#include <stdlib.h>
/**-----------------------------------------------------------------------------------------
 * Usage   : run client side
 *-----------------------------------------------------------------------------------------*/

using namespace std;
int nRet;
fd_set fr;
int nSocket;
queue<Packet_client> myqueue;
int port;
sockaddr_in address;
WSADATA wsaData;
char *ip;
std::string file_name;
MyPacket_client* myPacket = new MyPacket_client();

MyClient::MyClient()
{

}

void MyClient::set_initialVariables(int port_number, char *IP, std::string filename)
{
    port = port_number;
    ip = IP;
    file_name = filename;
}

bool MyClient::startUP_myClient()
{
    //Initialize Winsock, request the Winsock 2.2
    nRet = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(nRet < 0)
    {
        cout << "Failed to startup the WSADATA." << endl;
        WSACleanup();
        return false;
    }
    else
    {
        cout << "The WSADATA initialized." << endl;
    }
    nSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(nSocket < 0)
    {
        cout << "Error to create socket for the Client." << endl;
        WSACleanup();
        return false;
    }
    else
    {
        cout << "The socket is created Successfully." << endl;
    }
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(ip);
    memset(&(address.sin_zero), 0, 8);
    return true;
}

void MyClient::run_myClient()
{
    bool start = startUP_myClient();
    if(start)
    {
        nRet = connect(nSocket, (sockaddr *)&address, sizeof(address));
        if(nRet < 0)
        {
            cout << "Failed to connect." << endl;
            WSACleanup();
            exit(EXIT_FAILURE);
        }
        else
        {
            cout << "Connect to the server Successfully." << endl;
        }
        send_clientRequest();
        closesocket(nSocket);
    }
    else
    {
        cout << "Error in creation socket." << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }

}

void MyClient::send_clientRequest()
{
    char data[500];
    strcpy(data, file_name.c_str());
    Packet_client packet = myPacket->create_packet(file_name.size(), 0, data);
    cout << "packet length: " << packet.len << " packet seqno: " << packet.seqno << " packet checksum: " << packet.cksum << " packet data: " << packet.data<< endl;
    nRet = send(nSocket, (const char*)&packet, sizeof(packet), 0);
    Ack_serverPacket_client serverPacket = receive_ack_fromServer();
    receive_packetsFromServer(serverPacket.numOfPacket);
}

Ack_serverPacket_client MyClient::receive_ack_fromServer()
{
    Ack_serverPacket_client serverPacket;
    int received = recv(nSocket, (char*)&serverPacket, sizeof(serverPacket), 0);
    if(received != sizeof(serverPacket) || received <= 0)
    {
        perror("Not received ACK.");
        return Ack_serverPacket_client();
    }
    cout << "Received ACK Successfully." << endl;
    return serverPacket;
}

void MyClient::receive_packetsFromServer(uint32_t numOfPackets)
{
    int count_packets = 0;
    int expect_seqno = 0;
    int max_seq = 0;
    while(count_packets < numOfPackets)
    {
        cout << "Current seqno: " << expect_seqno << endl;
        Packet_client packet;
        int received = recv(nSocket, (char*)&packet, sizeof(packet), 0);
        cout << "received packet with seqno: " << packet.seqno << " and length: " << packet.len << endl;
        if(received != sizeof(packet) || received <= 0)
        {
            perror("Not received Packet.");
            packet = Packet_client();
        }
        bool notCorrupt = myPacket->check_corruption_packet(packet);
        if(notCorrupt)
        {
            Ack_Packet_client ackPacket = myPacket->create_ackPacket(sizeof(uint32_t) + sizeof(uint32_t), packet.seqno);
            nRet = send(nSocket, (const char*)&ackPacket, sizeof(ackPacket), 0);
        }
        if(notCorrupt && expect_seqno == packet.seqno)
        {
            cout << "write data of packet with seqno: " << packet.seqno << endl;
            std::string data(packet.data);
            write_inFile(data);
            expect_seqno = remove_fromQueue(expect_seqno);
            count_packets = expect_seqno;
        }
        else if(notCorrupt && expect_seqno != packet.seqno)
        {
            myqueue.push(packet);
            if(max_seq < packet.seqno)
            {
                max_seq = packet.seqno;
            }
        }
    }
}

void MyClient::write_inFile(std::string data)
{
    std::string full_path = "H:/CAndC++Projects/Transport_Client/" + file_name;
    FILE *file = fopen(full_path.c_str(),"a");
    if(file != NULL)
    {
        fwrite(data.c_str(), sizeof(char), data.size(),file);
        fclose(file);
    }
    else
    {
        cout << "Not found file." << endl;
    }

}

int MyClient::remove_fromQueue(int seq)
{
    int expected_newSeq = seq+1;
    while(!myqueue.empty())
    {
        if(myqueue.front().seqno == expected_newSeq)
        {
            expected_newSeq++;
            Packet_client packet = myqueue.front();
            cout << "write data of packet with seqno: " << packet.seqno << endl;
            std::string data(packet.data);
            write_inFile(data);
            myqueue.pop();
        }
        else
        {
            break;
        }

    }
    return expected_newSeq;
}
