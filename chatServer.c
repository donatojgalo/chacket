#include <signal.h>
#include "globals.h"

#define NEW(p, tipo)	\
	/*	*/if ((p = (tipo *) malloc (sizeof(tipo))) == NULL) {	\
	/*		*/printf("NEW: Sin memoria!!!\n");	\
	/*		*/exit(EXIT_FAILURE);	\
	/*	*/}

#define FREE(p)	\
	/*	*/if (p) {	\
	/*		*/free ((char *) p);	\
	/*		*/p = NULL;	\
	/*	*/}

#define ADD(head, p) \
	/*	*/if (head) {	\
	/*		*/p->next = head;	\
	/*		*/p->prev = head->prev;	\
	/*		*/head->prev = p;	\
	/*		*/p->prev->next = p;	\
	/*	*/} else {	\
	/*		*/head = p;	\
	/*		*/head->next = head->prev = p;	\
	/*	*/}

#define DEL(head, i) \
	/*	*/if (head) {	\
	/*		*/ptrAttendant p = head; \
	/*		*/ptrAttendant prev; \
	/*		*/ptrAttendant next; \
	/*		*/do { \
	/*			*/if (p->id == i) { \
	/*				*/prev = p->prev; \
	/*				*/next = p->next;	\
	/*				*/next->prev = p->prev; \
	/*				*/prev->next = p->next;	\
	/*				*/p->next = p->prev = p;	\
	/*				*/FREE(p);	\
	/*				*/break;	\
	/*			*/} \
	/*			*/p = p->next;	\
	/*		*/} while (p->id != 0);	\
	/*	*/}

typedef struct Attendant_s Attendant;
typedef Attendant *ptrAttendant;

struct Attendant_s {
	int id;
	ClientData clientData;
	ptrAttendant next;
	ptrAttendant prev;
};

int serverSocket;
int clientsNum = 0;
ptrAttendant listHead = NULL;

void closeServer();
void *clientManager(void *);
void broadcastExcl(int, const char *);
void broadcast(const char *);
int checkClientExist(char *);
void deleteClient(int);
void showClientsList();
void fixMessageInfo(char *, const char *);
MessageType readMessageType(const char *);

int main() {

	signal(SIGINT, closeServer);

	int clientSocket;
	socklen_t remoteAddrLen;
	struct sockaddr_in localAddr;
	struct sockaddr_in remoteAddr;
	pthread_t thread[MAX_CLIENTS];
	int attendantId = 0;
	ptrAttendant attendant;

	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(SERVER_PORT);
	localAddr.sin_addr.s_addr = INADDR_ANY;

	if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error abriendo el socket");
		exit(1);
	}

	if (bind(serverSocket, (struct sockaddr *)&localAddr,
			 sizeof(localAddr)) < 0) {
		perror("Error haciendo el bind");
		exit(1);
	}

	if (listen(serverSocket, 5) < 0) {
		perror("Error en el listen");
		exit(1);
	}

	printf("\nEsperando conexiones...\n");
	fflush(stdout);

	remoteAddrLen = sizeof(struct sockaddr_in);

	do {

		if ((clientSocket = //
			 accept(serverSocket, (void *)&remoteAddr, //
					(socklen_t *)&remoteAddrLen)) < 0) {

			fprintf(stderr, "Error en el accept.\nclientSocket = %d\n", clientSocket);
			exit(1);

		} else {

			NEW(attendant, Attendant);

			attendant->id = attendantId;
			attendant->clientData.socket = clientSocket;
			strcpy(attendant->clientData.addr, inet_ntoa(remoteAddr.sin_addr));

			ADD(listHead, attendant);

			printf("\nConexión recibida. Dirección IP del cliente: %s\n", //
				   attendant->clientData.addr);
			fflush(stdout);

			if (pthread_create(&thread[clientsNum], 0, clientManager, (void *)attendant) != 0) {
				perror("Error creando el hilo para atender el cliente.");
				fflush(stderr);
			}

			clientsNum++;
			attendantId++;

		}


	} while (1);

	closeServer();
	return(0);

}

void closeServer() {

	printf("\nCerrando servidor.\n");
	fflush(stdout);

	ptrAttendant aux;
	ptrAttendant p = listHead;
	int i = p->id;

	do {

		close(p->clientData.socket);

		aux = p;
		p = p->next;
		DEL(listHead, aux->id);

	} while (p->id != i);

	close(serverSocket);

	exit(0);

}

void *clientManager(void *p) {

	int live = 1;
	ptrAttendant attendant = (ptrAttendant)p;
	static int prb;
	int id = attendant->id;
	Message message;
	char msg[BUFSIZ];

	do {

		if (clientsNum < 1) {
			continue;
		}

		if (recv(attendant->clientData.socket, (void *)msg, BUFSIZ, 0) == -1) {
			perror("\nError recibiendo mensaje del cliente");
		}

		if (strlen(msg) < 2) {
			continue;
		}

		printf("\nMensaje desde el cliente (%s@%s) --> %s", //
			   attendant->clientData.username, attendant->clientData.addr, msg);
		fflush(stdout);

		message.type = readMessageType(msg);
		fixMessageInfo(message.info, msg);

		switch(message.type) {

		case Broadcast:

			broadcast(message.info);
			break;

		case Login:

			if (checkClientExist(message.info)) {

				printf("\nError: El nombre de usuario '%s' ya existe.", message.info);
				fflush(stdout);

				message.type = Error;
				strcpy(msg, ERROR);
				strcat(msg, message.info);

				if (send(attendant->clientData.socket, (void *)msg, BUFSIZ, 0) == -1) {
					perror("\nError enviando mensaje al cliente en Login");
				}

				break;

			}

			strcpy(attendant->clientData.username, message.info);

			message.type = Broadcast;
			strcpy(message.info, "<El usuario '");
			strcat(message.info, attendant->clientData.username);
			strcat(message.info, "' se ha unido.>");

			broadcastExcl(id, message.info);

			message.type = Ready;
			strcpy(message.info, READY);
			strcat(message.info, "<Bienvenido ");
			strcat(message.info, attendant->clientData.username);
			strcat(message.info, ">");

			if (send(attendant->clientData.socket, (void *)message.info, BUFSIZ, 0) == -1) {
				perror("\nError enviando mensaje de bienvenida al cliente en Login");
			}

			break;

		case Logout:

			strcpy(message.info, LOGOUT);
			strcat(message.info, "<Adiós ");
			strcat(message.info, attendant->clientData.username);
			strcat(message.info, ">");

			if (send(attendant->clientData.socket, (void *)message.info, BUFSIZ, 0) == -1) {
				perror("\nError enviando mensaje de despedida al cliente en Logout");
			}

			message.type = Broadcast;
			strcpy(message.info, "<El usuario '");
			strcat(message.info, attendant->clientData.username);
			strcat(message.info, "' ha salido.>");

			close(attendant->clientData.socket);
			deleteClient(id);
			--clientsNum;
			live = 0;

			broadcast(message.info);

			break;

		default:
			break;

		}

		//showClientsList();
		fflush(stdout);

	} while (live);

	prb = id;
	pthread_exit((void *)&prb);

}

void broadcastExcl(int clientSocket, const char *message) {

	if (clientsNum < 2) {
		return;
	}

	char msg[BUFSIZ];
	strcpy(msg, BROADCAST);
	strcat(msg, message);

	ptrAttendant p = listHead;
	int i = p->id;

	do {

		if (p->clientData.socket != clientSocket) {
			if (send(p->clientData.socket, (void *)msg, BUFSIZ, 0) == -1) {
				fprintf(stderr, "\nError enviando mensaje al usuario %s", p->clientData.username);
				fflush(stderr);
			}
		}

		p = p->next;

	} while (p->id != i);

}

void broadcast(const char *message) {

	if (clientsNum < 1) {
		return;
	}

	char msg[BUFSIZ];
	strcpy(msg, BROADCAST);
	strcat(msg, message);

	ptrAttendant p = listHead;
	int i = p->id;

	do {

		if (send(p->clientData.socket, (void *)msg, BUFSIZ, 0) == -1) {
			fprintf(stderr, "\nError enviando mensaje al usuario %s", p->clientData.username);
			fflush(stderr);
		}

		p = p->next;

	} while (p->id != i);

}

int checkClientExist(char *username) {

	if (clientsNum == 0) {
		return 0;
	}

	ptrAttendant p = listHead;
	int i = p->id;

	do {

		if (strcmp(p->clientData.username, username) == 0) {
			return 1;
		}

		p = p->next;

	} while (p->id != i);

	return 0;

}

void deleteClient(int clientId) {

	if (clientsNum == 0) {
		return;
	}

	DEL(listHead, clientId);

}

void showClientsList() {

	if (clientsNum == 0) {
		printf("\nNo hay clientes conectados.\n");
		fflush(stdout);
		return;
	}

	printf("\nLista de clientes conectados:\n");
	fflush(stdout);

	ptrAttendant p = listHead;
	int i = p->id;

	do {

		printf("Cliente %d, username: %s\n", p->id, p->clientData.username);
		fflush(stdout);

		p = p->next;

	} while (p->id != i);

}

void fixMessageInfo(char *dest, const char *src) {

	int i;
	for (i = 0; i < strlen(src); i++) {
		dest[i] = src[i + MSG_TYPE_SIZE];
	}

}

MessageType readMessageType(const char *message) {

	if (!strncmp(message, BROADCAST, MSG_TYPE_SIZE)) {
		return Broadcast;
	}
	if (!strncmp(message, LOGIN, MSG_TYPE_SIZE)) {
		return Login;
	}
	if (!strncmp(message, LOGOUT, MSG_TYPE_SIZE)) {
		return Logout;
	}
	if (!strncmp(message, READY, MSG_TYPE_SIZE)) {
		return Ready;
	}
	if (!strncmp(message, PRIVATE, MSG_TYPE_SIZE)) {
		return Private;
	}

	return Error;

}
