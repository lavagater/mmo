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
#include <unistd.h>

#include "wyatt_sock.h"
#include "meta.h"
#include "config.h"
#include "channel.h"
#include "frame_rate.h"
#include "blowfish.h"
#include "network_stack.h"
#include "bit_array.h"
#include "reliability.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define PRINT_ERROR(x) std::cout << "line " << __LINE__ << " function " << __FUNCTION__ << std::endl;

bool TestInferType();
bool TestStringToValue();
bool TestConfig();
bool TestHashFunction();
bool TestFrameRate();
bool TestBlowFish();
bool TestNetworkLayer();
bool TestBitArray();
bool TestReliability();

bool (*tests[])() = { 
    TestInferType, TestStringToValue, TestConfig, TestHashFunction, TestFrameRate, TestBlowFish,
    TestNetworkLayer, TestBitArray, TestReliability
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
	int Send(char *buffer, int bytes, __attribute__((unused))const sockaddr_in *dest, __attribute__((unused))BitArray<HEADERSIZE> &flags)
	{
		for (int i = 0; i < bytes; ++i)
		{
			buffer[i] += 1;
		}
		return bytes;
	}
	int Receive(char *buffer, int bytes, __attribute__((unused))sockaddr_in *location, __attribute__((unused))BitArray<HEADERSIZE> &flags)
	{
		for (int i = 0; i < bytes; ++i)
		{
			buffer[i] -= 1;
		}
		return bytes;
	}
	void Update(__attribute__((unused))double dt)
	{

	}
};

//this baby on send adds a header to the packet, just to test the network stack functionality
class TestLayer2 : public NetworkLayer
{
public:
	~TestLayer2()
	{
		//nothing to do.., but did test that this got called properly trust meee
	}
	int Send(char *buffer, int bytes, __attribute__((unused))const sockaddr_in *dest, __attribute__((unused))BitArray<HEADERSIZE> &flags)
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
		return bytes+8;
	}
	int Receive(char *buffer, int bytes, __attribute__((unused))sockaddr_in *location, __attribute__((unused))BitArray<HEADERSIZE> &flags)
	{
		//remove the header by shifting the buffer back over by 8 bytes
		for (int i = 0; i < bytes; ++i)
		{
			buffer[i] = buffer[i+8];
		}
		return bytes-8;
	}
	void Update(__attribute__((unused))double dt)
	{

	}
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
	NetworkStack net_send(send_sock);
	NetworkStack net_recv(recieve_sock);
	// sandwhich the layers on each other
	net_send.AddLayer(new TestLayer1());
	net_send.AddLayer(new TestLayer2());
	net_send.AddLayer(new TestLayer1());
	net_send.AddLayer(new TestLayer2());
	//create another for recieving
	net_recv.AddLayer(new TestLayer1());
	net_recv.AddLayer(new TestLayer2());
	net_recv.AddLayer(new TestLayer1());
	net_recv.AddLayer(new TestLayer2());
	//send some data
	std::string data = "this is data";
	BitArray<HEADERSIZE> flags;
	net_send.Send(data.c_str(), data.size()+1, &reciever, flags);
	//recieve the data
	char buf[MAXPACKETSIZE] = "Not the DATA i sent!";
	sockaddr_in location;
	net_recv.Receive(buf, MAXPACKETSIZE, &location);
	if (data != buf)
	{
		PRINT_ERROR();
		std::cout << data << " != " << buf << std::endl;
		return false;
	}
	//send a couple times then recieve a couple of times
	std::string data1 = "data one";
	net_send.Send(data1.c_str(), data1.size()+1, &reciever, flags);
	//large amount of data
	std::string data2;
	for (int i = 0; i < 1; ++i)
	{
		data2 += 'a';
	}
	net_send.Send(data2.c_str(), data2.size()+1, &reciever, flags);
	std::string data3 = "data three";
	net_send.Send(data3.c_str(), data3.size()+1, &reciever, flags);
	std::string data4 = "last";
	net_send.Send(data4.c_str(), data4.size()+1, &reciever, flags);
	//recieve the sent data
	net_recv.Receive(buf, MAXPACKETSIZE, &location);
	if (data1 != buf)
	{
		PRINT_ERROR();
		return false;
	}
	net_recv.Receive(buf, MAXPACKETSIZE, &location);
	if (data2 != buf)
	{
		PRINT_ERROR();
		return false;
	}
	net_recv.Receive(buf, MAXPACKETSIZE, &location);
	if (data3 != buf)
	{
		PRINT_ERROR();
		return false;
	}
	net_recv.Receive(buf, MAXPACKETSIZE, &location);
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

bool TestBitArray()
{
	BitArray<13> one;
	BitArray<32> two;
	if (sizeof(one) != 2)
	{
		PRINT_ERROR();
		return false;
	}
	if (sizeof(two) != 4)
	{
		PRINT_ERROR();
		return false;
	}
	bool bools[101];
	BitArray<101> bits;
	//set bools and bits to the same data
	for (unsigned i = 0; i < 101; ++i)
	{
		bool temp = rand() % 2 == 0;
		bools[i] = temp;
		bits.SetBit(i, temp);
	}
	//check that they are the same
	for (unsigned i = 0; i < 101; ++i)
	{
		if (bits[i] != bools[i])
		{
			PRINT_ERROR();
			return false;
		}
	}
	return true;
}

bool TestReliability()
{
	//set up a connection with ourself
	sockaddr_in sender;
	sockaddr_in reciever;
	Init();
	CreateAddress("127.0.0.1", 4500, &sender);
	CreateAddress("127.0.0.1", 4501, &reciever);
	SOCKET send_sock = CreateSocket(IPPROTO_UDP);
	SOCKET recieve_sock = CreateSocket(IPPROTO_UDP);
	Bind(send_sock, &sender);
	Bind(recieve_sock, &reciever);
	SetNonBlocking(send_sock);
	SetNonBlocking(recieve_sock);

	//now create our network stack
	NetworkStack net_send(send_sock);
	NetworkStack net_recv(recieve_sock);
	net_send.AddLayer(new Channel());
	net_send.AddLayer(new Reliability());
	net_recv.AddLayer(new Channel());
	net_recv.AddLayer(new Reliability());
	
	//send a reliable message to myself
	BitArray<HEADERSIZE> flags;
	flags.SetBit(0);
  net_send.Send("Hello World", 12, &reciever, flags);

  //recieve message
  char buf[100];
  sockaddr_in temp;
  net_recv.Receive(buf, 100, &temp);
  if (!(temp.sin_addr.s_addr == sender.sin_addr.s_addr))
  {
  	PRINT_ERROR();
  	return false;
  }
  if (std::string(buf)!=std::string("Hello World"))
  {
  	PRINT_ERROR();
  	return false;
  }
  //reciev the ack
  int n = net_send.Receive(buf, 100, &temp);
  if (n != 0)
  {
  	PRINT_ERROR();
  	return false;
  }
  if (!(temp.sin_addr.s_addr == reciever.sin_addr.s_addr))
  {
  	PRINT_ERROR();
  	return false;
  }
  //increase packet drop to 50 percent
  PacketDropRate = 0.5;
  //test that reliability will send packets even with packet loss
  FrameRate timer;
  //send 20 reliable messages
  std::string msg = "fruit loops";
  int msg_sent = 50;
  for (int i = 0; i < msg_sent; ++i)
  {
  	net_send.Send(msg.c_str(), msg.size()+1, &reciever, flags);
  }
  //a counter for how many of the reliable messages have been recieved
  int recved = 0;
  //loop for 5 seconds
  while(timer.GetTotalTime() < 5)
  {
  	//using same buf and addr as above recieve call
  	int n = net_recv.Receive(buf, 100, &temp);
  	//we got the message
  	if (n > 1)
  	{
  		recved += 1;
  	}
  	//to get the acks
  	net_send.Receive(buf, 100, &temp);
  	net_recv.Update();
  	net_send.Update();
  }
  //turn off artificail packet loss
  PacketDropRate = 0;
  if (recved != msg_sent)
  {
  	PRINT_ERROR();
  	std::cout << "only recieved " << recved << " of " << msg_sent << std::endl;
  	return false;
  }
	Close(send_sock, true);
	Close(recieve_sock, true);
	Deinit();
  return true;
}

#endif //DOXYGEN_SHOULD_SKIP_THIS