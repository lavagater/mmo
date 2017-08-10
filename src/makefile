# Macros ========================================

CC=g++
CFLAGS=-c -Wall -Wextra -Werror -ansi -pedantic --std=c++11

OUTDIR=bin/
ERASE=rm
DATABASE_OBJECTS=$(OUTDIR)database_app.o
ZONE_OBJECTS=$(OUTDIR)zone.o
BALANCE_OBJECTS=$(OUTDIR)load_balancer.o
GENERAL_OBJECTS=$(OUTDIR)wyatt_sock.o $(OUTDIR)config.o $(OUTDIR)meta.o $(OUTDIR)network_stack.o \
                $(OUTDIR)channel.o $(OUTDIR)frame_rate.o $(OUTDIR)blowfish.o $(OUTDIR)reliability.o \
                $(OUTDIR)prioritization.o $(OUTDIR)encryption.o $(OUTDIR)timer.o $(OUTDIR)database.o $(OUTDIR)event.o \
                $(OUTDIR)remote_database.o $(OUTDIR)protocol.o
TEST_OJECTS=$(OUTDIR)unit_test.o
ALL_OBJECTS=$(GENERAL_OBJECTS) $(BALANCE_OBJECTS) $(ZONE_OBJECTS) $(DATABASE_OBJECTS) $(TEST_OJECTS)
LOAD_BALANCER=load_balancer.out
DATABASE=databse.out
ZONE=zone.out
UNIT=unit.out

# Targets ========================================
all: $(OUTDIR)$(UNIT) $(OUTDIR)$(LOAD_BALANCER) $(OUTDIR)$(ZONE) $(OUTDIR)$(DATABASE) \
	$(OUTDIR)stress_test.out $(OUTDIR)table_maker.out $(OUTDIR)TextMMO.out $(OUTDIR)TextMMOClient.out

#unit tests
$(OUTDIR)$(UNIT) : $(TEST_OJECTS) $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)$(UNIT) $(TEST_OJECTS) $(GENERAL_OBJECTS)

#stress tests
$(OUTDIR)stress_test.out : $(OUTDIR)stress_test.o $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)stress_test.out $(OUTDIR)stress_test.o $(GENERAL_OBJECTS) 

#table maker
$(OUTDIR)table_maker.out : $(OUTDIR)table_maker.o $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)table_maker.out $(OUTDIR)table_maker.o $(GENERAL_OBJECTS) 

#The textmmo
$(OUTDIR)TextMMO.out : $(OUTDIR)TextMMO.o $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)TextMMO.out $(OUTDIR)TextMMO.o $(GENERAL_OBJECTS)

$(OUTDIR)TextMMO.o : TextMMO.cpp $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)TextMMO.o $(CFLAGS) TextMMO.cpp

#The textmmo
$(OUTDIR)TextMMOClient.out : $(OUTDIR)TextMMOClient.o $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)TextMMOClient.out $(OUTDIR)TextMMOClient.o $(GENERAL_OBJECTS)

$(OUTDIR)TextMMOClient.o : TextMMOClient.cpp $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)TextMMOClient.o $(CFLAGS) TextMMOClient.cpp

# Load balancer
$(OUTDIR)$(LOAD_BALANCER) : $(BALANCE_OBJECTS) $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)$(LOAD_BALANCER) $(BALANCE_OBJECTS) $(GENERAL_OBJECTS)

# Zone server
$(OUTDIR)$(ZONE) : $(ZONE_OBJECTS) $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)$(ZONE) $(ZONE_OBJECTS) $(GENERAL_OBJECTS)

# Database
$(OUTDIR)$(DATABASE) : $(DATABASE_OBJECTS) $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)$(DATABASE) $(DATABASE_OBJECTS) $(GENERAL_OBJECTS)


$(OUTDIR)unit_test.o  : unit_test.cpp $(GENERAL_OBJECTS) bit_array.h
	$(CC) -o $(OUTDIR)unit_test.o $(CFLAGS) unit_test.cpp

$(OUTDIR)stress_test.o  : stress_test.cpp $(GENERAL_OBJECTS) bit_array.h
	$(CC) -o $(OUTDIR)stress_test.o $(CFLAGS) stress_test.cpp

$(OUTDIR)table_maker.o  : table_maker.cpp $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)table_maker.o $(CFLAGS) table_maker.cpp

$(OUTDIR)zone.o  : zone.cpp
	$(CC) -o $(OUTDIR)zone.o $(CFLAGS) zone.cpp

$(OUTDIR)database_app.o  : database_app.cpp
	$(CC) -o $(OUTDIR)database_app.o $(CFLAGS) database_app.cpp

$(OUTDIR)database.o  : database.cpp database.h
	$(CC) -o $(OUTDIR)database.o $(CFLAGS) database.cpp

$(OUTDIR)remote_database.o  : remote_database.cpp remote_database.h
	$(CC) -o $(OUTDIR)remote_database.o $(CFLAGS) remote_database.cpp

$(OUTDIR)protocol.o  : protocol.cpp protocol.h
	$(CC) -o $(OUTDIR)protocol.o $(CFLAGS) protocol.cpp

$(OUTDIR)load_balancer.o  : load_balancer.cpp
	$(CC) -o $(OUTDIR)load_balancer.o $(CFLAGS) load_balancer.cpp

$(OUTDIR)wyatt_sock.o : wyatt_sock.cpp wyatt_sock.h
	$(CC) -o $(OUTDIR)wyatt_sock.o $(CFLAGS) wyatt_sock.cpp

$(OUTDIR)timer.o : timer.cpp timer.h
	$(CC) -o $(OUTDIR)timer.o $(CFLAGS) timer.cpp	

$(OUTDIR)event.o : event.cpp event.h
	$(CC) -o $(OUTDIR)event.o $(CFLAGS) event.cpp	

$(OUTDIR)channel.o : channel.cpp channel.h wyatt_sock.h
	$(CC) -o $(OUTDIR)channel.o $(CFLAGS) channel.cpp

$(OUTDIR)encryption.o : encryption.cpp encryption.h wyatt_sock.h
	$(CC) -o $(OUTDIR)encryption.o $(CFLAGS) encryption.cpp

$(OUTDIR)reliability.o : reliability.cpp reliability.h wyatt_sock.h
	$(CC) -o $(OUTDIR)reliability.o $(CFLAGS) reliability.cpp

$(OUTDIR)prioritization.o : prioritization.cpp prioritization.h reliability.h wyatt_sock.h
	$(CC) -o $(OUTDIR)prioritization.o $(CFLAGS) prioritization.cpp

$(OUTDIR)config.o : config.cpp config.h meta.h
	$(CC) -o $(OUTDIR)config.o $(CFLAGS) config.cpp

$(OUTDIR)meta.o : meta.cpp meta.h
	$(CC) -o $(OUTDIR)meta.o $(CFLAGS) meta.cpp

$(OUTDIR)blowfish.o : blowfish.cpp blowfish.h
	$(CC) -o $(OUTDIR)blowfish.o $(CFLAGS) blowfish.cpp

$(OUTDIR)frame_rate.o : frame_rate.cpp frame_rate.h
	$(CC) -o $(OUTDIR)frame_rate.o $(CFLAGS) frame_rate.cpp

$(OUTDIR)network_stack.o : network_stack.cpp network_stack.h wyatt_sock.h
	$(CC) -o $(OUTDIR)network_stack.o $(CFLAGS) network_stack.cpp

clean :
	$(ERASE) $(OUTDIR)$(DATABASE) $(OUTDIR)$(ZONE) $(OUTDIR)$(LOAD_BALANCER) $(OUTDIR)$(UNIT) $(ALL_OBJECTS)
	