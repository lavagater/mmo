INCLUDES+=-I../mmo

CXX = g++
CXX_FLAGS = -Wall -Wextra -Werror -ansi -pedantic --std=c++11 $(INCLUDES)

# Final binary
BIN = query_tester.out
BIN_DIR = bin
# Put all auto generated stuff to this build dir.
BUILD_DIR = ../build

# List of all .cpp source files.
CPP = $(wildcard *.cpp) $(wildcard ../mmo/*.cpp)

# All .o files go to build dir.
NO_DIR = $(notdir $(CPP))
OBJ = $(NO_DIR:%.cpp=$(BUILD_DIR)/%.o)
# Gcc/Clang will create these .d files containing dependencies.
DEP = $(OBJ:%.o=%.d)

# Default target named after the binary.
$(BIN) : $(BIN_DIR)/$(BIN)

# Actual target of the binary - depends on all .o files.
$(BIN_DIR)/$(BIN) : $(OBJ)
	# Create build directories - same structure as sources.
	mkdir -p $(BUILD_DIR)
	#the location of the final exe
	mkdir -p $(BIN_DIR)
	mkdir -p $(BIN_DIR)/assets
	echo this folder is for stuff like art or files that dont change while the program is running > $(BIN_DIR)/assets/ReadMe.txt
	mkdir -p $(BIN_DIR)/generated
	echo this is for files made from the project, should start empty. but can be used however obciously > $(BIN_DIR)/generated/ReadMe.txt
	mkdir -p $(BIN_DIR)/resources
	echo this folder is for config files, database tables, files that change frequently> $(BIN_DIR)/resources/ReadMe.txt
	mkdir -p $(BIN_DIR)/extra
	echo this folder is for what ever > $(BIN_DIR)/extra/ReadMe.txt
	# Just link all the object files.
	$(CXX) $(CXX_FLAGS) $^ -o $@

# Include all .d files
-include $(DEP)

define targets =
NO_DIR$(1) = $$(notdir $(1))
OBJ$(1) = $$(NO_DIR$(1):%.cpp=$$(BUILD_DIR)/%.o)

$$(OBJ$(1)): $(1)
	mkdir -p $$(@D)
	# The -MMD flags additionaly creates a .d file with
	# the same name as the .o file.
	$$(CXX) $$(CXX_FLAGS) -MMD -c $$< -o $$@

endef

$(foreach var,$(CPP),$(eval $(call targets,$(var))))

.PHONY : clean
clean :
	# This should remove all generated files.
	-rm $(BUILD_DIR)/$(BIN) $(OBJ) $(DEP)