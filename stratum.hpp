#include <iostream>
#include <string>

extern "C"{
#include "json-parser/json.h"
}

#include "tcp_client.hpp"

namespace stratum
{

    class Worker;
    class MsgParser;

    class Worker
    {
    private:
        tcp::TCPClient client;
        MsgParser msg_parser;
    
        std::string worker;
        std::string password;

    public:
        // Constructor
        Worker();
        Worker(const string &url, const int &port,
               const string &worker, const string &password);

        // Destructor
        ~Worker();

        // Utils
        bool setup(const string &url, const int &port,
                   const string &worker, const string &password);

        // Overlord
        explicit operator bool();

    private:

        bool _subscribe();
        bool _authorize();
    }



    class MsgParser
    {
    private:
    public:
    private:
    }

    
}
