CC = gcc
PROTOC = protoc

DIR_BIN = binary
DIR_OBJ = object
DIR_SRC = source
DIR_INC = include
DIR_LIB = lib
DIR_PROTO= proto


CFLAGS = -I $(DIR_INC) -Wall -g 
PROTOFLAGS = -I/usr/local/include -L/usr/local/lib/libprotobuf-c.a -lprotobuf-c -lpthread -lzookeeper_mt
LDFLAGS = ${PROTO_DIR}lib/libprotobuf-c.a 

MAIN_OBJ = $(addprefix $(DIR_OBJ)/,tree_client.o tree_server.o)
CLIENT_LIB = $(addprefix $(DIR_OBJ)/, data.o entry.o message.o client_stub.o network_client.o sdmessage.pb-c.o)
SERVER_LIB = $(addprefix $(DIR_OBJ)/, data.o entry.o tree.o message.o tree_skel.o network_server.o sdmessage.pb-c.o)

LIBS = $(addprefix $(DIR_LIB)/, client-lib.o server-lib.o)

PROTO_DIR =/usr/local


all: clean protoComp tree-client tree-server

tree-client: $(MAIN_OBJ) $(LIBS)
	$(CC) $(CFLAGS) $(DIR_OBJ)/tree_client.o $(DIR_LIB)/client-lib.o $(PROTOFLAGS) -o $(DIR_BIN)/tree-client 

tree-server: $(MAIN_OBJ) $(LIBS)
	$(CC) $(CFLAGS) $(DIR_OBJ)/tree_server.o $(DIR_LIB)/server-lib.o $(PROTOFLAGS) -o $(DIR_BIN)/tree-server

protoComp:
	$(PROTOC) -I=$(DIR_PROTO) --c_out=. $(DIR_PROTO)/sdmessage.proto
	mv sdmessage.pb-c.h $(DIR_INC)
	mv sdmessage.pb-c.c $(DIR_SRC)

$(DIR_LIB)/client-lib.o: $(CLIENT_LIB) 
	ld -r -o $@ $(CLIENT_LIB)

$(DIR_LIB)/server-lib.o: $(SERVER_LIB)
	ld -r -o $@ $(SERVER_LIB)

$(DIR_OBJ)/%.o: $(DIR_SRC)/%.c
	$(CC) $(CFLAGS) -o $@ -c $<




clean:
	rm -f $(DIR_OBJ)/*
	rm -f $(DIR_BIN)/*
	rm -f $(DIR_LIB)/*
	rm -f $(DIR_INC)/sdmessage.pb-c.h
	rm -f $(DIR_SRC)/sdmessage.pb-c.c
	
