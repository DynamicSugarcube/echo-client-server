#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "connection.h"

const int PERIOD = 1;
const int REPEATS = 50;

int main(int argc, char *argv[])
{
	int data_socket;
	int res;
	struct sockaddr_un addr;
	char message[] = "Stop right there criminal scum!";
	char response[sizeof(message)];
	
	data_socket = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (data_socket == -1)
	{
		perror("socket");
		exit(1);
	}
	 
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, PATHNAME);
	
	res = connect(data_socket, (struct sockaddr *) &addr,
		sizeof(addr));
	if (res == -1)
	{
		perror("connect");
		exit(2);
	}
	
	for (int i = 0; i < REPEATS; i++)
	{
		send(data_socket, message, sizeof(message), 0);
		recv(data_socket, response, sizeof(message), 0);
		printf("%s\n", response);
		sleep(PERIOD);
	}
	
	close(data_socket);
	
	return 0;
}
