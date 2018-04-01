#include "stratum.hpp"

#include <limits>

#ifndef _BTC_RETRY_CONNECTION_
    #define _BTC_RETRY_CONNECTION_ 3
#endif

namespace stratum
{

    ////////////////// stratum worker //////////////////


    // Constructor

    Worker::Worker() :
        client(),
        id_counter(1),
        connect_status(false)
    {
    }

    Worker::Worker(const std::string &url, const int &port,
                   const std::string &username, const std::string &password) :
            client(),
            id_counter(1),
            username(username),
            password(password),
            connect_status(false)
    {
        this->setup(url, port, username, password);
    }

    // Destructor
    
    Worker::~Worker()
    {
        this->close();
    }


    // Utils

    bool Worker::setup(const std::string &url, const int &port, 
                       const std::string &username, const std::string &password)
    {
        this->username = username;
        this->password = password;

        if(!this->client.setup(url, port))
        {
            return false;
        }

        if(!this->subscribe())
        {
            return false;
        }

        if(!this->authorize())
        {
            return false;
        }
        
        return this->connect_status = true;
    }

    
    bool Worker::status()
    {
        return this->connect_status;
    }

    void Worker::close()
    {
        if(this->client)
        {
            this->client.close();
        }

        this->connect_status = false;
    }

    bool Worker::empty()
    {
        return msgqueue.empty();
    }

    Message Worker::popMessage()
    {
        if(msgqueue.empty())
        {
            return Message::None();
        }
        Message msg = msgqueue.front();
        msgqueue.pop_front();
        return msg;
    }

    bool Worker::checkMessageByType(MsgType msgtype)
    {
        auto it = msgqueue.begin();
        for(it;it != msgqueue.end(); ++it)
        {
            if(it->getType() == msgtype)
                return true;
        }
        return false;
    }

    Message Worker::getMessageByType(MsgType msgtype)
    {
        Message msg = Message::None();

        auto it = msgqueue.begin();
        for(it;it != msgqueue.end(); ++it)
        {
            if(it->getType() == msgtype)
            {
                msg = *it;
                msgqueue.erase(it);
                break;
            }
        }
        return msg;
    }

    bool Worker::updateMessageQueue()
    {
        bool new_flag = false;
        std::string reply;
        if(!this->client.peek())
        {
            return new_flag;
        }

        while( this->_safe_getline(reply) && reply != "")
        {
            Message msg(reply);
            new_flag = true;
            msgqueue.push_back(msg);
            std::cout << reply << std::endl;   //call_std::cout
            reply.clear();
        }

        return new_flag;
    }

    MsgType Worker::getFirstMessageType()
    {
        return msgqueue.front().getType();
    }

    // client to server functions
    bool Worker::submit(const std::string job_id, const std::string extranonce2,
                        const std::string ntime, const std::string nonce)
    {
        int id = this->_get_counter();
        std::string string_msg = MsgParser::submit(this->username, job_id, extranonce2, 
                                                   ntime, nonce, id);
    #ifdef _DEBUG_
        std::cout << "Client: " << string_msg;  //call_std::cout
    #endif
        this->_safe_send(string_msg);

        Message msg = this->_wait_for_specific_id(id);

        if(msg.getType() == MsgType::NONE)
        {
            std::cout << "[in Worker::submit] Can't get server reply..." << std::endl;
            return false;
        }

    #ifdef _DEBUG_
        std::cout << "Server reply: " << msg.getJson() << std::endl; //call_std::cout

        std::cout << "  id: " << msg["id"] << std::endl;
        std::cout << "  result: " << msg["result"] << std::endl;
        std::cout << "  error: " << msg["error"] << std::endl;
    #endif

        return msg["error"]->type == json_null && (bool)msg["result"]->u.boolean;
    }


    //authorize
    bool Worker::authorize()
    {
        int id = this->_get_counter();

        std::string string_msg = MsgParser::authorize(this->username, this->password, id);
    #ifdef _DEBUG_
        std::cout << "Client: " << string_msg; //call_std::cout
    #endif
        this->_safe_send(string_msg);

        // wait for server reply
        Message msg = this->_wait_for_specific_id(id);

        if(msg.getType() == MsgType::NONE)
        {
            std::cout << "[in Worker::authorize] Can't get server reply..." << std::endl;
            return false;
        }

    #ifdef _DEBUG_
        std::cout << "Server reply: " << msg.getJson() << std::endl;  //call_std::cout

        std::cout << "  id: " << msg["id"] << std::endl;
        std::cout << "  result: " << msg["result"] << std::endl;
        std::cout << "  error: " << msg["error"] << std::endl;
    #endif

        return msg["error"]->type == json_null && (bool)msg["result"]->u.boolean;
    }


    bool Worker::subscribe()
    {

        int id = this->_get_counter();
        std::string string_msg = MsgParser::subscribe(id);
    #ifdef _DEBUG_
        std::cout << "Client: " << string_msg;  ////call_std::cout
    #endif
        this->_safe_send(string_msg);


        Message msg = this->_wait_for_specific_id(id);

        if(msg.getType() == MsgType::NONE)
        {
            std::cout << "[in Worker::subscribe] Can't get server reply..." << std::endl;
            return false;
        }

        this->setExtranonce(msg["result"]->u.array.values[1]->u.string.ptr);
        this->setExtranonce2Size(msg["result"]->u.array.values[2]->u.integer);

    #ifdef _DEBUG_
        std::cout << "Server reply: " << msg.getJson() << std::endl;  //call_std::cout

        std::cout << "  id: " << msg["id"] << std::endl;
        std::cout << "  result: " << std::endl
                  << "    extranonce1: " << msg["result"]->u.array.values[1] << std::endl
                  << "    extranonce2_size: " << msg["result"]->u.array.values[2] << std::endl;
        std::cout << "  error: " << msg["error"] << std::endl;
    #endif

        return msg["error"]->type == json_null;
    }
    
    bool Worker::get_transactions(std::string &job_id)
    {
        int id = this->_get_counter();
        std::string string_msg = MsgParser::get_transactions(job_id, id);

    #ifdef _DEBUG_
        std::cout << "Client: " << string_msg << std::endl; //call_str::cout
    #endif

        this->_safe_send(string_msg);

        Message msg = this->_wait_for_specific_id(id);
        if(msg.getType() == MsgType::NONE)
        {
            std::cout << "[in Worker::get_transactions] Can't get server reply..." << std::endl;
            return false;
        }

    #ifdef _DEBUG_
        std::cout << "Server reply: " << msg.getJson() << std::endl;

        std::cout << "  id: " << msg["id"] << std::endl;
        std::cout << "  result: " << msg["result"] << std::endl;
        std::cout << "  error: " << msg["error"] << std::endl;
    #endif

        return true;
    }


    //get/set
    std::string Worker::getUrl()
    {
        return this->client.getAddress();
    }

    int Worker::getPort()
    {
        return this->client.getPort();
    }

    std::string Worker::getUsername()
    {
        return this->username;
    }

    std::string Worker::getExtranonce()
    {
        return this->extranonce;
    }

    int Worker::getExtranonce2Size()
    {
        return this->extranonce2_size;
    }

    unsigned int Worker::getDifficulty()
    {
        return this->difficulty;
    }

    void Worker::setExtranonce(const std::string &enonce)
    {
        this->extranonce = enonce;
    }

    void Worker::setExtranonce2Size(const int enonce2_size)
    {
        this->extranonce2_size = enonce2_size;
    }

    void Worker::setDifficulty(const unsigned int diff)
    {
        this->difficulty = diff;
    }

    // Overload

    Worker::operator bool()
    {
        return this->connect_status;
    }

    // Private

    int Worker::_get_counter(bool increase)
    {
        int id = this->id_counter;

        if(increase)
        {
            this->id_counter++;

            if(id_counter == std::numeric_limits<int>::max() )
            {
                this->id_counter = 1; 
            }
        }

        return id;
    }


    Message Worker::_wait_for_specific_id(int id)
    {
        std::string reply;
        while( this->_safe_getline(reply) && reply != "")
        {
            Message msg(reply);
            if(msg["id"] != NULL && msg["id"]->type == json_integer)
            {
                if(msg["id"]->u.integer == id)
                {
                    return msg;
                }
            }
            msgqueue.push_back(msg);
            reply.clear();
        }
        return Message::None();
    }


    bool Worker::_reconnect()
    {
        //pass
    }

    bool Worker::_safe_send(std::string &msg)
    {
        int failed = 0;
        while(!this->client.send(msg) && failed < _BTC_RETRY_CONNECTION_)
        {
            failed++;
            std::cout << "[in Worker::_safe_send] Failed to send msg to Server..." << std::endl;  //call_std::cout
            std::cout << "[in Worker::_safe_send] reconnect " << failed << std::endl; //call_std::cout
            _reconnect();
        }

        if(failed >= 5)
            return false;
        return true;
    }

    bool Worker::_safe_recv(std::string &msg, size_t size)
    {
        int failed = 0;
        while(!this->client.recv(msg) && failed < _BTC_RETRY_CONNECTION_)
        {
            failed++;
            std::cout << "[in Worker::_safe_recv] Failed to recv msg from Server..." << std::endl;//call_std::cout
            std::cout << "[in Worker::_safe_recv] reconnect " << failed << std::endl;//call_std::cout
            _reconnect();
        }

        if(failed >= 5)
            return false;
        return true;
    }

    bool Worker::_safe_getline(std::string &msg, char delim)
    {
        int failed = 0;
        while(!this->client.getline(msg, delim) && failed < _BTC_RETRY_CONNECTION_)
        {
            failed++;
            std::cout << "[in Worker::_safe_getline] Failed to recv msg from Server..." << std::endl; //call_std::cout
            std::cout << "[in Worker::_safe_getline] reconnect " << failed << std::endl;//call_std::cout
            _reconnect();
        }
        
        
        if(failed >= 5)
            return false;
        return true;
    }
    
    
    ////////////////////// Message ///////////////////

    Message::Message() :
        _type(MsgType::NONE),
        json(),
        raw_value(NULL)
    {
    }

    Message::Message(const std::string &json) :
        _type(MsgType::NONE),
        json(json),
        raw_value(NULL)
    {
        parse();
    }

    Message::Message(const Message &msg) :
        _type(msg._type),
        json(msg.json),
        raw_value(NULL)
    {
        parse();
    }
       

    Message::~Message()
    {
        this->clear();
    }

    MsgType Message::getType()
    {
        return this->_type;
    }

    std::string &Message::getJson()
    {
        return this->json;
    }

    json_value *Message::getObject()
    {
        return this->raw_value;
    }

    bool Message::parse()
    {
        if(this->raw_value != NULL)
        {
            this->field.clear();
            json_value_free(this->raw_value);
        }
        this->raw_value = NULL;
        this->raw_value = json_parse(this->json.c_str(), this->json.size());
        if(this->raw_value != NULL)
            this->_parse_field_and_type();
        else
            this->_type = MsgType::NONE;
        return this->raw_value != NULL;
    }

    void Message::clear()
    {
        _type = MsgType::NONE;
        json = std::string();
        if(this->raw_value != NULL)
            json_value_free(this->raw_value);
        this->raw_value = NULL;
        this->field.clear();
    }


    json_value *Message::operator[](std::string fieldname)
    {
        if(this->field.find(fieldname) == this->field.end())
            return NULL;
        return this->field[fieldname];
    }

    Message &Message::operator=(const Message &msg)
    {
        this->clear();
        this->_type = msg._type;
        this->json = msg.json;
            this->parse();
        return *this;
    }

    Message Message::None()
    {
        Message msg;
        msg._type = MsgType::NONE;
        return msg;
    }

    // Private
    void Message::_parse_field_and_type()
    {
        if(this->raw_value == NULL)
            return;

        this->_type = MsgType::UNKNOWN;

        if(this->raw_value->type == json_object)
        {
            for(int x=0;x<this->raw_value->u.object.length; ++x)
            {
                std::string fieldname(this->raw_value->u.object.values[x].name);
                this->field[fieldname] = this->raw_value->u.object.values[x].value;

                if(fieldname == "method")
                {
                    json_value *v = this->raw_value->u.object.values[x].value;
                    this->_type = MsgType::match_type(v->u.string.ptr);
                }
            }
        }
    }


    ////////////////////// Msg parser ////////////////

    std::string MsgParser::subscribe(int id)
    {
        std::stringstream ss;
        ss << "{"
           << "\"id\": " << id << ", "
           << "\"method\": \"" << MsgType::match_type(MsgType::SUBSCRIBE) << "\", "
           << "\"params\": " << "[]"
           << "}\n";
        
        return ss.str();
    }

    std::string MsgParser::authorize(const std::string &username, 
                                     const std::string &password,
                                     int id)
    {
        std::stringstream ss;
        ss << "{"
           << "\"params\": [\"" << username << "\", \"" << password << "\"], "
           << "\"id\": " << id << ", "
           << "\"method\": \"" << MsgType::match_type(MsgType::AUTHORIZE) << "\""
           << "}\n";

        return ss.str();
    }

    std::string MsgParser::submit(const std::string &username, 
                                  const std::string &job_id,
                                  const std::string &extranonce2,
                                  const std::string &ntime,
                                  const std::string &nonce,
                                  int id)
    {
        std::stringstream ss;
        ss << "{"
           << "\"method\": \"" << MsgType::match_type(MsgType::SUBMIT) << "\", "
           << "\"params\": [\"" << username << "\", \""
                                << job_id << "\", \""
                                << extranonce2 << "\", \""
                                << ntime << "\", \""
                                << nonce << "\"], "
           << "\"id\": " << id
           << "}\n";

        return ss.str();
    }

    std::string MsgParser::get_transactions(const std::string &job_id,
                                           int id)
    {
        std::stringstream ss;
        ss << "{"
           << "\"method\": \"" << MsgType::match_type(MsgType::GET_TRANSACTIONS) << "\", "
           << "\"params\": [\""  << job_id << "\"],"
           << "\"id\": " << id
           << "}\n";

        return ss.str();
    }

    Message MsgParser::parse(const std::string &json_msg)
    {
        Message msg(json_msg);
        msg.parse();
        return msg;
    }


    ////////////////////// MsgType ///////////////////

    MsgType::MsgType(Type t) :
        t_(t)
    {
    }

    MsgType::operator Type() const
    {
        return this->t_;
    }

    MsgType MsgType::match_type(const std::string &method)
    {
        auto it = MsgType::s2t_mapping.find(method);
        if(it == MsgType::s2t_mapping.end())
        {
            return MsgType::UNKNOWN;
        }

        return it->second;
    }

    std::string MsgType::match_type(const MsgType &msg)
    {
        return MsgType::t2s_mapping[msg];
    }

    std::map<std::string, MsgType> MsgType::s2t_mapping = {
            {"none",                        MsgType::NONE},
            {"unknown",                     MsgType::UNKNOWN},
            {"mining.authorize",            MsgType::AUTHORIZE},
            {"mining.capabilities",         MsgType::CAPABILITIES},
            {"mining.get_transactions",     MsgType::GET_TRANSACTIONS},
            {"mining.submit",               MsgType::SUBMIT},
            {"mining.subscribe",            MsgType::SUBSCRIBE},
            {"mining.suggest_difficulty",   MsgType::SUGGEST_DIFFICULTY},
            {"mining.suggest_target",       MsgType::SUGGEST_TARGET},
            {"client.get_version",          MsgType::GET_VERSION},
            {"client.reconnect",            MsgType::RECONNECT},
            {"client.show_message",         MsgType::SHOW_MESSAGE},
            {"mining.notify",               MsgType::NOTIFY},
            {"mining.set_difficulty",       MsgType::SET_DIFFICULTY},
            {"mining.set_extranonce",       MsgType::SET_EXTRANONCE},
            {"mining.set_goal",             MsgType::SET_GOAL}
    };

    std::map<MsgType, std::string> MsgType::t2s_mapping = {
            {MsgType::NONE,                 "none"},
            {MsgType::UNKNOWN,              "unknown"},
            {MsgType::AUTHORIZE,            "mining.authorize"},
            {MsgType::CAPABILITIES,         "mining.capabilities"},
            {MsgType::GET_TRANSACTIONS,     "mining.get_transactions"},
            {MsgType::SUBMIT,               "mining.submit"},
            {MsgType::SUBSCRIBE,            "mining.subscribe"},
            {MsgType::SUGGEST_DIFFICULTY,   "mining.suggest_difficulty"},
            {MsgType::SUGGEST_TARGET,       "mining.suggest_target"},
            {MsgType::GET_VERSION,          "client.get_version"},
            {MsgType::RECONNECT,            "client.reconnect"},
            {MsgType::SHOW_MESSAGE,         "client.show_message"},
            {MsgType::NOTIFY,               "mining.notify"},
            {MsgType::SET_DIFFICULTY,       "mining.set_difficulty"},
            {MsgType::SET_EXTRANONCE,       "mining.set_extranonce"},
            {MsgType::SET_GOAL,             "mining.set_goal"}
    };

}

// UNITTEST
#ifdef __STRATUM_UNITTEST__

int main(int argc, char **argv)
{
    stratum::Worker worker("cn.stratum.slushpool.com", 3333,
                            "trashe725.cppminer", "password");
    if(!worker)
    {
        std::cout << "Failed to connect to slushpool..." << std::endl;
    }

    stratum::Message msg = worker.popMessage();

    while(msg.getType() != stratum::MsgType::NONE)
    {

        if(msg.getType() == stratum::MsgType::SET_DIFFICULTY)
        {
            std::cout << "set_difficulty \n" ;
            std::cout << "  : " << msg.getJson();
        }
        if(msg.getType() == stratum::MsgType::NOTIFY)
        {
            std::cout << "notify \n";
            std::cout << "  : " << msg.getJson();

            std::string job_id = msg["params"]->u.array.values[0]->u.string.ptr;
            std::string extranonce2 = "00000000";
            std::string ntime = msg["params"]->u.array.values[7]->u.string.ptr;
            std::string nonce = "e8832204";
        
            std::cout << "submit \n";
            std::cout << "  job_id: " << job_id << std::endl
                      << "  extranonce2: " << extranonce2 << std::endl
                      << "  ntime: " << ntime << std::endl
                      << "  nonce: " << nonce << std::endl;
            worker.submit(job_id, extranonce2, ntime, nonce);
        }
        msg = worker.popMessage();
    }

    std::cout << "close worker" << std::endl;
    worker.close();


    return 0;
}

#endif


