#pragma once

#include <string>
#include <cstring>

extern "C"{
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
}

namespace tcp
{
    class TCPClient;
    void default_error_callback(int error, const std::string &msg, 
                                TCPClient *client, bool reconnect = false);

    typedef void(*error_callback)(int error, const std::string &msg, 
                                  TCPClient *client, bool reconnect);

    class TCPClient
    {
    private:
        bool                connect_status;
        std::string         server_address;  // server ip address
        int                 server_port;  // server port
        std::string         server_user;
        std::string         server_password;

        error_callback      _error;

        int                 sock_number;
        struct sockaddr_in  sock_info;


    public:
        // Constructor
        TCPClient();
        TCPClient(const std::string &address, const int &port, 
                  const std::string &login, const std::string &password,
                  error_callback ec = default_error_callback);

        // Destructor
        ~TCPClient();

        // Utils
        bool setup(const std::string &address, const int &port,
                   const std::string &login, const std::string &password,
                   error_callback ec = default_error_callback);

        void close();

        bool reconnect();

        // Send/Recv 
        bool        send(std::string data);
        std::string recv(size_t size = 4096);

        // Get Func
        std::string getAddress();
        int         getPort();
        std::string getUser();
        std::string getPassword();

        // overload
        explicit operator bool();

    private:
        bool _connect();


    };  //TCPClient

    // exception code
    enum TCP_ERROR
    {
        SOCKET_ERROR       = 200,
        CONNECTION_ERROR   = 201,
        HOSTNAME_ERROR     = 202
    };
}
