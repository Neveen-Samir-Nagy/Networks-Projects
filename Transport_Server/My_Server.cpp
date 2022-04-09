#include "My_Server.h"
#include "My_Packet.h"
#include "Congestion_Control.h"
/**-----------------------------------------------------------------------------------------
 * Usage   : send packets to client
 *-----------------------------------------------------------------------------------------*/

using namespace std;
int port;
int random_number;
std::set<int> lost_packets;
float PLP;
int nRet;
fd_set fr;
int size_data = 500;
int nSocket;
queue<Packet> myqueue;
sockaddr_in address, client_address;
WSADATA wsaData;
int num_clients = 0;
mutex mtx;
MyPacket* myPacket = new MyPacket();
MyCongestion* con = new MyCongestion(1, 8);
int update_base(vector<int> timer);

MyServer::MyServer()
{

}

void MyServer::set_initialVariables(int port_number, int random, float p)
{
    port = port_number;
    random_number = random;
    PLP = p;
}

bool MyServer::startUP()
{
    // Initialize Winsock, request the Winsock 2.2
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
    //create a socket
    nSocket = socket(AF_INET, SOCK_STREAM, 0);
    if(nSocket < 0)
    {
        cout << "Error to create socket for the Server." << endl;
        WSACleanup();
        return false;
    }
    else
    {
        cout << "The socket is created Successfully." << endl;
    }
    //give the socket an IP address and port number
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = INADDR_ANY;
    memset(&(address.sin_zero), 0, 8);
    //binds the socket to the IP address and port number
    nRet = bind(nSocket, (sockaddr *)&address, sizeof(sockaddr));
    if(nRet < 0)
    {
        cout << "Failed to bind to the local port." << endl;
        WSACleanup();
        return false;
    }
    else
    {
        cout << "Bind to the local port Successfully." << endl;
    }
    return true;
}

void MyServer::run_myServer()
{
    bool start = startUP();
    if(start)
    {
        memset(&client_address, 0, 8);
        //waits to get a request from client to connect to the server
        //7--> is the maximum number of connections
        nRet = listen(nSocket, 7);
        if(nRet < 0)
        {
            cout << "Failed to listen to the local port." <<endl;
            WSACleanup();
            exit(EXIT_FAILURE);
        }
        else
        {
            cout << "Started Listening to the local port Successfully." << endl;
        }
        while(true)
        {
            int client;
            int len = sizeof(sockaddr);
            client = accept(nSocket, (SOCKADDR *)&client, &len);
            if(client < 0)
            {
                cout << "Failed to establish the connection." << endl;
                WSACleanup();
                exit(EXIT_FAILURE);
            }
            else
            {
                cout << "The connection is established successfully to client: " << client << "." << endl;
                //create thread for each client or connection
                std::thread t(&MyServer::my_client,this,client);
                t.detach();//this will allow the thread run on its own see join function in docs for more information
                mtx.lock();
                num_clients++;
                mtx.unlock();
            }
        }
    }
    else
    {
        exit(EXIT_FAILURE);
    }

}

void MyServer::my_client(int client)
{
    Packet packet;
    nRet = recv(client, (char*)&packet, sizeof(packet), 0);
    if(nRet != sizeof(packet) || nRet <= 0)
    {
        cout << "Error in receiving packet." << endl;
        packet = Packet();
    }
    bool notCorrupt = myPacket->check_corruption_packet(packet);
    if(notCorrupt)
    {
        cout << packet.data << endl;
        send_ackServerPacket(packet, client);
    }
    mtx.lock();
    num_clients--;
    mtx.unlock();
}
void MyServer::send_ackServerPacket(Packet packet, int client)
{
    std::string myfile(packet.data);
    std::string full_path = "H:/CAndC++Projects/Transport_Server/" + myfile;
    FILE *file = fopen(full_path.c_str(),"rb");
    if(file == NULL)
    {
        cout << "Cannot found this file." << endl;
        return;
    }
    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    rewind(file);
    uint32_t numOfPacket = ceil(float(length) * 1.0 / size_data);
    cout << "Number of packets that are sent to client: " << numOfPacket << endl;
    Ack_serverPacket ack_serverPacket;
    ack_serverPacket.numOfPacket = numOfPacket;
    nRet = send(client, (const char*)&ack_serverPacket, sizeof(ack_serverPacket), 0);
    send_packetsToClient(file, numOfPacket, client);
}

void MyServer::send_packetsToClient(FILE *file, uint32_t num_packets, int client)
{
    choose_lossPackets((int)num_packets, PLP);
    fseek(file, 0, SEEK_END);
    int length = ftell(file);
    rewind(file);
    int base = 0;
    int next_seqno = 0;
    int current_index = 0;
    int my_timer = -1;
    int time_out = 0;
    int max_ack = 0;
    vector<int> timeOfPackets;
    for(int i=0; i<num_packets; i++)
    {
        timeOfPackets.push_back(-1);
    }
    while(base < num_packets)
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        con->update("");
        cout << "Window Size: " << con->get_windowSize() << ", Threshold: " << con->get_threshold() << ", State: " << con->get_currentState() << endl;
        cout << "Sequence_number: " << current_index << ", Base: " << base << endl;
        if(current_index < base + con->get_windowSize() && current_index < num_packets)  //can send packet
        {
            if(!lost_packets.count(current_index))
            {
                cout << "SEND" << endl;
                // can send this packet
                int current_size = min(size_data, length - (current_index*size_data));
                char buffer[current_size];
                fseek(file,size_data*current_index,SEEK_SET);
                fread(buffer, sizeof(char),current_size,file);
                Packet packet = myPacket->create_packet(current_size, current_index, buffer);
                nRet = send(client, (const char*)&packet, sizeof(packet), 0);
                myqueue.push(packet);
                next_seqno++;
                current_index++;
                timeOfPackets[current_index-1] = clock();
            }
            else
            {
                cout << "---> Packet with seqno: " << current_index << " will be lost." << endl;
                int current_size = min(size_data, length - (current_index*size_data));
                char buffer[current_size];
                fseek(file,size_data*current_index,SEEK_SET);
                fread(buffer, sizeof(char),current_size,file);
                Packet packet = myPacket->create_packet(current_size, current_index, buffer);
                myqueue.push(packet);
                lost_packets.erase(current_index);
                next_seqno++;
                current_index++;
                timeOfPackets[current_index-1] = clock();
                auto t2 = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
                cout << "Time Taken --> " << duration << endl;
                continue;
            }
        }
        else
        {
            cout << "Can't send this packet." << endl;
            int time_now = clock();
            int index = 0;
            for(auto it = timeOfPackets.begin(); it != timeOfPackets.end(); it++)
            {
                if(*it != -1)
                {
                    my_timer = *it;
                    if(((time_now - my_timer) / (double) CLOCKS_PER_SEC) >= time_out)
                    {
                        con->update("TIME_OUT");
                        cout << "TIME_OUT" << endl;
                        int current_size = min(size_data, length - (index*size_data));
                        char buffer[current_size];
                        fseek(file,size_data*index,SEEK_SET);
                        fread(buffer, sizeof(char),current_size,file);
                        Packet packet = myPacket->create_packet(current_size, index, buffer);
                        nRet = send(client, (const char*)&packet, sizeof(packet), 0);
                        my_timer = -1;
                        Ack_Packet ackPacket;
                        nRet = recv(client, (char*)&ackPacket, sizeof(ackPacket), 0);
                        timeOfPackets[ackPacket.ackno] = -1;
                        cout << "received ACK with seqno: " << ackPacket.ackno << " and length: " << ackPacket.len << endl;
                        if(max_ack < ackPacket.ackno)
                        {
                            max_ack = ackPacket.ackno;
                        }
                        con->update("NEW_ACK");
                    }
                }
                index++;
            }
            if((base=update_base(timeOfPackets)) == -1)
            {
                base = max_ack+1;
            }
            else
            {
                base=update_base(timeOfPackets);
            }
            auto t2 = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
            cout << "Time Taken --> " << duration << endl;
            continue;
        }
        Ack_Packet ackPacket;
        nRet = recv(client, (char*)&ackPacket, sizeof(ackPacket), 0);
        timeOfPackets[ackPacket.ackno] = -1;
        cout << "received ACK with seqno: " << ackPacket.ackno << " and length: " << ackPacket.len << endl;
        if(max_ack < ackPacket.ackno)
        {
            max_ack = ackPacket.ackno;
        }
        bool notCorrupt = myPacket->check_corruption_ackPacket(ackPacket);
        if(notCorrupt)
        {
            if(ackPacket.ackno < base)
            {
                cout << "DUP_ACK" << endl;
                con->update("DUP_ACK");
                timeOfPackets[ackPacket.ackno] = -1;
            }
            else
            {
                if(ackPacket.ackno == base)
                {
                    if((base=update_base(timeOfPackets)) == -1)
                    {
                        base = max_ack+1;
                    }
                    else
                    {
                        base=update_base(timeOfPackets);
                    }
                    cout << "NEW_ACK" << endl;
                    con->update("NEW_ACK");
                    timeOfPackets[ackPacket.ackno] = -1;
                }
                else
                {
                    timeOfPackets[ackPacket.ackno] = -1;
                    cout << "NEW_ACK" << endl;
                    con->update("NEW_ACK");
                }
            }
        }
        int time_now = clock();
        int index = 0;
        for(auto it = timeOfPackets.begin(); it != timeOfPackets.end(); it++)
        {
            if(*it != -1)
            {
                my_timer = *it;
                cout << "--->The rest of Time for packet: " << ((time_now - my_timer) / (double) CLOCKS_PER_SEC) << endl;
                if(((time_now - my_timer) / (double) CLOCKS_PER_SEC) >= time_out)
                {
                    con->update("TIME_OUT");
                    cout << "TIME_OUT" << endl;
                    int current_size = min(size_data, length - (index*size_data));
                    char buffer[current_size];
                    fseek(file,size_data*index,SEEK_SET);
                    fread(buffer, sizeof(char),current_size,file);
                    Packet packet = myPacket->create_packet(current_size, index, buffer);
                    nRet = send(client, (const char*)&packet, sizeof(packet), 0);
                    my_timer = -1;
                    Ack_Packet ackPacket;
                    nRet = recv(client, (char*)&ackPacket, sizeof(ackPacket), 0);
                    timeOfPackets[ackPacket.ackno] = -1;
                    cout << "received ACK with seqno: " << ackPacket.ackno << " and length: " << ackPacket.len << endl;
                    if(max_ack < ackPacket.ackno)
                    {
                        max_ack = ackPacket.ackno;
                    }
                    con->update("NEW_ACK");
                }
            }
            index++;
        }
        if((base=update_base(timeOfPackets)) == -1)
        {
            base = max_ack+1;
        }
        else
        {
            base=update_base(timeOfPackets);
        }
        auto t2 = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>( t2 - t1 ).count();
        cout << "Time Taken --> " << duration << endl;
    }
}

int update_base(vector<int> timer)
{
    int index = 0;
    for(auto it = timer.begin(); it != timer.end(); it++)
    {
        if(*it != -1)
        {
            return index;
        }
        index++;
    }
    return -1;
}
void MyServer::choose_lossPackets(int numOfPackets, float P)
{
    int numOfLossPackets = (int)ceil(P * numOfPackets);
    int i = 0;
    while(i < numOfLossPackets)
    {
        int random = int(rand() % numOfPackets);
        if(!lost_packets.count(random))
        {
            lost_packets.insert(random);
            i++;
        }
    }
}

void MyServer::remove_fromQueue(int seq_number)
{
    while(myqueue.front().seqno <= seq_number)
    {
        if(myqueue.empty())
        {
            break;
        }
        myqueue.pop();
    }
}

int MyServer::resend_packets(int base, int client)
{
    int new_base = base;
    Packet temp_array[myqueue.size()];
    int temp_arraySeq[myqueue.size()];
    int index = 0;
    while(!myqueue.empty())
    {
        cout << "Window Size: " << con->get_windowSize() << ", Threshold: " << con->get_threshold() << ", State: " << con->get_currentState() << endl;
        if(myqueue.front().seqno <= new_base+con->get_windowSize())
        {
            Packet packet = myqueue.front();
            cout << "Resend Packet with seqno: " << packet.seqno << endl;
            nRet = send(client, (const char*)&packet, sizeof(packet), 0);
            Ack_Packet ackPacket;
            nRet = recv(client, (char*)&ackPacket, sizeof(ackPacket), 0);
            cout << "received ACK with seqno: " << ackPacket.ackno << " and length: " << ackPacket.len << endl;
            bool notCorrupt = myPacket->check_corruption_ackPacket(ackPacket);
            if(notCorrupt)
            {
                myqueue.pop();
                new_base = ackPacket.ackno + 1;
                con->update("NEW_ACK");
            }
        }
        else
        {
            break;
        }
    }
    return new_base;
}
