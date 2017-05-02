# Macros ========================================

CC=g++
CFLAGS=-c -Wall -Wextra -Werror -ansi -pedantic --std=c++11

OUTDIR=bin/
ERASE=rm
DATABASE_OBJECTS=
ZONE_OBJECTS=
BALANCE_OBJECTS=
GENERAL_OBJECTS=$(OUTDIR)wyatt_sock.o
TEST_OJECTS=$(OUTDIR)unit_test.o
ALL_OBJECTS=$(GENERAL_OBJECTS) $(BALANCE_OBJECTS) $(ZONE_OBJECTS) $(DATABASE_OBJECTS) $(TEST_OJECTS)
LOAD_BALANCER=load_balancer.out
DATABASE=databse.out
ZONE=zone.out
UNIT=unit.out

# Targets ========================================
all: $(OUTDIR)$(UNIT)
#$(OUTDIR)$(LOAD_BALANCER) $(OUTDIR)$(DATABASE) $(OUTDIR)$(ZONE)

#unit tests
$(OUTDIR)$(UNIT) : $(ALL_OBJECTS)
	$(CC) -o $(OUTDIR)$(UNIT) $(ALL_OBJECTS)

# Load balancer
$(OUTDIR)$(LOAD_BALANCER) : $(BALANCE_OBJECTS) $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)$(LOAD_BALANCER) $(BALANCE_OBJECTS) $(GENERAL_OBJECTS)

# Zone server
$(OUTDIR)$(LOAD_BALANCER) : $(ZONE_OBJECTS) $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)$(LOAD_BALANCER) $(ZONE_OBJECTS) $(GENERAL_OBJECTS)

# Database
$(OUTDIR)$(DATABASE) : $(DATABASE_OBJECTS) $(GENERAL_OBJECTS)
	$(CC) -o $(OUTDIR)$(DATABASE) $(DATABASE_OBJECTS) $(GENERAL_OBJECTS)


$(OUTDIR)unit_test.o  : unit_test.cpp
	$(CC) -o $(OUTDIR)unit_test.o $(CFLAGS) unit_test.cpp

$(OUTDIR)wyatt_sock.o : wyatt_sock.cpp wyatt_sock.h
	$(CC) -o $(OUTDIR)wyatt_sock.o $(CFLAGS) wyatt_sock.cpp


clean :
	$(ERASE) $(OUTDIR)$(DATABASE) $(OUTDIR)$(ZONE) $(OUTDIR)$(LOAD_BALANCER) $(OUTDIR)$(UNIT) $(ALL_OBJECTS)
	