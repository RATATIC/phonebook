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

#include "head_server.h"

#define PORT 1321
#define READING_SIZE 1024

int main () {
    FILE* fp;
    if ((fp = fopen ("phonebook.txt", "r+")) == NULL) {
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
        char buff [READING_SIZE];

        while (1) {
            if (recv (data->sock, buff, READING_SIZE, 0) < 0) {
                puts ("Failed recv message");
                exit (EXIT_FAILURE);
            }   
            printf("%s\n", buff);
            memset (buff, '\0', READING_SIZE);
        }
        close (data->sock);
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