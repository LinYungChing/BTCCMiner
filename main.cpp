#include <iostream>
#include "sha256.h"
#include "btc/btc_pool.hpp"

int main(int argc, char **argv)
{

    btc::StratumPool pool("cn.stratum.slushpool.com", 3333,
                        "trashe725.cppminer", "fuckyou123");

    pool.close();
    return 0;
}
