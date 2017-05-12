/*!
  \author
    Wyatt Lavigueure
  \date
    5/11/2017
  \brief
    Blow fish encrypton algorithm
*/
#ifndef BLOW_H
#define BLOW_H

/*!
  \brief
    Class for doing blowfish encryptions
*/
class BlowFish
{
public:
	/*!
	  \brief
	    initalizes the keys and s_box
	  \param key
	    The key for the blowfish
	  \param size
	    The size of the key
	*/
	BlowFish(unsigned *key, unsigned size);
	/*!
	  \brief
	    encrypts two unsigneds using blowfish encryption
	  \param lhs
	    the left unsigned to encrypt
	  \param rhs
	    the right unsigned to encrypt
	*/
	void encrypt(unsigned &lhs, unsigned &rhs);

	/*!
	  \brief
	    decrypts two unsigneds using blowfish
	  \param lhs
	    the left unsigned to decrypt
	  \param rhs
	    the right unsigned to decrypt
	*/
	void decrypt(unsigned &lhs, unsigned &rhs);

	/*!
	  \brief
	    Initalizes the keys for blowfish
	*/
	void init();
private:
		unsigned keys[18];
	  unsigned s_box[4][256];
		unsigned round_function(unsigned x);
};

#endif