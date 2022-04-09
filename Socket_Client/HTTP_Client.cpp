#include "HTTP_Client.h"
#define HOSTNAME 127.0.0.1

/**-----------------------------------------------------------------------------------------
 * USEAGE : HTTP Client
 * Usage   : send request to server
 *-----------------------------------------------------------------------------------------*/

using namespace std;
int nRet;
fd_set fr;
int nSocket;
char *hostname, *file_URL;
int port;
sockaddr_in address;
WSADATA wsaData;

HTTPClient::HTTPClient(char *host_name, int port_number, char *file)
{
    port = port_number;
    hostname = host_name;
    file_URL = file;
}

bool HTTPClient::startUP_clinet()
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
    address.sin_addr.s_addr = inet_addr(hostname);
    memset(&(address.sin_zero), 0, 8);
    return true;
}

void HTTPClient::run_client()
{
    bool start = startUP_clinet();
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
        char buffer_client[1000000];
        FILE *file;
        file = fopen(file_URL, "r");
        if(file == NULL)
        {
            cout << "Invalid file." << endl;
            WSACleanup();
            exit(EXIT_FAILURE);
        }
        while(true)
        {
            std::string request;
            if(fgets(buffer_client, sizeof(buffer_client), file) != NULL)
            {
                puts(buffer_client);
                std::string request2(buffer_client);
                request = request2.substr(0, request2.size()-1);
                if(request.size() == 0 || request == "\r")
                {
                    continue;
                }
            }
            else
            {
                break;
            }
            if(strncmp(buffer_client, "GET", 3) == 0)
            {
                GET_request(request);
            }
            else if(strncmp(buffer_client, "POST", 4) == 0)
            {
                std::string data;
                //while(fgets(buffer_client, sizeof(buffer_client), file) != NULL)
                //{
                 //   std::string data2(buffer_client);
                  //  if(data2.substr(0, data2.size()-1).size() == 0)
                  //  {
                   //     break;
                   // }
                   // data += data2.substr(0, data2.size()-1);
               // }
                POST_request(request);
            }
            else
            {
                send(nSocket, buffer_client, sizeof(buffer_client), 0);
                cout << "Press any key...";
                getchar();
                std::fill( std::begin( buffer_client ), std::end( buffer_client ), 0 );
                recv(nSocket, buffer_client, sizeof(buffer_client), 0);
                cout << "The server sends:" << endl << buffer_client << endl;
                if(!buffer_client)
                {
                    free(buffer_client);
                }
            }
        }
        fclose(file);
        nRet = closesocket(nSocket);
        if(nRet < 0)
        {
            cout << "Failed to close connection." << endl;
            WSACleanup();
            exit(EXIT_FAILURE);
        }
        cout << "The connection is closed Successfully." << endl;
    }
    else
    {
        cout << "Failed to connect to the server." << endl;
        WSACleanup();
        exit(EXIT_FAILURE);
    }
}

void HTTPClient::GET_request(std::string client_request)
{
    stringstream ssin(client_request);
    std::string word;
    int i = 0;
    char buffer_client[100000];
    std::string send_request;
    while(ssin >> word)
    {
        if(i == 0)
        {
            send_request += word + " ";
        }
        else if(i == 1)
        {
            send_request += word + " HTTP/1.1\r\n";
        }
        else if(i == 2)
        {
            send_request += "HOST: " + word;
        }
        i++;
    }
    send_request += "\r\n\r\n";
    sprintf(buffer_client, "%s", send_request.c_str());
    send(nSocket, buffer_client, strlen(buffer_client), 0);
    cout << "Press any key...";
    getchar();
    std::fill( std::begin( buffer_client ), std::end( buffer_client ), 0 );
    bool check = check_okResponse();
    if(!check)
    {
        cout << "Time out, there was no data while ten second." << endl;
        return;
    }
    recv(nSocket, buffer_client, sizeof(buffer_client), 0);
    cout << "The server sends:" << endl << buffer_client << endl;
    if(!buffer_client)
    {
        free(buffer_client);
    }
}

void HTTPClient::POST_request(std::string client_request)
{
    stringstream ssin(client_request);
    std::string word;
    int i = 0;
    char buffer_client[100000];
    char help_buffer[100000];
    std::string send_request;
    while(ssin >> word)
    {
        if(i == 0)
        {
            send_request += word + " ";
        }
        else if(i == 1)
        {
            send_request += word + " HTTP/1.1\r\n";
        }
        else if(i == 2)
        {
            send_request += "HOST: " + word;
        }
        i++;
    }
    send_request += "\r\n\r\n";
    sprintf(buffer_client, "%s", send_request.c_str());
    send(nSocket, buffer_client, strlen(buffer_client), 0);
    cout << "Press any key...";
    getchar();
    std::fill( std::begin( buffer_client ), std::end( buffer_client ), 0 );
    bool check = check_okResponse();
    if(!check)
    {
        cout << "Time out, there was no response while ten second." << endl;
        return;
    }
    recv(nSocket, buffer_client, sizeof(buffer_client), 0);
    cout << "The server sends:" << endl << buffer_client << endl;
    FILE *file;
    file = fopen(file_URL, "rb");
    if(file != NULL)
    {
        std::string send_response;
        fseek(file, 0, SEEK_END);
        int length = ftell(file);
        rewind(file);
        int num = fread(help_buffer, sizeof(char), length, file);
        std::string data(help_buffer);
        std::fill( std::begin( buffer_client ), std::end( buffer_client ), 0 );
        sprintf(buffer_client, "%s", data.c_str());
        send(nSocket, buffer_client, sizeof(buffer_client), 0);
    }
    else
    {
        cout << "Invalid File." << endl;
        return;
    }
    //std::fill( std::begin( buffer_client ), std::end( buffer_client ), 0 );
    //sprintf(buffer_client, "%s", data.c_str());
    //send(nSocket, buffer_client, sizeof(buffer_client), 0);
    if(!buffer_client)
    {
        free(buffer_client);
    }
}

bool HTTPClient::check_okResponse()
{
        //for timeout
        struct timeval tval;
        tval.tv_sec = 10;
        tval.tv_usec = 0;
        FD_ZERO(&fr);
        FD_SET(nSocket, &fr); //DF_SET is used to put the socket into set
        nRet = select(nSocket, &fr, NULL, NULL, &tval);
        if(nRet <= 0)
        {
            return false;
        }
        else
        {
            return true;
        }
}
