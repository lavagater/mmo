/*!
  \author
    Wyatt Lavigueure
  \date
    8/18/2018
  \brief
    Does an asyemetric encryption on a string
*/

#ifndef ASYMETRIC_ENCRYPTION_H
#define ASYMETRIC_ENCRYPTION_H

class AsymetricEncryption
{
public:
    virtual ~AsymetricEncryption();
    virtual unsigned encrypt(char *data, unsigned size);
    virtual unsigned decrypt(char *data, unsigned size);
};

#endif