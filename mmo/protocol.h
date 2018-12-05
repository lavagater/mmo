/*!
  \author
    Wyatt Lavigueure
  \date
    9/8/2018
  \brief
    This file defines a protocol for networking in the game engine. This protocol is used for Database and game logic or any other 
    uses. This protocol is not used for the underlying network stack, i.e. The network stack has its own protocol witch is
    decoupled from this protocol. There should be a function for each enum that will put info into the char* to be sent, these function
    go in a seperate file.
*/
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <unordered_map>
#include <vector>

typedef unsigned short MessageType;

class ProtocolLoader
{
public:
  /**
   * @brief Constructor adds the normal/system protocol file names to the protocol_files
   */
  ProtocolLoader();
  /**
   * @brief Reads all the protocol files and adds them to the messages_types
   */
  void LoadProtocol();
  /**
   * @brief gets the message type for the given string
   */
  MessageType LookUp(std::string);
  /**
   * @brief gets the message name from the id, for debugging mostly
   */
  std::string LookUp(MessageType);
  /**
   * @brief name of the files that contain protocol types the files are loaded in order of the vector
   */
  std::vector<std::string> protocol_files;
  /**
   * @brief maps name to message type
   */
  std::unordered_map<std::string, MessageType> message_types;
};

#endif