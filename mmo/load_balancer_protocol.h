/*****************************************************************************/
/*!
\author Wyatt Lavigueure
\date   9/8/2018
\brief  
  Protocol specific to the load balancer. not in the load balancer folder because
  each app has to comunicate to the others using these protocols so they all need 
  access to them.

*/
/*****************************************************************************/

#ifndef LOAD_BALANCER_PROTOCOL_H
#define LOAD_BALANCER_PROTOCOL_H
#include "protocol.h"
#include "asymetric_encryption.h"

/**
 * @brief makes a message saying to the load balancer that 
 *        the comunication should be encrypted and sends
 *        a key to use for the encryption
 * @param buffer memory to put the message
 * @param key the key to use for future encryption
 * @param size the number of bytes the key is
 * @param encryptor The asymetric encryption to use to encrypt this message
 */
int CreateEncryptionMessage(ProtocolLoader &protocol, char *buffer, char *key, short size, AsymetricEncryption &encryptor);

/**
 * @brief Extracts the key and key size from an Encrypt message
 * @param data the Encrypt message
 * @param size the size of data in bytes
 * @param key Memory to store the key into
 * @param encryptor The asymetric encryption to use to decrypt the message
 * @return returns the length of the key in bytes
 */
short ReadEncryptionMessage(char *data, unsigned size, char *key, AsymetricEncryption &encryptor);

#endif