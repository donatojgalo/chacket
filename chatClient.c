#include "globals.h"

void *receiveMessage(void *);
void *sendMessage(void *);
void fixMessageInfo(char *, const char *);
MessageType readMessageType(const char *);

int main() {

	pthread_t threads[2];
	int serverSocket;
	struct sockaddr_in addr;
	char message[BUFSIZ], tmp[BUFSIZ];

	bzero(&(addr), sizeof(struct sockaddr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(SERVER_PORT);
	addr.sin_addr.s_addr = inet_addr(SERVER_ADDRESS);

	if ((serverSocket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("Error abriendo socket");
		exit(1);
	}

	if (connect(serverSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		perror("Error en el conect");
		exit(1);
	}

	printf("Ingrese su nombre de usuario: ");
	fgets(tmp, BUFSIZ - 6, stdin);
	strcpy(message, LOGIN);
	strcat(message, tmp);
	message[strlen(message) - 1] = '\0';
	fflush(stdout);

	do {

		if (readMessageType(message) == Login) {
			if (send(serverSocket, (void *)message, BUFSIZ, 0) == -1) {
				perror("\nError enviando mensaje al servidor en Login");
			}
		}

		if (recv(serverSocket, (void *)message, BUFSIZ, 0) == -1) {
			perror("\nError recibiendo mensaje del servidor en login");
		}

		if (readMessageType(message) == Error) {

			printf("\nSeleccione otro nombre de usuario: ");
			fgets(tmp, BUFSIZ - 6, stdin);
			strcpy(message, LOGIN);
			strcat(message, tmp);
			message[strlen(message) - 1] = '\0';
			fflush(stdout);

		}

	} while (readMessageType(message) != Ready);

	printf("***A crear los hilos***\nserverSocket = %d\n", serverSocket);
	fflush(stdout);

	if (pthread_create(&threads[1], 0, receiveMessage, (void *)&serverSocket) != 0) {
		perror("Error creando el hilo para recibir.");
		close(serverSocket);
		exit(1);
	}

	if (pthread_create(&threads[0], 0, sendMessage, (void *)&serverSocket) != 0) {
		perror("Error creando el hilo para enviar.");
		close(serverSocket);
		exit(1);
	}

	pthread_join(threads[0], 0);
	pthread_join(threads[1], 0);

	close(serverSocket);

	return(0);

}

void *receiveMessage(void *p) {

	char message[BUFSIZ];
	char tmp[BUFSIZ];
	static int prb;
	int *id = (int *)p;

	do {

		if (recv(*id, (void *)message, BUFSIZ, 0) == -1) {
			perror("\nError recibiendo mensaje del servidor");
		}

		if (strlen(message) < 7) {
			continue;
		}

		fixMessageInfo(tmp, message);
		printf("\nMensaje desde el servidor --> %s\nMensaje: ", tmp);
		fflush(stdout);

	} while (readMessageType(message) != Logout);

	prb = *id;
	pthread_exit((void *)&prb);

}

void *sendMessage(void *p) {

	char message[BUFSIZ], tmp[BUFSIZ];
	static int prb;
	int *id = (int *)p;

	do {

		printf("Mensaje: ");
		fgets(tmp, BUFSIZ - 6, stdin);
		fflush(stdout);

		if (strlen(tmp) < 2) {
			continue;
		}

		strcpy(message, !strcmp(tmp, QUIT) ? LOGOUT : BROADCAST);
		strcat(message, tmp);
		message[strlen(message) - 1] = '\0';
		fflush(stdout);

		if (send(*id, (void *)message, BUFSIZ, 0) == -1) {
			perror("\nError enviando mensaje al servidor");
		}

	} while (readMessageType(message) != Logout);

	prb = *id;
	pthread_exit((void *)&prb);

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
