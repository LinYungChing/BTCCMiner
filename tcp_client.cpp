#include "tcp_client.hpp"
#include <iostream>

#define ERROR_CALL(e, msg)   _error(e, msg, this, false)
#define ERROR_RECONN(e, msg) _error(e, msg, this, true)

namespace tcp
{
    void default_error_callback(int error, const std::string &msg, TCPClient *client, bool reconnect)
    {
        std::cout << " :: [TCP ERROR] :: " << error << " :: " << msg << std::endl;

        // Reconnect
        if(reconnect){
            std::cout << " :: [TCP LOG] :: " << "try to reconnect to the server" << std::endl;
            client->reconnect();
        }
    }

    // Constructor
    TCPClient::TCPClient() : 
            connect_status(false),
            sock_number(-1)
    {
        //pass
    }

    TCPClient::TCPClient(const std::string &address, const int &port, 
                  const std::string &user, const std::string &password,
                  error_callback ec) :
            connect_status(false),
            sock_number(-1)
    {
        this->setup(address, port, user, password, ec);
    }

    // Destructor
    TCPClient::~TCPClient()
    {
        //TODO: Close()
    }

    // Utils
    bool TCPClient::setup(const std::string &address, const int &port,
                   const std::string &login, const std::string &password,
                   error_callback ec)
    {
        if(connect_status == true)
            this->close();

        this->server_address = address;
        this->server_port = port;
        this->server_user = user;
        this->server_password = password;
        this->_error = ec;

        // get hostname from address
        if(inet_addr(address.c_str()) == -1)
        {
            struct hostent *he;
            struct in_addr **addr_list;
            if( (he = gethostbyname(this->server_address.c_str())) == NULL)
            {
                ERROR_CALL(HOSTNAME_ERROR, "Failed to resolve hostname");
                return false;
            }

            addr_list = (struct in_addr **) he->h_addr_list;
            for(int i=0;addr_list[i] != NULL; ++i)
            {
                sock_info.sin_addr = *addr_list[i];
                break;
            }
        }
        else
        {
            sock_info.sin_addr.s_addr = inet_addr( this->server_address.c_str() );
        }

        // try to connect to the server
        return this->_connect();
    }

    // close socket
    void TCPClient::close()
    {
        close(sock_number);
        this->connect_status = false;
    }


    bool TCPClient::send(std::string data)
    {
        if(connect_status == true)
        {
            if(send(sock_number, data.c_str(), strlen(data.c_str()), 0) < 0)
            {
                ERROR_RECONN(CONNECTION_ERROR, "Failed to send message to the server");
                return false;
            }
        }
        else
            return false;
        return true;
    }

    std::string TCPClient::recv(size_t size = 4096)
    {
        char buffer[size+1];
        memset(&buffer[0], 0, sizeof(buffer));

        size_t count = recv(sock_number, buffer, size, 0);

        if( count < 0)
        {
            ERROR_RECONN(CONNECTION_ERROR, "Failed to recv message from the server");
            return nullptr;
        }

        buffer[count] = '\0';
        return std::string(buffer);
    }


    //Get Func
    std::string TCPClient::getAddress()   { return server_address; }
    int         TCPClient::getPort()      { return server_port; }
    std::string TCPClient::getUser()      { return server_user; }
    std::string TCPClient::getPassword()  { return server_password; }


    // private function

    bool _connect()
    {
        // create socket
        sock_number = socket(AF_INET, SOCK_STREAM, 0);
        if(sock_number == -1)
        {
            ERROR_CALL(SOCKET_ERROR, "Failed to create socket");
            return false;
        }

        // create connection
        sock_info.sin_family = AF_INET;
        sock_info.sin_port = htons(this->server_port);
        if(connect(sock_number, (struct sockaddr *)&sock_info, sizeof(server)) < 0)
        {
            ERROR_CALL(CONNECTION_ERROR, "Failed to connect to the server");
            return false;
        }

        return this->connect_status = true;
    }
}

#undef ERROR_CALL
#undef ERROR_RECONN

// Unit Test
#ifdef __TCP_CLIENT_UNITTEST__
int main(int argc, char **argv)
{
    tcp::TCPClient client("127.0.0.1", "8765", 
                          "zexlus1126", "fuckyou123");

    if(!client)
    {
        std::cout << "Failed to connect to server: " 
                << client.getAddress() << ":" << client.getPort();
        exit(0);
    }

    client.send("Hello");
    std::string str = client.recv(10);
    std::cout << "Recv: " << str << std::endl;

    client.close();

    return 0;
}
    
#endif