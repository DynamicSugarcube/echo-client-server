#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <set>
#include <algorithm>
#include "connection.h"

/* Функция select была заменена на pselect */

using namespace std;

const int NCLIENTS = 10;
const int NBYTES   = 1024;

int main()
{	
	/* Установка обрабатываемых сигналов */
	sigset_t sig_set;
	sigemptyset(&sig_set);
	sigaddset(&sig_set, SIGCONT);
	sigaddset(&sig_set, SIGTSTP);
	sigaddset(&sig_set, SIGTTIN);
	sigaddset(&sig_set, SIGTTOU);
	sigaddset(&sig_set, SIGSTOP);
	sigaddset(&sig_set, SIGKILL);
	sigaddset(&sig_set, SIGTERM);
	sigprocmask(SIG_UNBLOCK, &sig_set, NULL);
	
	int listener, data_socket;
	int res;
	struct sockaddr_un addr;
	char buffer[NBYTES];
	
	fd_set readset;
	set<int> clients;
	
	unlink(PATHNAME);
	
	listener = socket(AF_UNIX, SOCK_SEQPACKET, 0);
	if (listener == -1)
	{
		perror("socket");
		exit(1);
	}
	
	fcntl(listener, F_SETFL, O_NONBLOCK);
	
	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, PATHNAME);
	
	res = bind(listener, (struct sockaddr *) &addr, sizeof(addr));
	if (res == -1)
	{
		perror("bind");
		exit(2);
	}
	
	res = listen(listener, NCLIENTS);
	if (res == -1)
	{
		perror("listen");
		exit(3);
	}
	
	for(;;)
	{
		FD_ZERO(&readset);
		FD_SET(listener, &readset);
		
		for (auto it = clients.begin(); it != clients.end(); it++)
			FD_SET(*it, &readset);
		
		int mx = max(listener, *max_element(clients.begin(),
			clients.end()));
		res = pselect(mx + 1, &readset, NULL, NULL, NULL, &sig_set);
		if (res == -1)
		{
			perror("select");
			exit(4);
		}
		
		if (FD_ISSET(listener, &readset))
		{
			data_socket = accept(listener, NULL, NULL);
			if (data_socket == -1)
			{
				perror("accept");
				exit(4);
			}
			fcntl(data_socket, F_SETFL, O_NONBLOCK);
			
			clients.insert(data_socket);
		}
		
		for(auto it = clients.begin(); it != clients.end(); it++)
		{
			if (FD_ISSET(*it, &readset))
			{
				res = recv(data_socket, buffer, NBYTES, 0);
				if (res <= 0)
				{
					close(*it);
					clients.erase(*it);
					continue;
				}
				send(data_socket, buffer, sizeof(buffer), 0);
			}
		}
	}
	
	return 0;
}
