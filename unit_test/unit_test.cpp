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
#include "prioritization.h"
#include "encryption.h"
#include "database.h"
#include "event.h"
#include "protocol.h"

#ifndef DOXYGEN_SHOULD_SKIP_THIS

#define PRINT_ERROR(x) std::cout << "line " << __LINE__ << " function " << __FUNCTION__ << std::endl;

//helper function prints content of buffer as hex
void HexDump(char *buffer, int length);
//prints entire buffer not stopping at null character
void Print(char *buffer, int length);

bool TestInferType();//0
bool TestStringToValue();//1
bool TestConfig();//2
bool TestHashFunction();//3
bool TestFrameRate();//4
bool TestBlowFish();//5
bool TestNetworkLayer();//6
bool TestBitArray();//7
bool TestReliability();//8
bool TestBandwidth();//9
bool TestPriority();//10
bool TestEncryptionLayer();//11
bool TestDatabase();//12
bool TestEventSystem();//13
bool TestDatabaseCreate();//14
bool TestDatabaseGetAndSet();//15
bool TestDatabaseFind();//16
bool TestDatabaseDelete();//17
bool TestProtocol();//18

bool (*tests[])() = { 
    TestInferType, TestStringToValue, TestConfig, TestHashFunction, TestFrameRate, TestBlowFish,
    TestNetworkLayer, TestBitArray, TestReliability, TestBandwidth, TestPriority, TestEncryptionLayer,
    TestDatabase, TestEventSystem, TestDatabaseCreate, TestDatabaseGetAndSet, TestDatabaseFind,
    TestDatabaseDelete, TestProtocol
}; 

int main(int argc, char **argv)
{
	//skip tests
	unsigned i = 0;
	if (argc == 2)
	{
		i = atoi(argv[1]);
	}
	srand(time(0));
	int num_failed = 0;
	for (;i < sizeof(tests) / sizeof(tests[0]); ++i)
	{
		std::cout << "test "<<i << std::endl;
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
	config.Init("resources/test_conf.txt");
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
  void RemoveConnection(__attribute__((unused))const sockaddr_in *addr)
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
	void RemoveConnection(__attribute__((unused))const sockaddr_in *addr)
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

//this one is hard to test, the real test will be in the stress test
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

//this one is hard to test, will be tested better in the stress test
//bandwidth is no longer being used.
bool TestBandwidth()
{
	sockaddr_in sender;
	sockaddr_in reciever;
	sockaddr_in other;
	Init();
	CreateAddress("127.0.0.1", 4876, &sender);
	CreateAddress("127.0.0.1", 4877, &reciever);
	SOCKET send_sock = CreateSocket(IPPROTO_UDP);
	SOCKET recieve_sock = CreateSocket(IPPROTO_UDP);
	Bind(send_sock, &sender);
	Bind(recieve_sock, &reciever);
	SetNonBlocking(send_sock);
	SetNonBlocking(recieve_sock);

	//now create our network stack
	NetworkStack net_send(send_sock);
	NetworkStack net_recv(recieve_sock);
	
	//flags just default nothing set
	BitArray<HEADERSIZE> flags;
	//number of messages to send
	unsigned num = 100;
	//empty memory
	char buffer[MAXPACKETSIZE] = {0};
	//size of each message
	unsigned size_of_message = 100;
	//timer
	FrameRate timer;
	//time to sleep for in seconds
	float sleep_time = 0.01;
	//stop interation
	unsigned stop = 5;
	//the mesaured bandwidths
	std::vector<double> bandwidths;
	for (unsigned i = 0; i <num; ++i)
	{
		//wait for a bit
		FrameRate t;
		while (t.GetTotalTime() < sleep_time){}
		net_send.Send(buffer, size_of_message, &reciever, flags);
		net_recv.Send(buffer, size_of_message, &sender, flags);
		net_send.Receive(buffer, MAXPACKETSIZE, &other);
		net_recv.Receive(buffer, MAXPACKETSIZE, &other);
		if (i == stop)
		{
			//get the bandwidth
			bandwidths.push_back(net_send.GetBandwidth());
			//increase message size to increase next bandwidth
			size_of_message += 100;
			//make a new stop time
			stop += 5;
			net_send.Update();
			net_recv.Update();
		}
	}
	//make sure the bandwidths are in increasing order
	for (unsigned i = 1; i < bandwidths.size(); ++i)
	{
		if (bandwidths[i-1] > bandwidths[i])
		{
			PRINT_ERROR();
	    Deinit();
			return false;
		}
	}
	Deinit();
	return true;
}

bool TestPriority()
{
	sockaddr_in sender;
	sockaddr_in reciever;
	sockaddr_in other;
	Init();
	CreateAddress("127.0.0.1", 4878, &sender);
	CreateAddress("127.0.0.1", 4879, &reciever);
	SOCKET send_sock = CreateSocket(IPPROTO_UDP);
	SOCKET recieve_sock = CreateSocket(IPPROTO_UDP);
	Bind(send_sock, &sender);
	Bind(recieve_sock, &reciever);
	SetNonBlocking(send_sock);
	SetNonBlocking(recieve_sock);

	//now create our network stack
	NetworkStack net_send(send_sock);
	NetworkStack net_recv(recieve_sock);
	int bandwidth = 1000;
	//add some layers
	net_send.AddLayer(new Channel());
	net_send.AddLayer(new Reliability());
	net_send.AddLayer(new Prioritization(bandwidth));
	net_recv.AddLayer(new Channel());
	net_recv.AddLayer(new Reliability());
	net_recv.AddLayer(new Prioritization(bandwidth));
	//flags just default nothing set
	BitArray<HEADERSIZE> flags;
	flags.SetBit(ReliableFlag);
	char *packet = new char[bandwidth]();
	int num_send = 8;
	int num_sent = 0;
	//send some messages
	for (int i = 0; i < num_send; ++i)
	{
		int n = net_send.Send(packet, bandwidth, &reciever, flags);
		if (n > 0)
		{
			num_sent += 1;
		}
	}
	//we should have been way over the bandwidth so it should not have sent most of them
	if (num_send == num_sent)
	{
		PRINT_ERROR();
		return false;
	}
	int num_rec = 0;
	//to prevent infinite loop
	int counter = 100000;
	while (num_rec != num_sent)
	{
		int n = net_recv.Receive(packet, bandwidth, &other);
		if (n > 0)
		{
			num_rec += 1;
		}
		net_send.Update();
		net_recv.Update();
		if (counter-- == 0)
		{
			PRINT_ERROR();
			return false;
		}
	}
	delete [] packet;
	return true;
}

//test without network stack
bool TestEncryptionLayer()
{
	Encryption test_layer;
	BitArray<HEADERSIZE> flags;
	flags.SetBit(EncryptFlag);
	char buffer[MAXPACKETSIZE] = "A secret message for secret people!";
	int len = strlen(buffer);
	sockaddr_in test_addr;
	memset(&test_addr, 0, sizeof(test_addr));
	unsigned key[10];
	for (unsigned iter = 0; iter < 100; ++iter)
	{
		char temp[MAXPACKETSIZE] = {0};
		for (int i = 0; i < len; ++i)
		{
			temp[i] = buffer[i];
		}
		for (unsigned i = 0; i < sizeof(key) / sizeof(unsigned); ++i)
		{
			key[i] = rand();
		}
		test_layer.blowfish[test_addr].SetKey(key, sizeof(key) / sizeof(unsigned));
		int n = test_layer.Send(buffer, len, &test_addr, flags);
		if (unsigned(n-len) > 8)
		{
			PRINT_ERROR();
			return false;
		}
		//make sure the buffer changed suffeicently
		int changes = 0;
		for (int i = 0; i < len; ++i)
		{
			if (buffer != temp)
			{
				changes += 1;
			}
		}
		//if the buffers arnt 90 percent different its wrong
		if (changes / (float)len < 0.9)
		{
			PRINT_ERROR();
			Print(buffer, len);
			Print(temp, len);
			return false;
		}
		n = test_layer.Receive(buffer, MAXPACKETSIZE, &test_addr, flags);
		//make sure they are te same again
		for (int i = 0; i < len; ++i)
		{
			if (buffer[i] != temp[i])
			{
				PRINT_ERROR();
				HexDump(buffer, len);
				HexDump(temp, len);
				return false;
			}
		}
		//minus ten for room to add pad bytes and minus ten plus ten to avoid length zero
		len = rand() % (MAXPACKETSIZE - 10 - 10)+10;
		for (int i = 0; i < len; ++i)
		{
			buffer[i] = rand();
		}
	}
	return true;
}

//TODO: this unit test tests the entire database, the entire database is not a unit this
//should be broken up into multiple unit tests for each database function
bool TestDatabase()
{
	std::vector<unsigned> rows = {16,8,8,2,1,4};
	std::vector<unsigned> types = { Database::Types::String, Database::Types::Double, Database::Types::Double, 
		                              Database::Types::Short, Database::Types::Char, Database::Types::Integer };
  std::vector<unsigned> sorted = {1, 1, 1, 1, 1, 1};
	//makes a new table replacing the old one
	Database db("generated/test_table.tbl", rows, types, sorted);
	char buffer[16+8+8+2+1+4] = {0};
	//try finding an element when there are no elemenets
	//does not matter whats in buffer
	db.Find(0, buffer);
	unsigned num_elements = 100;
	//add random things to the database
	for (unsigned i = 0; i < num_elements; ++i)
	{
		//randomise the buffer
		for (unsigned j = 0; j < sizeof(buffer) / sizeof(buffer[0]); ++j)
		{
			buffer[j] = rand();
		}
		int id = db.Create();
		db.Set(id, 0, buffer);
		db.Set(id, 1, buffer+16);
		db.Set(id, 2, buffer+16+8);
		db.Set(id, 3, buffer+16+8+8);
		db.Set(id, 4, buffer+16+8+8+2);
		db.Set(id, 5, buffer+16+8+8+1+1);
	}
	//test database things
	for (unsigned iter = 0; iter < 1000; ++iter)
	{
		//get a random id to test
		unsigned id = rand()%db.size;
		if (id == 0)
		{
			id = 1;
		}
		//random row
		unsigned row = rand()%rows.size();
		//randomise the value
		for (unsigned j = 0; j < rows[row]; ++j)
		{
			buffer[j] = rand();
		}
		//set the value
		db.Set(id, row, buffer);
		//then search the database for that value
		std::vector<unsigned> res = db.Find(row, buffer);
		//make sure the id is in the list
		bool found = false;
		for (unsigned i = 0; i < res.size(); ++i)
		{
			if (res[i] == id)
			{
				found = true;
				break;
			}
		}
		if (found == false)
		{
			PRINT_ERROR();
			std::cout << res.size() << " id= " << id << " row = " << row << std::endl;
			if (res.size() > 0)
			{
				std::cout << res[0] << std::endl;
			}
			return false;
		}
	}
	//test delete, this could delete every object, but there is garrenteed to be at least one double delete
	for (unsigned iter = 0; iter < db.size+1; ++iter)
	{
		//get a random id to test, (this could be the id of a deleted object nd that should be fine)
		unsigned id = rand()%db.size;
		//make sure its not null id
		if (id == 0)
		{
			id = 1;
		}
		//random row
		unsigned row = rand()%rows.size();
		//randomise the value
		for (unsigned j = 0; j < rows[row]; ++j)
		{
			buffer[j] = rand();
		}
		//set the value(if the id is already deleted this will still set the data)
		db.Set(id, row, buffer);
		//delete the object
		db.Delete(id);
		//then search the database for that value (the object that we deleted should not be found)
		std::vector<unsigned> res = db.Find(row, buffer);
		//make sure the id is NOT in the list
		for (unsigned i = 0; i < res.size(); ++i)
		{
			if (res[i] == id)
			{
				PRINT_ERROR();
				return false;
			}
		}
	}
	//loop for some of the deleted objects and create new ones
	for (int i = 0; i < db.num_ids; ++i)
	{
		unsigned id = db.Create();
		//random row
		unsigned row = rand()%rows.size();
		for (unsigned j = 0; j < rows[row]; ++j)
		{
			buffer[j] = rand();
		}
		//set the value(if the id is already deleted this will still set the data)
		db.Set(id, row, buffer);
		//then search the database for that value
		std::vector<unsigned> res = db.Find(row, buffer);
		//make sure the id is in the list
		bool found = false;
		for (unsigned i = 0; i < res.size(); ++i)
		{
			if (res[i] == id)
			{
				found = true;
				break;
			}
		}
		if (found == false)
		{
			PRINT_ERROR();
			return false;
		}
	}
	return true;
}

//class to test event system, in all tests the data is an integer
class EventTester
{
public:
	int num_called = 0;
	int num_called2 = 0;
	void memberfunction(__attribute__((unused))void *data)
	{
		num_called += 1;
	}
	void memberfunction2(__attribute__((unused))void *data)
	{
		num_called2 += 1;
	}
};

//to test a non member function function, the data is an integer, each function call the integer is incremented
void normalfuntion(void *data)
{
	*static_cast<int*>(data) += 1;
}

//TODO: Similar to Database test this could be split into threeish unit tests
bool TestEventSystem()
{
	Event event;
	//make sure sending an event with noboy listening does not crash
	event.SendEvent(EventType::TestEvent, 0);
	//connect a normal function
	event.ConnectEvent(EventType::TestEvent, &normalfuntion);
	//inteer that should be incremented everytime normal function is called, starting at a magic number
	int num = 4;
	event.SendEvent(EventType::TestEvent, &num);
	if (num != 5)
	{
		std::cout << num << std::endl;
		PRINT_ERROR();
		return false;
	}
	//connect a second noramal function of the same type, (now on send event the function should be called twice)
	event.ConnectEvent(EventType::TestEvent, &normalfuntion);
	event.SendEvent(EventType::TestEvent, &num);
	if (num != 7)
	{
		std::cout << num << std::endl;
		PRINT_ERROR();
		return false;
	}
	//remove one of the normal functions
	event.DisconnectEvent(EventType::TestEvent, &normalfuntion);
	//check to make sure that there is now only one function called
	event.SendEvent(EventType::TestEvent, &num);
	if (num != 8)
	{
		std::cout << num << std::endl;
		PRINT_ERROR();
		return false;
	}
	//now add a member fucntion as well
	EventTester obj;
	event.ConnectEvent(EventType::TestEvent, &obj, &EventTester::memberfunction);
	event.SendEvent(EventType::TestEvent, &num);
	//this should only increment by one because it only calls the normal function once
	if (num != 9)
	{
		std::cout << num << std::endl;
		PRINT_ERROR();
		return false;
	}
	//the object funtion should have been called once
	if (obj.num_called != 1)
	{
		std::cout << num << std::endl;
		PRINT_ERROR();
		return false;
	}
	//add memer function to a different event type to make sure that different event types work
	event.ConnectEvent(EventType::TestEvent2, &obj, &EventTester::memberfunction);
	event.SendEvent(EventType::TestEvent2, &num);
	//num should remain unchanged
	if (num != 9)
	{
		std::cout << num << std::endl;
		PRINT_ERROR();
		return false;
	}
	//the object funtion should have been called once
	if (obj.num_called != 2)
	{
		std::cout << num << std::endl;
		PRINT_ERROR();
		return false;
	}
	//add a second object to the event system
	EventTester obj2;
	event.ConnectEvent(EventType::TestEvent, &obj2, &EventTester::memberfunction);
	//add prev object with different member function
	event.ConnectEvent(EventType::TestEvent, &obj, &EventTester::memberfunction2);
	//test removing a member function
	event.DisconnectEvent(EventType::TestEvent, &obj, &EventTester::memberfunction);
	event.SendEvent(EventType::TestEvent, &num);
	//num should have incremented once
	if (num != 10)
	{
		PRINT_ERROR();
		return false;
	}
	//the object funtion should not have been called
	if (obj.num_called != 2)
	{
		PRINT_ERROR();
		return false;
	}
	//second object shoud have had his fun called
	if (obj2.num_called != 1)
	{
		PRINT_ERROR();
		return false;
	}
	//member function 2 should have still been called
	if (obj.num_called2 != 1)
	{
		PRINT_ERROR();
		return false;
	}
	return true;
}

bool TestDatabaseCreate()
{
	return true;
}
bool TestDatabaseGetAndSet()
{
	return true;
}
bool TestDatabaseFind()
{
	return true;
}
bool TestDatabaseDelete()
{
	return true;
}

bool TestProtocol()
{
	ProtocolLoader pl;
	pl.LoadProtocol();
	std::cout << "billy" << std::endl;
	if (pl.message_types["one"] != 0)
	{
		std::cout << "Message one is " << pl.message_types["one"] << std::endl;
		return false;
	}
	if (pl.message_types["two"] != 1)
	{
		std::cout << "Message two is " << pl.message_types["two"] << std::endl;
		return false;
	}
	if (pl.message_types["three"] != 2)
	{
		std::cout << "Message three is " << pl.message_types["three"] << std::endl;
		return false;
	}
	std::cout << "yo wtf" << std::endl;
	return true;
}

void HexDump(char *buffer, int length)
{
  std::cout << std::hex;
	for (int i = 0; i < length; ++i)
	{
		std::cout << (int)((unsigned char)buffer[i]) << " ";
	}
	std::cout << std::endl;
	std::cout << std::dec;
}

void Print(char *buffer, int length)
{
	for (int i = 0; i < length; ++i)
	{
		std::cout << buffer[i];
	}
	std::cout << std::endl;
}

#endif //DOXYGEN_SHOULD_SKIP_THIS