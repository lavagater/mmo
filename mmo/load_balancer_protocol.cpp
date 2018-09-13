#include "load_balancer_protocol.h"
#include <string.h>

int CreateEncryptionMessage(char *buffer, char *key, short size, AsymetricEncryption &encryptor)
{
    Protocol proto = Protocol::EncryptionKey;
    memcpy(buffer, &proto, message_type_size);
    buffer += message_type_size;
    memcpy(buffer, &size, sizeof(size));
    memcpy(buffer+sizeof(size), key, size);
    unsigned length = encryptor.encrypt(buffer, sizeof(size)+size);
    return length +message_type_size;
}

short ReadEncryptionMessage(char *data, unsigned size, char *key, AsymetricEncryption &encryptor)
{
    short ret;
    if (size < message_type_size)
    {
        return -1;
    }
    data += message_type_size;
    unsigned length = encryptor.decrypt(data, size - message_type_size);
    if (length < sizeof(ret))
    {
        return -1;
    }
    memcpy(&ret, data, sizeof(ret));
    if (length < sizeof(ret) + ret)
    {
        return -1;
    }
    memcpy(key, data+sizeof(ret), ret);
    return ret;
}