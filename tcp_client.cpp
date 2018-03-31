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
                  error_callback ec) :
            connect_status(false),
            sock_number(-1)
    {
        this->setup(address, port, ec);
    }

    // Destructor
    TCPClient::~TCPClient()
    {
        this->close();
    }

    // Utils
    bool TCPClient::setup(const std::string &address, const int &port,
                   error_callback ec)
    {
        if(connect_status == true)
            this->close();

        this->server_address = address;
        this->server_port = port;
        this->_error = ec;

        std::cout << "Addr: " << address << std::endl;
        std::cout << "port: " << port << std::endl;

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
        if(this->connect_status == true)
            ::close(sock_number);
        this->connect_status = false;
    }


    bool TCPClient::reconnect()
    {
        this->close();
        return this->_connect();
    }

    bool TCPClient::status()
    {
        return this->connect_status;
    }


    // send/recv
    //
    bool TCPClient::send(std::string data)
    {
        if(connect_status == true)
        {
            if(::send(sock_number, data.c_str(), data.size(), 0) < 0)
            {
                ERROR_RECONN(CONNECTION_ERROR, "Failed to send message to the server");
                return false;
            }
        }
        else
            return false;
        return true;
    }

    bool TCPClient::recv(std::string &data, size_t size)
    {
        char buffer[size+1];
        memset(&buffer[0], 0, sizeof(buffer));

        size_t count = ::recv(sock_number, buffer, size, 0);

        if( count < 0)
        {
            ERROR_RECONN(CONNECTION_ERROR, "Failed to recv message from server");
            return false;
        }

        buffer[count] = '\0';
        data = buffer;
        return true;
    }

    bool TCPClient::peek()
    {
        char buffer;
        if(::recv(sock_number, &buffer, 1, MSG_PEEK | MSG_DONTWAIT) > 0)
        {
            return true;
        }
        return false;
    }

    bool TCPClient::getline(std::string &data, char delim)
    {
        char buffer;
        std::string line;
        line.reserve(1024);

        char num_available;

        int count = 1;
        while(count > 0)
        {
            count = ::recv(sock_number, &buffer, 1, 0);
            
            if(count < 0)
            {
                ERROR_RECONN(CONNECTION_ERROR, "Failed to getline from server");
                return false;
            }
            if(count > 0)
            {
                if(buffer != delim)
                    line.push_back(buffer);
                else 
                    break;
            }
        }

        data = line;

        return true;
    }


    //Get Func
    std::string TCPClient::getAddress()   { return server_address; }
    int         TCPClient::getPort()      { return server_port; }


    // overload
    TCPClient::operator bool()
    {
        return this->connect_status;
    }

    // private function
    bool TCPClient::_connect()
    {
        // create socket
        sock_number = socket(PF_INET, SOCK_STREAM, 0);

        if(sock_number == -1)
        {
            ERROR_CALL(SOCKET_ERROR, "Failed to create socket");
            return false;
        }

        // create connection
        sock_info.sin_family = AF_INET;
        sock_info.sin_port = htons(this->server_port);
        if(connect(sock_number, (struct sockaddr *)&sock_info, sizeof(sock_info)) < 0)
        {
            if(errno != EINPROGRESS)
            {
                ERROR_CALL(CONNECTION_ERROR, "Failed to connect to the server");
                return false;
            }
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
    tcp::TCPClient client("127.0.0.1", 8765);

    if(!client)
    {
        std::cout << "Failed to connect to server: " 
                << client.getAddress() << ":" << client.getPort();
        exit(0);
    }

    client.send("Hello");

    std::string str;
    bool ret = client.recv(str, 10);
    
    if(ret)
    {
        std::cout << "Recv: " << str << std::endl;
    }
    else
    {
        std::cout << "error" << std::endl;
    }
    client.close();

    return 0;
}
    
#endif
