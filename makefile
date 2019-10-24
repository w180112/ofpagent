############################################################
# ofagent makefile
############################################################

######################################
# Set variable
######################################	
CC	= gcc -g
INCLUDE = -I. -I./com_util
CFLAGS = $(INCLUDE) -Wall -fPIC -g -std=c99 -D_XOPEN_SOURCE -D_BSD_SOURCE

TARGET = ofpagent
SRC = ofpd.c ofp_sock.c ofp_fsm.c ofp_codec.c ofp_dbg.c

OBJ = $(SRC:.c=.o)
	
######################################
# Compile & Link
# 	Must use \tab key after new line
######################################
$(TARGET): $(OBJ) *.h
	$(CC) $(CFLAGS) -L./com_util $(OBJ) -o $(TARGET) \
	-static -lcom_util
	
######################################
# Clean 
######################################
clean:
	rm -f *.o ofagent
