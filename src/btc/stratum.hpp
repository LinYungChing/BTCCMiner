#pragma once

#include <iostream>
#include <sstream>
#include <string>

#include <deque>
#include <map>

extern "C"{
#include "json-parser/json.h"
}

#include "json-parser/json_utils.hpp"
#include "tcp/tcp_client.hpp"

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

        std::string subscription;

        bool connect_status;

        std::string extranonce;
        int extranonce2_size;
        unsigned int difficulty;

        std::deque<Message> msgqueue;
        int id_counter;

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

        bool empty();
        Message popMessage();
        bool checkMessageByType(MsgType msgtype);
        Message getMessageByType(MsgType msgtype);

        bool updateMessageQueue();
        MsgType getFirstMessageType();

        // client to server function
        bool authorize();
        bool subscribe();
        bool get_transactions(std::string &job_id);
        bool submit(const std::string job_id, const std::string extranonce2,
                    const std::string ntime, const std::string nonce);

        //get/set
        std::string getUrl();
        int getPort();
        std::string getUsername();

        std::string getExtranonce();
        int getExtranonce2Size();
        unsigned int getDifficulty();

        void setExtranonce(const std::string &enonce);
        void setExtranonce2Size(const int enonce2_size);
        void setDifficulty(const unsigned int diff);

        // Overload
        explicit operator bool();

    private:

        int _get_counter(bool increase=true);
        Message _wait_for_specific_id(int id);

        bool _reconnect();
        bool _safe_send(std::string &msg);
        bool _safe_recv(std::string &msg, size_t size = 4096);
        bool _safe_getline(std::string &msg, char delim='\n');
    };

    ////////////// Message ///////////////////

    class Message
    {
    private:

        MsgType _type;

        std::string json;
        json_value *raw_value;
        std::map< std::string, json_value* > field;

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

        json_value* operator[](std::string fieldname);
        Message &operator=(const Message &msg);

        /// static ///
        friend class MsgParser;
        static Message None();

    private:
        void _parse_field_and_type();
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
        static std::string get_transactions(const std::string &job_id,
                                           int id = 5);
        static Message parse(const std::string &json_msg);
    private:
    };

    

}
