#include <iostream>
#include <bits/stdc++.h>
#include "HTTP_Server.h"
#define PORT 80

using namespace std;

int main()
{
    int port,random_number;
    float PLP;
    std::string input_file = "server.in.txt";
    ifstream file;
    file.open(input_file);
    if(file.is_open())
    {
        file >> port;
        file >> random_number;
        file >> PLP;
        file.close();
    }
    HTTPServer* server = new HTTPServer(80, 0, 0.1);
    server->run_server();
    cout << "Finished." << endl;

    return 0;
}
