#include <iostream>
#include <string>
#include <map>
#include <sstream>

extern "C"{
#include "json-parser/json.h"
}

#include "tcp_client.hpp"

namespace stratum
{

    class Worker;
    class MsgParser;
    class Message;

    struct MsgType
    {
        enum Type
        {
            NONE,
            UNKNOWN,
            // Client to Server
            AUTHORIZE,
            CAPABILITIES,
            GET_TRANSACTIONS,
            SUBMIT,
            SUBSCRIBE,
            SUGGEST_DIFFICULTY,
            SUGGEST_TARGET,

            // Server to Client
            GET_VERSION,
            RECONNECT,
            SHOW_MESSAGE,
            NOTIFY,
            SET_DIFFICULTY,
            SET_EXTRANONCE,
            SET_GOAL
        };


        Type t_;
        MsgType(Type t);
        operator Type() const;

        static MsgType match_type(const std::string &method);
        static std::string match_type(const MsgType &msg);

    private:
        static std::map<std::string, MsgType> s2t_mapping;
        static std::map<MsgType, std::string> t2s_mapping;

    };
    

    /////////////// Worker ///////////////////

    class Worker
    {
    private:
        tcp::TCPClient client;
    
        std::string username;
        std::string password;

        bool connect_status;

        std::string extranonce1;
        int extranonce2_size;

    public:
        // Constructor
        Worker();
        Worker(const std::string &url, const int &port,
               const std::string &username, const std::string &password);

        // Destructor
        ~Worker();

        // Utils
        bool setup(const std::string &url, const int &port,
                   const std::string &username, const std::string &password);
        bool status();
        void close();

        //get/set
        std::string getUrl();
        int getPort();
        std::string getUsername();

        // Overload
        explicit operator bool();

    private:

        bool _subscribe();
        bool _authorize();
    };

    ////////////// Message ///////////////////

    class Message
    {
    private:

        MsgType _type;

        std::string json;
        json_value *value;
    public:
        Message();
        Message(const std::string &json);
        Message(const Message &msg);
        ~Message();
        
        MsgType getType();
        std::string &getJson();
        json_value *getObject();

        bool parse();
        bool parse(const std::string &json);

        void clear();

        friend class MsgParser;

    private:
        void _parse_type();
    };


    ///////////// Msg Parser //////////////////

    class MsgParser
    {
    private:
    public:
        static std::string subscribe(int id = 1);
        static std::string authorize(const std::string &username, 
                                     const std::string &password, 
                                     int id = 2);
        static std::string submit(const std::string &username,
                                  const std::string &job_id,
                                  const std::string &extranonce2,
                                  const std::string &ntime,
                                  const std::string &nonce,
                                  int id = 4);
        static Message parse(const std::string &json_msg);
    private:
    };

    

}
