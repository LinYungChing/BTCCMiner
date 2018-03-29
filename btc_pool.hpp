#pragma once

#include <iostream>
#include <string>

extern "C"{
#include "json-parser/json.h"
}

#include "tcp_client.hpp"

namespace btc
{
    class BTCBlock
    {
    private:
    public:
    private:
    }

    class BTCPoolInterface
    {
    private:
    public:
        virtual BTCBlock getNewBlock() = 0;
        virtual bool submit(BTCBlock b) = 0;
    private:
    };


    // use stratum protocol:
    //     https://slushpool.com/help/manual/stratum-protocol
    class StratumPool : public BTCPoolInterface
    {
    private:
        tcp::TCPClient stratum_pool;
    public:
        // constructor
        StratumPool();
        StratumPool(const string &url, const int &port, 
                    const string &user, const string &password);

        // destructor
        ~StratumPool();

        // Utils
        bool setup(const string &url, const int &port,
                   const string &user, const string &password);

        // Overload
        explicit operator bool();
    private:
    };


    // getblocktemplate protocol:
    //     ...
    class HTTPPool : public BTCPoolInterface
    {
    private:
    public:
    private:
    };

}
