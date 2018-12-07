#include "load_balancer_protocol.h"
#include <string.h>

int CreateEncryptionMessage(ProtocolLoader &protocol,char *buffer, char *key, short size, AsymetricEncryption &encryptor)
{
    MessageType type = protocol.LookUp("EncryptionKey");
    memcpy(buffer, &type, sizeof(MessageType));
    buffer += sizeof(MessageType);
    memcpy(buffer, &size, sizeof(size));
    memcpy(buffer+sizeof(size), key, size*sizeof(unsigned));
    unsigned length = encryptor.encrypt(buffer, sizeof(size)+size);
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
    if (length < sizeof(ret) + ret)
    {
        return -1;
    }
    //because ret is num unsigned
    memcpy(key, data+sizeof(ret), ret*sizeof(unsigned));
    return ret;
}