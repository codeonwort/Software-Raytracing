# variables
CC=g++
CFLAGS=
SRC=src/
DST=bin/

# ready to create directories if not exists
DIRS=bin

# main target
main: $(SRC)main.cc
	$(CC) -o $(DST)main $(SRC)main.cc

# create all directories if not exists
$(shell mkdir -p $(DIRS))
