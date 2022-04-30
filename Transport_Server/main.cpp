#include <iostream>
#include <bits/stdc++.h>
#include "My_Server.h"
#include <set>
#include<iterator>

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
    MyServer* server = new MyServer();
    server->set_initialVariables(80, 0, 0.3);
    server->run_myServer();
    cout << "Finished." << endl;
    return 0;
}
