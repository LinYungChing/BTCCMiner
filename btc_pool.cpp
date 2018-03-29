#include "btc_pool.hpp"

namespace btc
{
    StratumPool::StratumPool() :
            stratum_pool()
    {
    }

    StratumPool::StratumPool(const string &url, const int &port,
                             const string &user, const string &password) :
                stratum_pool(url, port, user, password)
    {
    }

    StratumPool::~StratumPool()
    {
        stratum_pool.close();
    }

    bool StratumPool::setup(const string &url, const int &port,
                            const string &user, const string &password)
    {
        this->stratum_pool.setup(url, port, user, password);
    }

    

    StratumPool::operator bool()
    {
        return this->stratum_pool.status();
    }
}
