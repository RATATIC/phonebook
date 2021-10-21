/*
* @file main.c
* @author Renat Kagal <kagal@itspartner.net>
*
* Assembling : gcc -Wall main.c -o main
*
* Description : client in phone book
*
* Copyright (c) 2021, ITS Partner LLC.
* All rights reserved.
*
* This software is the confidential and proprietary information of
* ITS Partner LLC. ("Confidential Information"). You shall not
* disclose such Confidential Information and shall use it only in
* accordance with the terms of the license agreement you entered into
* with ITS Partner.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 1321
#define MESSAGE_SIZE 1024

int main () {
	int sock;
	struct sockaddr_in  addr;

	if ((sock = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
		puts ("Failed create socket");
		exit (EXIT_FAILURE);
	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons (PORT);
	addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

	if (connect (sock, (struct sockaddr*) &addr, sizeof (addr)) < 0) {
		puts ("Failed connection to server");
		exit (EXIT_FAILURE);
	}
	char message[MESSAGE_SIZE];

	memset (message, ' ', MESSAGE_SIZE);
	fflush (stdin);

	while (fgets (message, MESSAGE_SIZE - 1, stdin)) {
		if (strcmp (message, "stop\n") == 0) {
			break;
		}
		if (send (sock, message, strlen (message), 0) < 0) {
			puts ("Failed send message on server");
			exit (EXIT_FAILURE);
		}
		memset (message, '\0', MESSAGE_SIZE);
	}
	close (sock);
}