#include "stratum.hpp"

namespace stratum
{

    ////////////////// stratum worker //////////////////


    // Constructor

    Worker::Worker() :
        client(), 
        connect_status(false)
    {
    }

    Worker::Worker(const std::string &url, const int &port,
                   const std::string &username, const std::string &password) :
            client(),
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

        if(!this->_subscribe())
        {
            return false;
        }

        if(!this->_authorize())
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

    // Overload

    Worker::operator bool()
    {
        return this->connect_status;
    }


    // Private

    bool Worker::_subscribe()
    {
        this->client.send(MsgParser::subscribe());
        std::string reply = this->client.recv(4000);
        std::cout << "subscribe: " << reply << std::endl;
        return true;
    }

    bool Worker::_authorize()
    {
        this->client.send(MsgParser::authorize(this->username, this->password));
        std::string reply = this->client.recv(4000);
        std::cout << "authorize: " << reply << std::endl;
        return true;
    }

    ////////////////////// Message ///////////////////

    Message::Message() :
        _type(MsgType::NONE),
        json(),
        value(NULL)
    {
    }

    Message::Message(const std::string &json) :
        _type(MsgType::NONE),
        json(json),
        value(NULL)
    {
    }

    Message::Message(const Message &msg) :
        _type(msg._type),
        json(msg.json),
        value(NULL)
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
        return this->value;
    }

    bool Message::parse()
    {
        this->value = json_parse(this->json.c_str(), this->json.size());
        this->_parse_type();
        return this->value != NULL;
    }

    void Message::clear()
    {
        _type = MsgType::NONE;
        json = std::string();
        json_value_free(value);
    }

    // Private
    void Message::_parse_type()
    {
        if(this->value == NULL)
            return;

        this->_type = MsgType::UNKNOWN;
        if(this->value->type == json_object)
        {
            for(int x=0;x<this->value->u.object.length; ++x)
            {
                if(std::string(this->value->u.object.values[x].name) == "method")
                {
                    std::string method(this->value->u.object.values[x].value->u.string.ptr);

                    this->_type = MsgType::match_type(method);
                    return;
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
#define __STRATUM_UNITTEST__
#ifdef __STRATUM_UNITTEST__

int main(int argc, char **argv)
{
    stratum::Worker worker("cn.stratum.slushpool.com", 3333,
                            "trashe725.cppminer", "password");
    if(!worker)
    {
        std::cout << "Failed to connect to slushpool..." << std::endl;
    }

    return 0;
}

#endif


