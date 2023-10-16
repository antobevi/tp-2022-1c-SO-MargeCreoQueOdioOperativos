#include "cliente.h"

void mandar_credencial_al_patova(int socket) {
	char* credencial = "holo";

	send(socket, credencial, strlen("holo")+1, 0);
}

int conectar_con_servidor(char* ip, int puerto) {
	char* server_name = ip;
	int server_port, socket_fd;
	struct hostent *server_host;
	struct sockaddr_in server_address;

	/* Get server name from command line arguments or stdin. */

	/* Get server port from command line arguments or stdin. */

	server_port = puerto;

	/* Get server host from server name. */
	server_host = gethostbyname(server_name);

	/* Initialise IPv4 server address with server host. */

	memset(&server_address, 0, sizeof server_address);
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(server_port);
	memcpy(&server_address.sin_addr.s_addr, server_host->h_addr, server_host->h_length);

	/* Create TCP socket. */

	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd == -1) {
		perror("Error en la creacion del socket");

		return -1;
	}

	/* Connect to socket with server address. */

	if (connect(socket_fd, (struct sockaddr *)&server_address, sizeof server_address) == -1) {
		perror("Error en la conexion del socket! No se pudo conectar al servidor.");

		return -1;
	}

	mandar_credencial_al_patova(socket_fd);

	return socket_fd;
}


void liberar_conexion(int socket_cliente) {
	close(socket_cliente);
}
