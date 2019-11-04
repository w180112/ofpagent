############################################################
# ofpagent makefile
############################################################

######################################
# Set variable
######################################	
CC	= gcc -g
INCLUDE = -I./src/ -I./lib/com_util
CFLAGS = $(INCLUDE) -Wall -fPIC -g -std=c99 -D_XOPEN_SOURCE -D_BSD_SOURCE

TARGET = ofpagent
SRC = ./src/ofpd.c ./src/ofp_sock.c ./src/ofp_fsm.c ./src/ofp_codec.c ./src/ofp_dbg.c

OBJ = $(SRC:.c=.o)
	
######################################
# Compile & Link
# 	Must use \tab key after new line
######################################
$(TARGET): $(OBJ) ./src/*.h
	$(CC) $(CFLAGS) -L./lib/com_util $(OBJ) -o $(TARGET) \
	-static -lcom_util
	
######################################
# Clean 
######################################
clean:
	rm -f *.o ofpagent
