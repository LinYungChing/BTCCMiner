#include "btc_pool.hpp"

namespace btc
{

    ///////////////// Stratum Pool ///////////////
    //

    StratumPool::StratumPool() :
            stratum_pool()
    {
    }

    StratumPool::StratumPool(const std::string &url, const int &port,
                             const std::string &username, const std::string &password) :
                stratum_pool(url, port, username, password)
    {
    }

    StratumPool::~StratumPool()
    {
        this->close();
    }

    bool StratumPool::setup(const std::string &url, const int &port,
                            const std::string &username, const std::string &password)
    {
        this->stratum_pool.setup(url, port, username, password);
    }

    bool StratumPool::get_transactions(std::string job_id)
    {
        return this->stratum_pool.get_transactions(job_id);
    }

    bool StratumPool::detectNewBlock()
    {
        stratum_pool.updateMessageQueue();
        return stratum_pool.checkMessageByType(stratum::MsgType::NOTIFY);
    }


    BTCBlock StratumPool::getNewBlock()
    {
        BTCBlock b;

        stratum_pool.updateMessageQueue();
        this->_flush_msgqueue_until_next_block();

        stratum::Message msg = stratum_pool.getMessageByType(stratum::MsgType::NOTIFY);
        
        if(msg.getType() == stratum::MsgType::NOTIFY)
        {
            b.job_id = msg["params"]->u.array.values[0]->u.string.ptr;
            b.prevhash = msg["params"]->u.array.values[1]->u.string.ptr;
            b.coinbase1 = msg["params"]->u.array.values[2]->u.string.ptr;
            b.coinbase2 = msg["params"]->u.array.values[3]->u.string.ptr;
            for(int v = 0;v < msg["params"]->u.array.values[4]->u.array.length;++v)
            {
                b.merkle_branch.push_back(msg["params"]->u.array.values[4]->u.array.values[v]->u.string.ptr);
            }
            b.version = msg["params"]->u.array.values[5]->u.string.ptr;
            b.nbits = msg["params"]->u.array.values[6]->u.string.ptr;
            b.ntime = msg["params"]->u.array.values[7]->u.string.ptr;
            b.clean_job = (bool)msg["params"]->u.array.values[8]->u.boolean;
            b.extranonce1 = this->stratum_pool.getExtranonce();
            b.difficulty = this->stratum_pool.getDifficulty();
            b.extranonce2_size = this->stratum_pool.getExtranonce2Size();
        }

        return b;
    }

    bool StratumPool::submit(BTCBlock b)
    {
        return stratum_pool.submit(b.job_id, b.extranonce2, b.ntime, b.nonce);
    }

    StratumPool::operator bool()
    {
        return this->stratum_pool.status();
    }

    void StratumPool::close()
    {
        stratum_pool.close();
    }

    //get/set

    std::string StratumPool::getExtranonce()
    {
        return this->stratum_pool.getExtranonce();
    }

    unsigned int StratumPool::getDifficulty()
    {
        return this->stratum_pool.getDifficulty();
    }

    int StratumPool::getExtranonce2Size()
    {
        return this->stratum_pool.getExtranonce2Size();
    }

    //private
    
    bool StratumPool::_set_extranonce(std::string enonce)
    {
        std::cout << "Update extranonce1: " << enonce << std::endl; //call_std::cout
        this->stratum_pool.setExtranonce(enonce);
        return true;
    }

    bool StratumPool::_set_difficulty(unsigned int dif)
    {
        std::cout << "Update difficulty: " << dif << std::endl; //call_std::cout
        this->stratum_pool.setDifficulty(dif);
        return true;
    }

    bool StratumPool::_set_extranonce2_size(int size)
    {
        std::cout << "Update extranonce2 size: " << size << std::endl; // call_std::cout
        this->stratum_pool.setExtranonce2Size(size);
        return true;
    }

    bool StratumPool::_flush_msgqueue_until_next_block()
    {
        while(this->stratum_pool.getFirstMessageType() != stratum::MsgType::NOTIFY && !this->stratum_pool.empty())
        {
            stratum::Message msg = this->stratum_pool.popMessage();
            if(msg.getType() == stratum::MsgType::SET_DIFFICULTY)
            {
                this->_set_difficulty(msg["params"]->u.array.values[0]->u.integer);
            }
            else if(msg.getType() == stratum::MsgType::SET_EXTRANONCE)
            {
                this->_set_extranonce(msg["params"]->u.array.values[0]->u.string.ptr);
            }
            else if(msg.getType() == stratum::MsgType::SHOW_MESSAGE)
            {
                std::cout << "Message from server: " << msg["params"]->u.array.values[0]->u.string.ptr << std::endl;  //call_std::cout
            }
            else
            {
                std::cout << "Recv Message: " << msg.getJson() << std::endl;
            }
        }

        if(this->stratum_pool.empty())
            return false;
        return true;
    }

}

// UNITTEST
#ifdef __STRATUM_POOL_UNITTEST__

int main(int argc, char **argv)
{
    btc::StratumPool pool("cn.stratum.slushpool.com", 3333,
                          "trashe725.cppminer", "password");

    if(!pool)
    {
        std::cout << "Failed to connect to slushpool...." << std::endl;
        return 1;
    }

    std::cout << "detect new block..." << std::endl;

    btc::BTCBlock block;
    if(!pool.detectNewBlock())
    {
        std::cout << "No block found..." << std::endl;
        return 0;
    }
    else
    {
        std::cout << "New block found!" << std::endl;
    }
    
    block = pool.getNewBlock();

    std::cout << "job_id: " << block.job_id << std::endl;
    std::cout << "prevhash: " << block.prevhash << std::endl;
    std::cout << "coinbase1: " << block.coinbase1 << std::endl;
    std::cout << "coinbase2: " << block.coinbase2 << std::endl;
    std::cout << "version: " << block.version << std::endl;
    std::cout << "nbits: " << block.nbits << std::endl;
    std::cout << "ntime: " << block.ntime << std::endl;
    std::cout << "clean_job: " << (block.clean_job ? "true":"false") << std::endl;

    block.extranonce2 = "00000000";
    block.nonce = "e8832204";

    pool.get_transactions(block.job_id);

    if(pool.submit(block))
    {
        std::cout << "submit successfully" << std::endl;
    }
    else
    {
        std::cout << "submit failed" << std::endl;
    }

    std::cout << "close pool" << std::endl;
    pool.close();

    return 0;
}

#endif

