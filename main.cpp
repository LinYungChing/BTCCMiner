#include <iostream>
#include <cstdio>
#include <cstring>

#include "btc_pool.h"
#include "btc_miner.h"
#include "sha256.h"


int main(int argc, char **argv)
{
    btc::StratumPool pool("cn.stratum.slushpool.com", 3333,
                        "trashe725.zexlus", "fuckyou123");

    if(!pool)
    {
        std::cout << "Failed to connect to Stratum Pool..." << std::endl;
        exit(1);
    }

    btc::BTCBlock block = pool.getNewBlock();

    std::cout << block << std::endl;

	return 0;
}
