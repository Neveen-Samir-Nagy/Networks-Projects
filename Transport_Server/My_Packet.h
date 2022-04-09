#ifndef MY_PACKET_H_INCLUDED
#define MY_PACKET_H_INCLUDED
#include <bits/stdc++.h>
#include <winsock2.h>

struct Packet
{
    uint16_t cksum;
    uint16_t len;
    uint32_t seqno;
    char data[500];
};

struct Ack_Packet
{
    uint16_t cksum;
    uint16_t len;
    uint32_t ackno;
};

struct Ack_serverPacket
{
    uint32_t numOfPacket;
};

class MyPacket
{
public:
    Packet create_packet(uint16_t length, uint32_t sequenseNumber, char* data);
    Ack_Packet create_ackPacket(uint16_t length, uint32_t ackNumber);
    uint16_t calculate_checksum_packet(uint16_t length, uint32_t sequenseNumber, char* data);
    uint16_t calculate_checksum_ackPacket(uint16_t length, uint32_t ackNumber);
    bool check_corruption_packet(Packet packet);
    bool check_corruption_ackPacket(Ack_Packet ackPacket);
};

#endif // MY_PACKET_H_INCLUDED
