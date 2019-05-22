#include "load_balancer_protocol.h"
#include "logger.h"
#include <string.h>

int CreateEncryptionMessage(ProtocolLoader &protocol,char *buffer, char *key, short size, AsymetricEncryption &encryptor)
{
    MessageType type = protocol.LookUp("EncryptionKey");
    memcpy(buffer, &type, sizeof(MessageType));
    buffer += sizeof(MessageType);
    memcpy(buffer, &size, sizeof(size));
    memcpy(buffer+sizeof(size), key, size*sizeof(unsigned));
    unsigned length = encryptor.encrypt(buffer, sizeof(size)+size*sizeof(unsigned));
    return length + sizeof(MessageType);
}

short ReadEncryptionMessage(char *data, unsigned size, char *key, AsymetricEncryption &encryptor)
{
    short ret;
    if (size < sizeof(MessageType))
    {
        return -1;
    }
    data += sizeof(MessageType);
    unsigned length = encryptor.decrypt(data, size - sizeof(MessageType));
    if (length < sizeof(ret))
    {
        return -1;
    }
    memcpy(&ret, data, sizeof(ret));
    if (length < sizeof(ret) + ret*sizeof(unsigned))
    {
        return -1;
    }
    //because ret is num unsigned
    memcpy(key, data+sizeof(ret), ret*sizeof(unsigned));
    return ret;
}

char *ParseForwardMessage(char *data, unsigned &size, int &dest, unsigned &id)
{
    //           type                dest            id           any data in message
    if (size < sizeof(MessageType) + sizeof(char) + sizeof(unsigned) + 1)
    {
        return 0;
    }
    dest = *reinterpret_cast<unsigned char*>(data+sizeof(MessageType));
    id = *reinterpret_cast<unsigned*>(data+sizeof(MessageType) + sizeof(char));
    size -= sizeof(MessageType) + sizeof(char) + sizeof(unsigned);
    return data + sizeof(MessageType) + sizeof(char) + sizeof(unsigned);
}


void CreateForwardMessage(ProtocolLoader &protocol, char *data, unsigned &size, unsigned id, char *output)
{
    //shift over the message
    for (unsigned i = 1; i <= size; ++i)
    {
        output[size-i + sizeof(MessageType) + sizeof(unsigned)] = data[size-i];
    }
    *reinterpret_cast<MessageType*>(output) = protocol.LookUp("Forward");
    output += sizeof(MessageType);
    *reinterpret_cast<unsigned*>(output) = id;
    LOGW("Creating forward message to id " << id);
    size += sizeof(MessageType) + sizeof(unsigned);
}