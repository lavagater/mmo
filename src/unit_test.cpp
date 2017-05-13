/*****************************************************************************/
/*!
\author Wyatt Lavigueure
\date   5/2/2017
\brief  
  Test cases

*/
/*****************************************************************************/
#include <iostream>
#include <sstream>
#include <string.h>
#include <time.h>

#include "wyatt_sock.h"
#include "meta.h"
#include "config.h"
#include "channel.h"
#include "frame_rate.h"
#include "blowfish.h"
#include "network_stack.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define PRINT_ERROR(x) std::cout << "line " << __LINE__ << " function " << __FUNCTION__ << std::endl;

bool TestInferType();
bool TestStringToValue();
bool TestConfig();
bool TestHashFunction();
bool TestFrameRate();
bool TestBlowFish();
bool TestNetworkLayer();

bool (*tests[])() = { 
    TestInferType, TestStringToValue, TestConfig, TestHashFunction, TestFrameRate, TestBlowFish,
    TestNetworkLayer
}; 

int main()
{
	srand(time(0));
	int num_failed = 0;
	for (unsigned i = 0; i < sizeof(tests) / sizeof(tests[0]); ++i)
	{
		if (!tests[i]())
		{
			num_failed += 1;
		}
	}
	if (num_failed)
	{
		getchar();
	}
  return 1337;
}

//test cases
bool TestInferType()
{
	//check int
	for (int i = 0; i < 999999; i += 13)
	{
		std::stringstream s;
		s << i;
		int type = InferType(s.str().c_str());
		if (type != type_int)
		{
			PRINT_ERROR();
			std::cout << "str = " << s.str() << std::endl;
			std::cout << "type is " << type << std::endl;
			return false;
		}
	}
	//check negative
	for (long i = -9999999999; i < -9999998999; i += 753)
	{
		std::stringstream s;
		s << i;
		int type = InferType(s.str().c_str());
		if (type != type_int && type != type_long)
		{
			PRINT_ERROR();
			std::cout << "str = " << s.str() << std::endl;
			std::cout << "type is " << type << std::endl;
			return false;
		}
	}
	//check bool
	int type = InferType("TRUE");
	if (type != type_bool)
	{
		PRINT_ERROR();
		std::cout << "type is " << type << std::endl;
		return false;
	}
	type = InferType("false");
	if (type != type_bool)
	{
		PRINT_ERROR();
		std::cout << "type is " << type << std::endl;
		return false;
	}
	type = InferType("FALSE");
	if (type != type_bool)
	{
		PRINT_ERROR();
		std::cout << "type is " << type << std::endl;
		return false;
	}
	type = InferType("true");
	if (type != type_bool)
	{
		PRINT_ERROR();
		std::cout << "type is " << type << std::endl;
		return false;
	}
	//check floating point
	for (int j = 0; j < 100; ++j)
	{
		for (int i = -50; i < 50; ++i)
		{
			if (i == 0)
				continue;
		  double test = double(j) / double(i);
		  //make sure its not an int
		  if (test == int(test))
		  {
		  	continue;
		  }
			std::stringstream s;
			s << test;
			int type = InferType(s.str().c_str());
			if (type != type_float && type != type_double)
			{
				PRINT_ERROR();
				std::cout << "str = " << s.str() << std::endl;
				std::cout << "type is " << type << std::endl;
				return false;
			}
		}
	}
	//check char
	{//start a scope so i can copy pasta
		char temp[] = "\'a\'";
		temp[1] = 1;
		for (int i = 0; i < 127; ++i)
		{
			type = InferType(temp);
			if (type != type_char)
			{
				PRINT_ERROR();
				std::cout << "str is "<< temp << std::endl;
				std::cout << "type is " << type << std::endl;
			}
			temp[1] += 1;
		}
	}//end scope
	//check string
	{//start a scope so i can copy pasta
		char temp[] = "\"baby sitter\"";
		for (int i = 0; i < 1337; ++i)
		{
			type = InferType(temp);
			if (type != type_string)
			{
				PRINT_ERROR();
				std::cout << "str is "<< temp << std::endl;
				std::cout << "type is " << type << std::endl;
			}
			temp[i%11+1] += i % 77;
			if (temp[i%11+1] == 0)
			{
				temp[i%11+1] = 'a';
			}

		}
	}//endscope

  //test nothing
  type = InferType("");
	if (type != type_null)
	{
		PRINT_ERROR();
		std::cout << "type is " << type << std::endl;
		return false;
	}

	//test hex
	type = InferType("0x77");
	if (type != type_hex)
	{
		PRINT_ERROR();
		std::cout << "type is " << type << std::endl;
		return false;
	}
	//test hex
	type = InferType("ABCDEF");
	if (type != type_hex)
	{
		PRINT_ERROR();
		std::cout << "type is " << type << std::endl;
		return false;
	}
	return true;
}

bool TestStringToValue()
{
	//test int
	if ((int)StringToValue("14") != 14)
	{
		PRINT_ERROR();
		std::cout << "14 gives " << (int)StringToValue("14") << std::endl; 
		return false;
	}
	//test negative
	if ((int)StringToValue("-2") != -2)
	{
		PRINT_ERROR();
		std::cout << "-2 gives " << (int)StringToValue("-2") << std::endl; 
		return false;
	}
	//test big number
	if ((long)StringToValue("1234567890") != 1234567890)
	{
		PRINT_ERROR();
		std::cout << "1234567890 gives " << (long)StringToValue("1234567890") << std::endl; 
		return false;
	}
	//test floating points
	if ((double)StringToValue("1234.567890") != 1234.567890)
	{
		PRINT_ERROR();
		std::cout << "1234.567890 gives " << (double)StringToValue("1234.567890") << std::endl; 
		return false;
	}
	//test empty string
	if ((int)StringToValue("") != 0)
	{
		PRINT_ERROR();
		std::cout << "\"\" = " << (int)StringToValue("") << std::endl; 
		return false;
	}
	//test boolean
	if ((bool)StringToValue("true") != true)
	{
		PRINT_ERROR();
		std::cout << "true gives " << (bool)StringToValue("true") << std::endl; 
		return false;
	}
	//test boolean
	if ((bool)StringToValue("TRUE") != true)
	{
		PRINT_ERROR();
		std::cout << "TRUE gives " << (bool)StringToValue("TRUE") << std::endl; 
		return false;
	}
	//test boolean
	if ((bool)StringToValue("false") != false)
	{
		PRINT_ERROR();
		std::cout << "false gives " << (bool)StringToValue("false") << std::endl; 
		return false;
	}
	//test boolean
	if ((bool)StringToValue("FALSE") != false)
	{
		PRINT_ERROR();
		std::cout << "FALSE gives " << (bool)StringToValue("FALSE") << std::endl; 
		return false;
	}
	//test char
	if ((char)StringToValue("'f'") != 'f')
	{
		PRINT_ERROR();
		std::cout << "'f' gives " << (char)StringToValue("'f'") << std::endl; 
		return false;
	}
	if ((std::string)StringToValue("\"huehuehue\"") != "huehuehue")
	{
		PRINT_ERROR();
		std::cout << "huehuehue !=" << (std::string)StringToValue("\"huehuehue\"") << std::endl; 
		return false;
	}

	//check hex
	if ((int)StringToValue("0x5") != 5)
	{
		PRINT_ERROR();
		std::cout << "0x5 is " << (int)StringToValue("0x5") << std::endl;
		return false;
	}
	//check hex more
	if ((int)StringToValue("0x45") != 69)
	{
		PRINT_ERROR();
		std::cout << "0x45 is " << (int)StringToValue("0x45") << std::endl;
		return false;
	}
	//check impropper casting
	if ((int)StringToValue("3.14159") != 3)
	{
		PRINT_ERROR();
		std::cout << "3.14159 is casted as int to " << (int)StringToValue("3.14159") << std::endl;
		return false;
	}
	return true;
}

bool TestConfig()
{
	Config config;
	config.Init("test_conf.txt");
	if ((float)config.properties["item1"] != 34.5)
	{
		PRINT_ERROR();
		return false;
	}
	if ((float)config.properties["name with spaces"] != 34.5)
	{
		PRINT_ERROR();
		return false;
	}
	if ((bool)config.properties["test_bool2"] != true)
	{
		PRINT_ERROR();
		return false;
	}
	if ((int)config.properties["test_negative"] != -4)
	{
		PRINT_ERROR();
		return false;
	}
	if ((std::string)config.properties[""] != "Empty name")
	{
		PRINT_ERROR();
		return false;
	}
	if ((std::string)config.properties["test_string"] != "Hello World")
	{
		PRINT_ERROR();
		return false;
	}
	if ((std::string)config.properties["test_string_of_numbers"] != "123")
	{
		PRINT_ERROR();
		return false;
	}
	if ((unsigned)config.properties["test_hex"] != 0x7F)
	{
		PRINT_ERROR();
		return false;
	}
	if ((long)config.properties["test_long"] != 123456789101112)
	{
		PRINT_ERROR();
		return false;
	}
	if ((char)config.properties["test_char"] != 'a')
	{
		PRINT_ERROR();
		return false;
	}
	if ((int)config.properties["test_empty_value"] != 0)
	{
		PRINT_ERROR();
		return false;
	}
	return true;
}

bool TestHashFunction()
{
	SockAddrHash hashfunc;
	//testing hash to same value
	sockaddr_in tester;
	CreateAddress("127.0.0.1", 7327, &tester);
	if (hashfunc(tester) != hashfunc(tester))
	{
		PRINT_ERROR();
		return false;
	}
	sockaddr_in tester2;
	CreateAddress("127.0.0.1", 7327, &tester2);
	if (hashfunc(tester) != hashfunc(tester2))
	{
		PRINT_ERROR();
		return false;
	}
	//test different ports only
	for (unsigned i = 1; i < 1000; ++i)
	{
		CreateAddress("127.0.0.1", 7327+i, &tester2);
		if (hashfunc(tester) == hashfunc(tester2))
		{
			PRINT_ERROR();
			return false;
		}
	}
	//test different ip only
	for (unsigned i = 2; i < 1000; ++i)
	{
		std::string new_ip = (std::string("127.0.0.") + std::to_string(i));
		CreateAddress(new_ip.c_str(), 7327, &tester2);
		if (hashfunc(tester) == hashfunc(tester2))
		{
			PRINT_ERROR();
			return false;
		}
	}
	return true;
}

bool TestFrameRate()
{
	FrameRate ft;
  double jj = 0;
	ft.GetTime();
	for (unsigned i = 0; i < 10000000; ++i)
	{
		jj = jj + i * i / jj - 16 + (1 - jj) * i;
	}
	double first = ft.GetTime();
	jj = 0;
	ft.GetTime();
	for (unsigned i = 0; i < 1000000; ++i)
	{
		jj = jj + i * i / jj - 16 + (1 - jj) * i;
	}
	double second = ft.GetTime();
  //check to make sure that the second for loop ran 10 times faster +-1 percent
	if (abs(second / first - 0.1) > 0.01)
	{
		PRINT_ERROR();
		return false;
	}
	return true;
}

bool TestBlowFish()
{
	//make a key
	unsigned key[18];
	for (unsigned i = 0; i < 18; ++i)
	{
		key[i] = rand();
	}
	BlowFish bf(key, 18);
	for (int i = 0; i < 1337; ++i)
	{
		unsigned lhs = rand();
		unsigned rhs = rand();
		unsigned prev_lhs = lhs;
		unsigned prev_rhs = rhs;
		bf.encrypt(lhs, rhs);
		//make sure it did something
		if (rhs == prev_lhs || lhs == prev_rhs)
		{
			PRINT_ERROR();
			std::cout << "blowfish did not encrypt..." << std::endl;
			return false;
		}
		bf.decrypt(lhs, rhs);
		if (lhs != prev_lhs || rhs!= prev_rhs)
		{
			PRINT_ERROR();
			std::cout << "blowfish did not decrypt..." << std::endl;
			return false;
		}
	}
	return true;
}

//this baby on send adds 1 to every byte, just to test the network stack functionality
class TestLayer1 : public NetworkLayer
{
public:
	~TestLayer1()
	{
		//nothing to do..
	}
	TestLayer1()
	{
		count_sent = 0;
		count_recv = 0;
	}
	int Send(char *buffer, int bytes, __attribute__((unused))sockaddr_in *dest)
	{
		memcpy(save_buf[count_sent], buffer, bytes);
		for (int i = 0; i < bytes; ++i)
		{
			buffer[i] += 1;
		}
		count_sent += 1;
		return bytes;
	}
	int Receive(char *buffer, int bytes, __attribute__((unused))sockaddr_in *location)
	{
		for (int i = 0; i < bytes; ++i)
		{
			buffer[i] -= 1;
			if (buffer[i] != save_buf[count_recv][i])
			{
				PRINT_ERROR();
			}
		}
		count_recv += 1;
		return bytes;
	}
	void Update(__attribute__((unused))float dt)
	{

	}
private:
	//save the originals here
	char save_buf[5][MAXSOCKETSIZE];
	//these never get reset so only 5 sends can be tested
	int count_sent;
	int count_recv;
};

//this baby on send adds a header to the packet, just to test the network stack functionality
class TestLayer2 : public NetworkLayer
{
public:
	~TestLayer2()
	{
		//nothing to do.., but did test that this got called properly trust meee
	}
	TestLayer2()
	{
		count_sent = 0;
		count_recv = 0;
	}
	int Send(char *buffer, int bytes, __attribute__((unused))sockaddr_in *dest)
	{
		//shift the buffer over by 8 bytes
		for (int i = bytes - 1; i >=0; --i)
		{
			buffer[i+8] = buffer[i];
		}
		//fill the header with random stuff
		for (int i = 0; i < 8; ++i)
		{
			buffer[i] = rand();
		}
		//save buffer with the header attached
		memcpy(save_buf[count_sent], buffer, bytes+8);
		count_sent += 1;
		return bytes+8;
	}
	int Receive(char *buffer, int bytes, __attribute__((unused))sockaddr_in *location)
	{
		//check that the buffer still has the header and its the same as we sent it
		for (int i = 0; i < bytes; ++i)
		{
			if (buffer[i] != save_buf[count_recv][i])
			{
				PRINT_ERROR();
			}
		}
		//remove the header by shifting the buffer back over by 8 bytes
		for (int i = 0; i < bytes; ++i)
		{
			buffer[i] = buffer[i+8];
		}
		count_recv += 1;
		return bytes-8;
	}
	void Update(__attribute__((unused))float dt)
	{

	}
private:
	//save the originals here
	char save_buf[5][MAXSOCKETSIZE];
	//these never get reset so only 5 sends can be tested
	int count_sent;
	int count_recv;
};

bool TestNetworkLayer()
{
	//set up a connection with ourself
	sockaddr_in sender;
	sockaddr_in reciever;
	Init();
	CreateAddress("127.0.0.1", 6900, &sender);
	CreateAddress("127.0.0.1", 6901, &reciever);
	SOCKET send_sock = CreateSocket(IPPROTO_UDP);
	SOCKET recieve_sock = CreateSocket(IPPROTO_UDP);
	Bind(send_sock, &sender);
	Bind(recieve_sock, &reciever);
	SetNonBlocking(send_sock);
	SetNonBlocking(recieve_sock);

	//now create our network stack
	NetworkStack net;
	// sandwhich the layers on each other
	net.layers.push_back(new TestLayer1());
	net.layers.push_back(new TestLayer2());
	net.layers.push_back(new TestLayer1());
	net.layers.push_back(new TestLayer2());
	//send some data
	std::string data = "this is data";
	net.Send(send_sock, data.c_str(), data.size()+1, &reciever);
	//recieve the data
	char buf[MAXSOCKETSIZE] = "Not the DATA i sent!";
	sockaddr_in location;
	net.Receive(recieve_sock, buf, MAXSOCKETSIZE, &location);
	if (data != buf)
	{
		PRINT_ERROR();
		return false;
	}
	//send a couple times then recieve a couple of times
	std::string data1 = "data one";
	net.Send(send_sock, data1.c_str(), data1.size()+1, &reciever);
	//large amount of data
	std::string data2;
	for (int i = 0; i < 1; ++i)
	{
		data2 += 'a';
	}
	net.Send(send_sock, data2.c_str(), data2.size()+1, &reciever);
	std::string data3 = "data three";
	net.Send(send_sock, data3.c_str(), data3.size()+1, &reciever);
	std::string data4 = "last";
	net.Send(send_sock, data4.c_str(), data4.size()+1, &reciever);
	//recieve the sent data
	net.Receive(recieve_sock, buf, MAXSOCKETSIZE, &location);
	if (data1 != buf)
	{
		PRINT_ERROR();
		return false;
	}
	net.Receive(recieve_sock, buf, MAXSOCKETSIZE, &location);
	if (data2 != buf)
	{
		PRINT_ERROR();
		return false;
	}
	net.Receive(recieve_sock, buf, MAXSOCKETSIZE, &location);
	if (data3 != buf)
	{
		PRINT_ERROR();
		return false;
	}
	net.Receive(recieve_sock, buf, MAXSOCKETSIZE, &location);
	if (data4 != buf)
	{
		PRINT_ERROR();
		return false;
	}
	Close(send_sock, true);
	Close(recieve_sock, true);
	Deinit();
	return true;
}

#endif //DOXYGEN_SHOULD_SKIP_THIS