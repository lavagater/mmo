#include "protocol.h"
#include "logger.h"
#include <fstream>
#include <iostream>
#include <string>

ProtocolLoader::ProtocolLoader(std::string protocol_directory)
{
    protocol_files.push_back(protocol_directory + "database_system_protocol.txt");
    protocol_files.push_back(protocol_directory + "load_balancer_system_protocol.txt");
    protocol_files.push_back(protocol_directory + "zone_system_protocol.txt");
    protocol_files.push_back(protocol_directory + "client_system_protocol.txt");
    protocol_files.push_back(protocol_directory + "database_game_protocol.txt");
    protocol_files.push_back(protocol_directory + "load_balancer_game_protocol.txt");
    protocol_files.push_back(protocol_directory + "zone_game_protocol.txt");
    protocol_files.push_back(protocol_directory + "client_game_protocol.txt");
}

void ProtocolLoader::LoadProtocol()
{
    MessageType max = 0;
    message_types.clear();
    for (unsigned i = 0;i < protocol_files.size(); ++i)
    {
        std::ifstream file(protocol_files[i]);
        if (!file.good())
        {
            LOGW("protocol file " << protocol_files[i] << " failed to open");
        }
        while(file.good() && !file.eof())
        {
            std::string name;
            file >> name;
            if (name == "")
            {
                break;
            }
            message_types[name] = max;
            LOG("Message type " << name << " = " << max);
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

std::string ProtocolLoader::LookUp(MessageType type)
{
    for (auto it = message_types.begin(); it != message_types.end(); ++it)
    {
        if (it->second == type)
        {
            return it->first;
        }
    }
    return "";
}