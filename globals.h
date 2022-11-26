#ifndef GLOBALS_H
#define GLOBALS_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>

#define MAX_CLIENTS 300
#define USERNAME_SIZE 50
#define QUIT "\\q\n"

#define SERVER_PORT 31038
#define SERVER_ADDRESS "127.0.0.1"
//#define SERVER_ADDRESS "192.168.1.101"
#define SERVER_IF "eth0"

#define MSG_TYPE_SIZE 6
#define BROADCAST "BRCST:"
#define LOGIN "LOGIN:"
#define LOGOUT "LGOUT:"
#define READY "READY:"
#define PRIVATE "PRIVT:"
#define ERROR "ERROR:"

enum Message_Type {
	Broadcast,
	Login,
	Logout,
	Ready,
	Private,
	Error
};

typedef enum Message_Type MessageType;

struct Message_s {
	char info[BUFSIZ];
	MessageType type;
};

typedef struct Message_s Message;

struct ClientData_s {
	int socket;
	char addr[15];
	char username[USERNAME_SIZE];
};

typedef struct ClientData_s ClientData;

#endif // GLOBALS_H
