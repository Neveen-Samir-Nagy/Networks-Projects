#include <iostream>
#include "My_Packet.h"
#include "My_Client.h"

using namespace std;

int main()
{
    std::string input_file = "client.in.txt";
    int port;
    std::string file_name;
    char *IP;
    ifstream file;
    file.open(input_file);
    if(file.is_open())
    {
        file >> IP;
        file >> port;
        file >> file_name;
        file.close();
    }
    MyClient* client = new MyClient();
    client->set_initialVariables(port, IP, file_name);
    client->run_myClient();
    cout << "Finished." << endl;
    return 0;
}
