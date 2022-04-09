#ifndef MY_PACKET_H_INCLUDED
#define MY_PACKET_H_INCLUDED
#include <bits/stdc++.h>
#include <winsock2.h>

struct Packet_client
{
    uint16_t cksum;
    uint16_t len;
    uint32_t seqno;
    char data[500];
};

struct Ack_Packet_client
{
    uint16_t cksum;
    uint16_t len;
    uint32_t ackno;
};

struct Ack_serverPacket_client
{
    uint32_t numOfPacket;
};

class MyPacket_client
{
public:
    Packet_client create_packet(uint16_t length, uint32_t sequenseNumber, char* data);
    Ack_Packet_client create_ackPacket(uint16_t length, uint32_t ackNumber);
    uint16_t calculate_checksum_packet(uint16_t length, uint32_t sequenseNumber, char* data);
    uint16_t calculate_checksum_ackPacket(uint16_t length, uint32_t ackNumber);
    bool check_corruption_packet(Packet_client packet);
    bool check_corruption_ackPacket(Ack_Packet_client ackPacket);
};

#endif // MY_PACKET_H_INCLUDED
