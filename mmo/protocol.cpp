#include "protocol.h"
#include "logger.h"
#include <fstream>
#include <iostream>

ProtocolLoader::ProtocolLoader()
{
    protocol_files.push_back("resources/database_system_protocol.enum");
    protocol_files.push_back("resources/load_balancer_system_protocol.enum");
    protocol_files.push_back("resources/zone_system_protocol.enum");
    protocol_files.push_back("resources/client_system_protocol.enum");
    protocol_files.push_back("resources/database_game_protocol.enum");
    protocol_files.push_back("resources/load_balancer_game_protocol.enum");
    protocol_files.push_back("resources/zone_game_protocol.enum");
    protocol_files.push_back("resources/client_game_protocol.enum");
}

void ProtocolLoader::LoadProtocol()
{
    MessageType max = 0;
    message_types.clear();
    for (unsigned i = 0;i < protocol_files.size(); ++i)
    {
        std::ifstream file(protocol_files[i]);
        while(file.good() && !file.eof())
        {
            std::string name;
            file >> name;
            message_types[name] = max;
            max += 1;
            if (max == 0)
            {
              //overflow
              LOGW("To many protocol types, need to increase size of MessageType");
              //make this print for every file after it
              max -= 1;  
            }
        }
    }
}

MessageType ProtocolLoader::LookUp(std::string message)
{
    auto it = message_types.find(message);
    if (it != message_types.end())
    {
        return it->second;
    }
    else
    {
        LOGW("LookUp found no message typee for message " << message);
        return -1;
    }
}