CC = gcc
CFLAGS = -Wall
SDLFLAGS = -lSDL2

SRC_DIR = src

CLIENT_SRC = $(SRC_DIR)/client_main.c $(SRC_DIR)/game.c $(SRC_DIR)/ui.c
SERVER_SRC = $(SRC_DIR)/server_main.c

CLIENT_OUT = client
SERVER_OUT = server

all: $(CLIENT_OUT) $(SERVER_OUT)

$(CLIENT_OUT): $(CLIENT_SRC)
	$(CC) $(CFLAGS) $(CLIENT_SRC) -o $(CLIENT_OUT) $(SDLFLAGS)

$(SERVER_OUT): $(SERVER_SRC)
	$(CC) $(CFLAGS) $(SERVER_SRC) -o $(SERVER_OUT)

clean:
	rm -f $(CLIENT_OUT) $(SERVER_OUT)