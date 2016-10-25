#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {

	int socket_id = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_id < 0) {
		perror("Error: Socket().\n");
		exit(1);
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	server_addr.sin_port = htons(5100);

	//int server_len = sizeof(server_addr);
	if (connect(socket_id, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
		perror("Error: Connect().\n");
		exit(1);
	}

	if (send(socket_id, "OLA", 3, 0) != 3) {
		perror("Error: Send().\n");
		exit(1);
	}

	close(socket_id);
	exit(0);
}
