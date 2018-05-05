CC=g++
CFLAGS=-c -Wall -Wextra -Werror -ansi -pedantic --std=c++11

OUTDIR=../bin/
ERASE=rm
LOCAL_OBJECTS=$(OUTDIR)wyatt_sock.o $(OUTDIR)config.o $(OUTDIR)meta.o $(OUTDIR)network_stack.o \
              $(OUTDIR)channel.o $(OUTDIR)frame_rate.o $(OUTDIR)blowfish.o $(OUTDIR)reliability.o \
              $(OUTDIR)prioritization.o $(OUTDIR)encryption.o $(OUTDIR)timer.o $(OUTDIR)database.o $(OUTDIR)event.o \
              $(OUTDIR)remote_database.o $(OUTDIR)protocol.o
#OBJECTS should be exported from a previous makefile, and contain all the objects to build the exe
#the objects that were from the previous make file should be up to date
OBJECTS+=$(LOCAL_OBJECTS)

#get the exe from the previous make file
$(EXE) : $(LOCAL_OBJECTS)
	$(CC) -o $(EXE) $(OBJECTS)

$(OUTDIR)protocol.o : protocol.cpp protocol.h
	$(CC) -o $(OUTDIR)protocol.o $(CFLAGS) protocol.cpp

$(OUTDIR)remote_database.o : remote_database.cpp remote_database.h
	$(CC) -o $(OUTDIR)remote_database.o $(CFLAGS) remote_database.cpp

$(OUTDIR)event.o : event.cpp event.h
	$(CC) -o $(OUTDIR)event.o $(CFLAGS) event.cpp

$(OUTDIR)database.o : database.cpp database.h
	$(CC) -o $(OUTDIR)database.o $(CFLAGS) database.cpp

$(OUTDIR)timer.o : timer.cpp timer.h
	$(CC) -o $(OUTDIR)timer.o $(CFLAGS) timer.cpp

$(OUTDIR)encryption.o : encryption.cpp encryption.h
	$(CC) -o $(OUTDIR)encryption.o $(CFLAGS) encryption.cpp

$(OUTDIR)prioritization.o : prioritization.cpp prioritization.h
	$(CC) -o $(OUTDIR)prioritization.o $(CFLAGS) prioritization.cpp

$(OUTDIR)reliability.o : reliability.cpp reliability.h
	$(CC) -o $(OUTDIR)reliability.o $(CFLAGS) reliability.cpp

$(OUTDIR)blowfish.o : blowfish.cpp blowfish.h
	$(CC) -o $(OUTDIR)blowfish.o $(CFLAGS) blowfish.cpp

$(OUTDIR)frame_rate.o : frame_rate.cpp frame_rate.h
	$(CC) -o $(OUTDIR)frame_rate.o $(CFLAGS) frame_rate.cpp

$(OUTDIR)channel.o : channel.cpp channel.h
	$(CC) -o $(OUTDIR)channel.o $(CFLAGS) channel.cpp

$(OUTDIR)network_stack.o : network_stack.cpp network_stack.h
	$(CC) -o $(OUTDIR)network_stack.o $(CFLAGS) network_stack.cpp

$(OUTDIR)meta.o : meta.cpp meta.h
	$(CC) -o $(OUTDIR)meta.o $(CFLAGS) meta.cpp

$(OUTDIR)config.o : config.cpp config.h
	$(CC) -o $(OUTDIR)config.o $(CFLAGS) config.cpp

$(OUTDIR)wyatt_sock.o : wyatt_sock.cpp wyatt_sock.h
	$(CC) -o $(OUTDIR)wyatt_sock.o $(CFLAGS) wyatt_sock.cpp

clean :
	$(ERASE) $(LOCAL_OBJECTS)