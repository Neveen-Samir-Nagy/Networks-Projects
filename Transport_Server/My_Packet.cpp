#include"My_Packet.h"
/**-----------------------------------------------------------------------------------------
 * USEAGE : packet
 * Usage   : create packet to send request to server
 *-----------------------------------------------------------------------------------------*/

 using namespace std;

 uint16_t MyPacket::calculate_checksum_ackPacket(uint16_t length, uint32_t ackNumber)
 {
     uint32_t check_sum = 0;
     check_sum += length;
     check_sum += ackNumber;
     while(check_sum >> 16)
     {
         check_sum = (check_sum >> 16) + (check_sum & 0xFFFF);
     }
     uint16_t check = (check_sum & 0xFFFF);
     check = ~check;
     return check;
 }

 uint16_t MyPacket::calculate_checksum_packet(uint16_t length, uint32_t sequenseNumber, char* data)
 {
     uint32_t check_sum = 0;
     for(int i=0; i < length; i++)
     {
         check_sum += data[i];
     }
     check_sum += length;
     check_sum += sequenseNumber;
     while(check_sum >> 16)
     {
         check_sum = (check_sum >> 16) + (check_sum & 0xFFFF);
     }
     uint16_t check = (check_sum & 0xFFFF);
     check = ~check;
     return check;
 }

 bool MyPacket::check_corruption_ackPacket(Ack_Packet ackPacket)
 {
     uint16_t checksum_ofPacket = calculate_checksum_ackPacket(ackPacket.len, ackPacket.ackno);
     if(ackPacket.cksum == checksum_ofPacket)
     {
         return true;
     }
     return false;
 }

 bool MyPacket::check_corruption_packet(Packet packet)
 {
     uint16_t checksum_ofPacket = calculate_checksum_packet(packet.len, packet.seqno, packet.data);
     if(packet.cksum == checksum_ofPacket)
     {
         return true;
     }
     return false;
 }

 Ack_Packet MyPacket::create_ackPacket(uint16_t length, uint32_t ackNumber)
 {
     Ack_Packet ackPacket;
     ackPacket.ackno = ackNumber;
     ackPacket.len = length;
     ackPacket.cksum = calculate_checksum_ackPacket(length, ackNumber);
     return ackPacket;
 }

 Packet MyPacket::create_packet(uint16_t length, uint32_t sequenseNumber, char* data)
 {
     Packet packet;
     packet.len = length;
     packet.seqno = sequenseNumber;
     strcpy(packet.data, data);
     packet.cksum = calculate_checksum_packet(length, sequenseNumber, data);
     return packet;
 }
