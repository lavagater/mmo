# Macros ========================================

CC=g++
CFLAGS=-c -Wall -Wextra -Werror -ansi -pedantic --std=c++11

OUTDIR=bin/
ERASE=rm
DATABASE_OBJECTS=$(OUTDIR)database.o
ZONE_OBJECTS=$(OUTDIR)zone.o
BALANCE_OBJECTS=$(OUTDIR)load_balancer.o
GENERAL_OBJECTS=$(OUTDIR)wyatt_sock.o $(OUTDIR)config.o
TEST_OJECTS=$(OUTDIR)unit_test.o
ALL_OBJECTS=$(GENERAL_OBJECTS) $(BALANCE_OBJECTS) $(ZONE_OBJECTS) $(DATABASE_OBJECTS) $(TEST_OJECTS)
LOAD_BALANCER=load_balancer.out
DATABASE=databse.out
ZONE=zone.out
UNIT=unit.out

# Targets ========================================
all: $(OUTDIR)$(UNIT) $(OUTDIR)$(LOAD_BALANCER) $(OUTDIR)$(ZONE) $(OUTDIR)$(DATABASE)

#unit tests
$(OUTDIR)$(UNIT) : $(TEST_OJECTS) $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)$(UNIT) $(TEST_OJECTS) $(GENERAL_OBJECTS)

# Load balancer
$(OUTDIR)$(LOAD_BALANCER) : $(BALANCE_OBJECTS) $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)$(LOAD_BALANCER) $(BALANCE_OBJECTS) $(GENERAL_OBJECTS)

# Zone server
$(OUTDIR)$(ZONE) : $(ZONE_OBJECTS) $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)$(ZONE) $(ZONE_OBJECTS) $(GENERAL_OBJECTS)

# Database
$(OUTDIR)$(DATABASE) : $(DATABASE_OBJECTS) $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)$(DATABASE) $(DATABASE_OBJECTS) $(GENERAL_OBJECTS)


$(OUTDIR)unit_test.o  : unit_test.cpp
	$(CC) -o $(OUTDIR)unit_test.o $(CFLAGS) unit_test.cpp

$(OUTDIR)zone.o  : zone.cpp
	$(CC) -o $(OUTDIR)zone.o $(CFLAGS) zone.cpp

$(OUTDIR)database.o  : database.cpp
	$(CC) -o $(OUTDIR)database.o $(CFLAGS) database.cpp

$(OUTDIR)load_balancer.o  : load_balancer.cpp
	$(CC) -o $(OUTDIR)load_balancer.o $(CFLAGS) load_balancer.cpp

$(OUTDIR)wyatt_sock.o : wyatt_sock.cpp wyatt_sock.h
	$(CC) -o $(OUTDIR)wyatt_sock.o $(CFLAGS) wyatt_sock.cpp

$(OUTDIR)config.o : config.cpp config.h
	$(CC) -o $(OUTDIR)config.o $(CFLAGS) config.cpp

clean :
	$(ERASE) $(OUTDIR)$(DATABASE) $(OUTDIR)$(ZONE) $(OUTDIR)$(LOAD_BALANCER) $(OUTDIR)$(UNIT) $(ALL_OBJECTS)
	