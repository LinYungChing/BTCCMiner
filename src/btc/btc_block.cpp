#include "btc_block.hpp"

namespace btc
{
    /////////////////// BTCBlock ////////////////
    //

    BTCBlock::BTCBlock() :
        clean_job(false)
    {
    }

    BTCBlock BTCBlock::None()
    {
        return BTCBlock();
    }

    bool BTCBlock::valid()
    {
        return job_id != "";
    }

}
