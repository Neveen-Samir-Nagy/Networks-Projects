#include "HTTP_Server.h"
#define PORT 80

/**-----------------------------------------------------------------------------------------
 * USEAGE : HTTP Server
 * Take   : request from client
 *-----------------------------------------------------------------------------------------*/

using namespace std;
int nRet;
int port;
int nSocket;
sockaddr_in address;
WSADATA wsaData;
int size_data = 500;
fd_set fr;
int num_clients = 0;
mutex mtx;
int random_number;
float PLP;
MyPacket* myPacket = new MyPacket();

HTTPServer::HTTPServer(int port_number, int random, float p)
{
    port = port_number;
    random_number = random;
    PLP = p;
}

bool HTTPServer::startUP_server()
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

void HTTPServer::run_server()
{
    bool start = startUP_server();
    if(start)
    {
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
                std::thread t(&HTTPServer::my_client,this,client);
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

void HTTPServer::my_client(int client)
{
    Packet packet;
    nRet = recv(client, (char*)&packet, sizeof(packet), 0);
    if(nRet != sizeof(packet) || nRet <= 0)
    {
        cout << "Error in receiving packet." << endl;
        packet = Packet();
    }
    cout << packet.data << endl;
    send_ackServerPacket(packet, client);
    mtx.lock();
    num_clients--;
    mtx.unlock();
}

void HTTPServer::send_ackServerPacket(Packet packet, int client)
{
    FILE *file = fopen(packet.data,"rb");
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
    //send_packetsToClient(file, numOfPacket, client);
}

void HTTPServer::GET_request(std::string client_request, int client)
{
    cout << "Receive GET request." << endl;
    char buffer_server[100000];
    char help_buffer[100000];
    std::fill( std::begin( buffer_server ), std::end( buffer_server ), 0 );
    stringstream ssin(client_request);
    std::string path;
    ssin >> path;
    ssin >> path; //path of file
    path.erase(path.begin() + 0);
    FILE *file;
    file = fopen(path.c_str(), "rb");
    if(file != NULL)
    {
        std::string send_response;
        send_response += "HTTP/1.1 200 Okay\r\nContent-Length: ";
        fseek(file, 0, SEEK_END);
        int length = ftell(file);
        rewind(file);
        send_response += to_string(length) + "\r\n\r\n";
        int num = fread(help_buffer, sizeof(char), length, file);
        std::string data(help_buffer);
        send_response += data + "\r\n";
        sprintf(buffer_server, "%s", send_response.c_str());
        send(client, buffer_server, strlen(buffer_server), 0);
        fclose(file);
    }
    else
    {
        send(client, "HTTP/1.1 404 Not Found\r\n" ,strlen("HTTP/1.1 404 Not Found\r\n"), 0);
    }
}

void HTTPServer::POST_request(std::string client_request, int client)
{
    cout << "Receive POST request." << endl;
    char buffer_server[100000];
    char buffer[100000];
    std::fill( std::begin( buffer_server ), std::end( buffer_server ), 0 );
    stringstream ssin(client_request);
    std::string path;
    ssin >> path;
    ssin >> path; //path of file
    path.erase(path.begin() + 0);
    FILE *file;
    file = fopen(path.c_str(), "a+");
    if(file != NULL)
    {
        std::string send_response;
        send_response += "HTTP/1.1 200 Okay\r\n";
        sprintf(buffer_server, "%s", send_response.c_str());
        send(client, buffer_server, strlen(buffer_server), 0);
        bool check = check_data(client);
        if(!check)
        {
            cout << "Time out, there was no data while ten second." << endl;
            closesocket(client);
            return;
        }
        std::fill( std::begin( buffer_server ), std::end( buffer_server ), 0 );
        recv(client, buffer_server, sizeof(buffer_server), 0);
        cout << "The client sends the data: " << buffer_server << endl;
        fwrite(buffer_server, strlen(buffer_server), 1, file);
        fclose(file);
    }
    else
    {
        send(client, "HTTP/1.1 404 Not Found\r\n" ,strlen("HTTP/1.1 404 Not Found\r\n"), 0);
    }
}

bool HTTPServer::check_data(int client)
{
    //for timeout
        struct timeval tval;
        tval.tv_sec = 10;
        tval.tv_usec = 0;
        FD_ZERO(&fr);
        FD_SET(client, &fr); //DF_SET is used to put the socket into set
        nRet = select(client, &fr, NULL, NULL, &tval);
        if(nRet <= 0)
        {
            return false;
        }
        else
        {
            return true;
        }
}
