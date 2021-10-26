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
#define _GNU_SOURCE

#include "head_client.h"

#define PORT 13230
#define BUFFER_SIZE 1024

#define RECORD_SIZE 98

#define CHAR_SIZE 8

#define RECORD_FIELD_TO_BUFF(buff, field, offset, size) ({ for (int i = 0; i < size; i++, offset++) buff[offset] = field[i];})

#define RECORD_FIELD_FROM_BUFF(buff, field, offset, size) ({ for (int i = 0; i < size; i++, offset++) field[i] = buff[offset];})

void print_bits (int n, int size) {
    char* bits = "";

    for (int i = 0; i < size; i++) {
        asprintf (&bits, "%d%s", (n & 1), bits);
        n = n >> 1;
    }
    printf ("%s\n", bits);
}

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
	char buff[BUFFER_SIZE];
	memset (buff, ' ', BUFFER_SIZE);

	while (fgets (buff, BUFFER_SIZE - 1, stdin)) {
		if (strcmp (buff, "stop\n") == 0) {
			break;
		}

		if (strcmp (buff, "add\n") == 0) {
			add_record (sock);
		}

		if (strcmp (buff, "search\n") == 0) {
			serch_record (sock);
		}
		memset (buff, '\0', BUFFER_SIZE);
	}
	close (sock);
}

void search_record (int sock) {
	char buff[BUFFER_SIZE];

	fgets (buff, BUFFER_SIZE - 1, stdin);

	if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
		puts ("Failed send");
		exit (EXIT_FAILURE);
	}

	int offset = 0;
	char record_buff[RECORD_SIZE];

	int size_found_record = 0;
	struct record* found_records = (struct record*)malloc (sizeof (struct record));
	if (found_records == NULL) {
		puts ("Failed malloc");
		exit (EXIT_FAILURE);
	}

	while (1) {
		if (recv (sock, record_buff, RECORD_SIZE, 0) < 0) {
			puts ("Failed recv");
			exit (EXIT_FAILURE);
		}

		if (record_buff[0] == '0') {
			break;
		}

		if (size_found_record > 0) {
			found_records = realloc (found_records, sizeof (struct record) * (size_found_record + 1));
			if (found_records == NULL) {
				puts ("Failed realloc");
				exit (EXIT_FAILURE);
			}
		}
		RECORD_FIELD_FROM_BUFF (record_buff, offset, found_records[size_found_record].second_name);
		RECORD_FIELD_FROM_BUFF (record_buff, offset, found_records[size_found_record].name);
		RECORD_FIELD_FROM_BUFF (record_buff, offset, found_records[size_found_record].patronymic);
		RECORD_FIELD_FROM_BUFF (record_buff, offset, found_records[size_found_record].country);
		RECORD_FIELD_FROM_BUFF (record_buff, offset, found_records[size_found_record].city);
		RECORD_FIELD_FROM_BUFF (record_buff, offset, found_records[size_found_record].street);

		 for (int i = 0; i < sizeof (int); i++, offset++) {
        rec.house += (int)(buff[offset] << (i * CHAR_SIZE));
   		}

    	for (int i = 0; i < sizeof (int); i++, offset++) {
        	rec.flat += (int)(buff[offset] << (i * CHAR_SIZE));
    	}
    }

    for (int i = 0; i < size_found_record; i++) {
    	printf ("%s %s %s %s %s %s %d %d\n", found_records[i].second_name, found_records[i].name, found_records[i].patronymic, found_records[i].country, found_records[i].city, found_records[i].street, found_records[i].house,found_records[i].flat);
    }
}

void add_record (int sock) {
	char buff[BUFFER_SIZE] = "add"; ////////////////// less memory

	if (send (sock, buff, strlen (buff), 0) < 0) {
		puts ("Failed send add_record");
		exit (EXIT_FAILURE);
	}
	struct record rec;

	scanf ("%s%s%s%s%s%s%d%d", rec.second_name, rec.name, rec.patronymic, rec.country, rec.city, rec.street, &rec.house, &rec.flat);

	send_record (sock, rec);
}

void send_record (int sock, struct record rec) {
	char record_buff [RECORD_SIZE];
	int offset = 0;

	RECORD_FIELD_TO_BUFF(record_buff, rec.second_name, offset, FIELD_SIZE);
	RECORD_FIELD_TO_BUFF(record_buff, rec.name, offset, FIELD_SIZE);
	RECORD_FIELD_TO_BUFF(record_buff, rec.patronymic, offset, FIELD_SIZE);
	RECORD_FIELD_TO_BUFF(record_buff, rec.country, offset, FIELD_SIZE);
	RECORD_FIELD_TO_BUFF(record_buff, rec.city, offset, FIELD_SIZE);
	RECORD_FIELD_TO_BUFF(record_buff, rec.street, offset, FIELD_SIZE);

	for (int i = 0; i < sizeof (int); i++, offset++){
		record_buff[offset] = (0xff & (rec.house >> (i * CHAR_SIZE)));
	}
	for (int i = 0; i < sizeof (int); i++, offset++){
		record_buff[offset] = (0xff & (rec.flat >> (i * CHAR_SIZE)));
	}

	if (send (sock, record_buff, RECORD_SIZE, 0) < 0) {
		puts ("Failed send buff");
		exit (EXIT_FAILURE);
	}
}