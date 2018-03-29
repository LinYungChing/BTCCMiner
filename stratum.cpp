#include "stratum.hpp"

namespace stratum
{

    ////////////////// stratum worker //////////////////


    Worker::Worker() :
        client(), 
        msg_parser()
    {
    }

    Worker::Worker(const string &url, const int &port,
                   const string &worker, const string &password) :
            client(url, port),
            msg_parser(),
            worker(worker),
            password(password)
    {
        if(!this->_subscribe())
        {
            return false;
        }

        if(!this->_authorize())
        {
            return false;
        }
    }

    Worker::~Worker()
    {
        this->client.close();
    }

    bool Worker::setup(const string &url, const int &port, 
                       const string &worker, const string &password)
    {
        this->worker = worker;
        this->password = password;

        if(!this->client.setup(url, port))
        {
            return false;
        }

        if(!this->_subscribe())
        {
            return false;
        }

        if(!this->_authorize())
        {
            return false;
        }
        
    }

    Worker::operator bool()
    {
        return this->client();
    }

    bool Worker::_subscribe()
    {

        //TODO
    }

    bool Worker::_authorize()
    {
        //TODO
    }

    ////////////////////// Msg parser ////////////////


}
