/*
* @file main.c
* @author Renat Kagal <kagal@itspartner.net>
*
* Assembling : gcc -Wall client.c -o client
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

#define SEND_REQUEST 1
#define RECV_RESPONSE 2

#define RECORD_SIZE 98

#define CHAR_SIZE 8

#define MAX_SIZE_FOR_TIME 20

#define RECORD_FIELD_TO_BUFF(buff, field, offset, size) ({ for (int i = 0; i < size; i++, offset++) buff[offset] = field[i];})

#define RECORD_FIELD_FROM_BUFF(buff, field, offset, size) ({ for (int i = 0; i < size; i++, offset++) field[i] = buff[offset];})

#define INT_FROM_BUFFER(buff, offset) (((int)((unsigned char)buff[(offset)])) | ((int)((unsigned char)buff[(offset) + 1]) << 8) | ((int)((unsigned char)buff[(offset) + 2]) << 16) | ((int)((unsigned char)buff[(offset) + 3]) << 24))

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
            search_record (sock);
        }

        if (strcmp (buff, "delete\n") == 0) {           
            delete_record (sock);
        }
        memset (buff, '\0', BUFFER_SIZE);
    }
    close (sock);
}

void print_mess(int conditional) {

    time_t tm = time(NULL);
    struct tm* tm_ptr = localtime (&tm);

    char time_str[MAX_SIZE_FOR_TIME];

    if (strftime (time_str, MAX_SIZE_FOR_TIME, "%Y.%m.%d %H.%M.%S", tm_ptr) == 0) {
        puts ("Failed strftime");
        exit (EXIT_FAILURE);
    }

    if (conditional == SEND_REQUEST) {
        printf ("[%s] send request\n", time_str);
    }
    else {
        printf ("[%s] recv respense\n", time_str);
    }
}

void delete_record (int sock) {
    char buff[BUFFER_SIZE] = "delete";

    if (send (sock, buff, strlen (buff), 0) < 0) {
        puts ("Failed send");
        exit (EXIT_FAILURE);
    }
    memset (buff, '\0', BUFFER_SIZE);

    while (fgets (buff, BUFFER_SIZE - 1, stdin)) {
        if (strcmp (buff, "end\n") == 0) {
            if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
                puts ("Failed send");
                exit (EXIT_FAILURE);
            }
            break;
        }

        if (send (sock, buff, BUFFER_SIZE, 0) < 0) {
            puts ("Failed send");
            exit (EXIT_FAILURE);
        }
        memset (buff, '\0', BUFFER_SIZE);
    }
    print_mess (SEND_REQUEST);

    memset (buff, '\0', BUFFER_SIZE);
    if (recv (sock, buff, BUFFER_SIZE, 0) < 0) {
        puts ("Failed recv");
        exit (EXIT_FAILURE);
    }
    print_mess (RECV_RESPONSE);
}

void search_record (int sock) {
    char buff[BUFFER_SIZE] = "search";

    if (send (sock, buff, strlen (buff), 0) < 0) {
        puts ("Failed send");
        exit (EXIT_FAILURE);
    }
    memset (buff, '\0', BUFFER_SIZE);

    while (fgets (buff, BUFFER_SIZE - 1, stdin)) {
        if (strcmp (buff, "end\n") == 0) {
            if (send (sock, buff, strlen (buff), 0) < 0) {
                puts ("Failed send");
                exit (EXIT_FAILURE);
            }
            break;
        }
        if (send (sock, buff, strlen (buff), 0) < 0) {
            puts ("Failed send");
            exit (EXIT_FAILURE);
        }
        memset (buff, '\0', BUFFER_SIZE);
    }
    print_mess (SEND_REQUEST);

    int offset = 0;
    char record_buff[RECORD_SIZE];

    int size_found_record = 0;
    struct record* found_records = (struct record*)malloc (sizeof (struct record));
    if (found_records == NULL) {
        puts ("Failed malloc");
        exit (EXIT_FAILURE);
    }

    while (1) {
        offset = 0;

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

        RECORD_FIELD_FROM_BUFF (record_buff, found_records[size_found_record].second_name, offset, FIELD_SIZE);
        RECORD_FIELD_FROM_BUFF (record_buff, found_records[size_found_record].name, offset, FIELD_SIZE);
        RECORD_FIELD_FROM_BUFF (record_buff, found_records[size_found_record].patronymic, offset, FIELD_SIZE);
        RECORD_FIELD_FROM_BUFF (record_buff, found_records[size_found_record].country, offset, FIELD_SIZE);
        RECORD_FIELD_FROM_BUFF (record_buff, found_records[size_found_record].city, offset, FIELD_SIZE);
        RECORD_FIELD_FROM_BUFF (record_buff, found_records[size_found_record].street, offset, FIELD_SIZE);

        found_records[size_found_record].house = INT_FROM_BUFFER (record_buff, offset);
        offset += sizeof (int);
        found_records[size_found_record].flat = INT_FROM_BUFFER (record_buff, offset);
        
        size_found_record++;
    }
    print_mess (RECV_RESPONSE);
    
    for (int i = 0; i < size_found_record; i++) {
        printf ("%s %s %s %s %s %s %d %d\n", found_records[i].second_name, found_records[i].name, found_records[i].patronymic, found_records[i].country, found_records[i].city, found_records[i].street, found_records[i].house,found_records[i].flat);
    }
}

void add_record (int sock) {
    char buff[BUFFER_SIZE] = "add";

    if (send (sock, buff, strlen (buff), 0) < 0) {
        puts ("Failed send add_record");
        exit (EXIT_FAILURE);
    }
    struct record rec;

    scanf ("%s%s%s%s%s%s%d%d", rec.second_name, rec.name, rec.patronymic, rec.country, rec.city, rec.street, &rec.house, &rec.flat);

    send_record (sock, rec);

    memset (buff, '\0', BUFFER_SIZE);
    if (recv (sock, buff, BUFFER_SIZE, 0) < 0) {
        puts ("Failed recv");
        exit (EXIT_FAILURE);
    }
    print_mess (RECV_RESPONSE);
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
    print_mess (SEND_REQUEST);
}