/*!
  \author
    Wyatt Lavigueure
  \date
    8/6/2017
  \brief
    Functions for the database messages
*/
#ifndef DATABASE_PROTOCOL_H
#define DATABASE_PROTOCOL_H
#include "protocol.h"

/*!
  \breif
    Creates a DatabaseGet message and puts it in buffer. buffer should be large enough
    to fit 2 unsigneds and one char
  \param buffer
    The buffer to put the message shoud be preallocated
  \param id
    The id of the object in the database to get
  \param row
    The row/property in the object to get
  \return
    returns the number of byts written into buffer
*/
int CreateGetMessage(char *buffer, unsigned id, unsigned row);
/*!
  \breif
    Creates a DatabaseSet message and puts it in buffer. buffer should be large enough
    to fit 2 unsigneds one char and the raw data
  \param buffer
    The buffer to put the message shoud be preallocated
  \param id
    The id of the object in the database to set
  \param row
    The row/property in the object to set
  \param data
    The raw data to set the property too
  \param size
    The size in bytes of data
  \return
    returns the number of byts written into buffer
*/
int CreateSetMessage(char *buffer, unsigned id, unsigned row, void *data, int size);
/*!
  \breif
    Creates a DatabaseCreate message and puts it in buffer. buffer should be large enough
    to fit 2 unsigneds one char and the raw data
  \param buffer
    The buffer to put the message shoud be preallocated
  \param nonce
    A unique number to identify this create call, the database app will put this number in the response so that 
    create responses can be distinguished
  \return
    returns the number of byts written into buffer
*/
int CreateCreateMessage(char *buffer, unsigned nonce);
/*!
  \breif
    Creates a DatabaseFind message and puts it in buffer. buffer should be large enough
    to fit an unsigned, a char and the raw data
  \param buffer
    The buffer to put the message shoud be preallocated
  \param row
    The row/property in the object to find
  \param data
    The raw data of the property too match
  \param size
    The size in bytes of data
  \return
    returns the number of byts written into buffer
*/
int CreateFindMessage(char *buffer, unsigned row, void *data, int size);
/*!
  \breif
    Creates a DatabaseDelete message and puts it in buffer. buffer should be large enough
    to fit an unsigned, and a char 
  \param buffer
    The buffer to put the message shoud be preallocated
  \param id
    The id of the object to delete
  \return
    returns the number of byts written into buffer
*/
int CreateDeleteMessage(char *buffer, unsigned id);
#endif