#include <iostream>
#include <bits/stdc++.h>
#include "HTTP_Client.h"
#define PORT 80

using namespace std;

int main()
{
    HTTPClient* client = new HTTPClient("192.168.1.2", PORT, "H:/CAndC++Projects/Socket_Client/client.txt");
    client->run_client();
    return 0;
}
