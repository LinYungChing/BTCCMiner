#pragma once

#include <iostream>
#include <vector>
#include <string>

extern "C"{
#include "json-parser/json.h"
}

#include "stratum.hpp"

namespace btc
{
    struct BTCBlock
    {
        std::string job_id;
        std::string prevhash;
        std::string coinbase1;
        std::string coinbase2;
        std::vector<std::string> merkle_branch;
        std::string version;
        std::string nbits;
        std::string ntime;
        std::string extranonce1;
        std::string extranonce2;
        std::string nonce;
        bool clean_job;
        unsigned int difficulty;
        int extranonce2_size;

        BTCBlock();
        bool valid();
        static BTCBlock None();
    };



    class BTCPoolInterface
    {
    private:
    public:
        virtual bool detectNewBlock() = 0;
        virtual bool detectNewMessage() = 0;
        virtual BTCBlock getNewBlock() = 0;
        virtual bool submit(BTCBlock b) = 0;
    private:
    };


    // use stratum protocol:
    //     https://slushpool.com/help/manual/stratum-protocol
    class StratumPool : public BTCPoolInterface
    {
    private:
        stratum::Worker stratum_pool;

    public:
        // constructor
        StratumPool();
        StratumPool(const std::string &url, const int &port, 
                    const std::string &username, const std::string &password);

        // destructor
        ~StratumPool();

        // Utils
        bool setup(const std::string &url, const int &port,
                   const std::string &username, const std::string &password);

        virtual bool detectNewBlock() override;
        virtual bool detectNewMessage() override;
        virtual BTCBlock getNewBlock() override;
        virtual bool submit(BTCBlock b) override;

        //stratum unique method
        bool get_transactions(std::string job_id);

        void close();

        //get/set
        std::string getExtranonce();
        unsigned int getDifficulty();
        int getExtranonce2Size();

        // Overload
        explicit operator bool();

    private:
        bool _set_extranonce(std::string enonce);
        bool _set_difficulty(unsigned int dif);
        bool _set_extranonce2_size(int size);
        bool _flush_msgqueue_until_next_block();
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
