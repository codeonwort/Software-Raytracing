# variables
CC=g++
CFLAGS=-std=c++14
SRC_DIR=src/
SRC_FILES=$(wildcard $(SRC_DIR)*.cc)
DST=bin/

#$(info $$SRC_FILES is [${SRC_FILES}])

# ready to create directories if not exists
DIRS=bin

# main target
main: $(SRC_DIR)main.cc
	$(CC) -o $(DST)main $(SRC_FILES) $(CFLAGS)

# create all directories if not exists
$(shell mkdir -p $(DIRS))
