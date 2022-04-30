#ifndef HTTP_CLIENT_H_INCLUDED
#define HTTP_CLIENT_H_INCLUDED
#include <bits/stdc++.h>
#include <winsock2.h>


class HTTPClient
{
public:
    HTTPClient(char *host_name, int port_number, char *file);
    void run_client();
private:
    bool startUP_clinet();
    void GET_request(std::string client_request);
    void POST_request(std::string client_request);
    bool check_okResponse();
};

#endif // HTTP_CLIENT_H_INCLUDED
