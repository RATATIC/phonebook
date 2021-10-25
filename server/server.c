/*
* @file main.c
* @author Renat Kagal <kagal@itspartner.net>
*
* Assembling : gcc -Wall main.c -o main
*
* Description : server phone book
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

#include "head_server.h"

#define PORT 13230
#define BUFFER_SIZE 1024

#define FIELD_SIZE 15
#define RECORD_SIZE 98

#define CHAR_SIZE 8

#define RECORD_FIELD_FROM_BUFF(buff, field, offset, size) ({ for (int i = 0; i < size; i++, offset++) field[i] = buff[offset];})

#define INT_FROM_BUFFER(buff, offset) (((unsigned int)buff[(offset)] << 0) | ((unsigned int)buff[(offset) + 1] << 8) | ((unsigned int)buff[(offset) + 2] << 16) | ((unsigned int)buff[(offset) + 3] << 24))

#define GET_INT16(buff, offset) ((uint16t)buff[(offset)] | ((uint16t)buff[(offset) + 1] << 8))


struct record {
    char second_name [FIELD_SIZE];
    char name [FIELD_SIZE];
    char patronymic [FIELD_SIZE];

    char country [FIELD_SIZE];
    char city [FIELD_SIZE];
    char street [FIELD_SIZE];
    int house;
    int flat; 
};

void print_bits (int n, int size) {
    char* bits = "";

    for (int i = 0; i < size; i++) {
        asprintf (&bits, "%d%s", (n & 1), bits);
        n = n >> 1;
    }
    printf ("%s\n", bits);
}

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

int main () {
    FILE* fp;
    if ((fp = fopen ("phonebook.txt", "a")) == NULL) {
        puts ("Failed open file");
        exit (EXIT_FAILURE);
    }

    int listen_sock, sock;
    struct sockaddr_in addr;

    if ((listen_sock = socket (AF_INET, SOCK_STREAM, 0)) < 0) {
        puts ("Failed create socket");
        exit (EXIT_FAILURE);
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons (PORT);
    addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);

    if (bind (listen_sock, (struct sockaddr*) &addr, sizeof (addr)) < 0) {
        puts ("Failed bind sock");
        exit (EXIT_FAILURE);
    }
    listen (listen_sock, 1);
    struct thr_node* thr_top = NULL;
    struct thr_data* data = NULL;

    while (1) {
        if ((sock = accept (listen_sock, NULL, NULL)) < 0) {
            puts ("Failed accept connection");
            exit (EXIT_FAILURE);
        }
        data = (struct thr_data*)malloc (sizeof (struct thr_data));

        if (data == NULL) {
            puts ("Failed alloc memory data");
            exit (EXIT_FAILURE);
        }
        data->sock = sock;
        data->fp = fp;

        create_thread (&thr_top, data);
    }
    close (listen_sock);

    if (fclose (fp)) {
        puts ("Failed close file");
        exit (EXIT_FAILURE);
    }
}

void client_accept (struct thr_data* data) {
        char buff [BUFFER_SIZE];
       
        while (1) {
            if (recv (data->sock, buff, BUFFER_SIZE, 0) <= 0) {
                break;
            }

            if (strcmp (buff, "add") == 0) {
                add_record (data);
            }
            if (strcmp (buff, "search") == 0) {
                search_record (data);
            }

            memset (buff, '\0', BUFFER_SIZE);
        }
        close (data->sock);
}

void search_record (struct thr_data* data) {
    char buff[BUFFER_SIZE];

    if (recv (data->sock, buff, BUFFER_SIZE, 0) <= 0) {
        puts ("Failed recv");
        exit (EXIT_FAILURE);
    }
}

void add_record (struct thr_data* data) {
    char buff [RECORD_SIZE];                                                                        ///////////////////////
    int offset = 0;

    if (recv (data->sock, buff, BUFFER_SIZE, 0) <= 0) {
        puts ("Failed recv add_record");
        exit (EXIT_FAILURE);
    }

    struct record rec;
    rec.house = 0;
    rec.flat = 0;

    RECORD_FIELD_FROM_BUFF(buff, rec.second_name, offset, FIELD_SIZE);
    RECORD_FIELD_FROM_BUFF(buff, rec.name, offset, FIELD_SIZE);
    RECORD_FIELD_FROM_BUFF(buff, rec.patronymic, offset, FIELD_SIZE);
    RECORD_FIELD_FROM_BUFF(buff, rec.country, offset, FIELD_SIZE);
    RECORD_FIELD_FROM_BUFF(buff, rec.city, offset, FIELD_SIZE);
    RECORD_FIELD_FROM_BUFF(buff, rec.street, offset, FIELD_SIZE);

    for (int i = 0; i < sizeof (int); i++, offset++) {
        rec.house += (int)(buff[offset] << (i * CHAR_SIZE));
        print_bits (rec.house, 32);
        print_bits (buff[offset], 8);
    }

    for (int i = 0; i < sizeof (int); i++, offset++) {
        rec.flat += (int)(buff[offset] << (i * CHAR_SIZE));
    }

    printf ("%s %s %s %s %s %s %d %d\n", rec.second_name, rec.name, rec.patronymic, rec.country, rec.city, rec.street, rec.house, rec.flat);
    fflush (stdin);

    if (pthread_mutex_lock (&mtx)) {
        puts ("Failed lock mtx");
        exit (EXIT_FAILURE);
    }

    fprintf (data->fp, "%s %s %s %s %s %s %d %d\n", rec.second_name, rec.name, rec.patronymic, rec.country, rec.city, rec.street, rec.house, rec.flat);
    fflush (data->fp);

    if (pthread_mutex_unlock (&mtx)) {
        puts ("Failed unlock mtx");
        exit (EXIT_FAILURE);
    }
}

void create_thread(struct thr_node** thr_top, struct thr_data* data) {
    if (*thr_top == NULL) {
        (*thr_top) = (struct thr_node*)malloc (sizeof (struct thr_node));
        
        if ((*thr_top) == NULL) {
            puts ("Failed alloc memory for thr_top");
            exit (EXIT_FAILURE);
        }
        (*thr_top)->id = 1;
        (*thr_top)->next = NULL;

        if (pthread_create (&((*thr_top)->thread), NULL, client_accept, data)) {
            puts ("Failed create thread top");
            exit (EXIT_FAILURE);
        }
    }
    else {
        struct thr_node* tmp = (struct thr_node*)malloc (sizeof (struct thr_node));
        
        if (tmp == NULL) {
            puts ("Failed alloc memory for tmp_thread");
            exit (EXIT_FAILURE);
        }
        struct thr_node* tmp2 = (*thr_top);

        while (tmp2->next != NULL)
             tmp2 = tmp2->next;
        tmp2->next = tmp;

        tmp->id = tmp2->id + 1;
        tmp->next = NULL;
        
        if (pthread_create (&(tmp->thread), NULL, client_accept, data)) {
            puts ("Failed create thread");
            exit (EXIT_FAILURE);
        }
    }
}