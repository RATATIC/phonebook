#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

struct thr_node {
    int id;
    pthread_t thread;
    struct thr_node* next;
};

struct thr_data {
    int sock;
    FILE* fp;
};

void client_accept (struct thr_data* data);

void create_thread(struct thr_node** thr_top, struct thr_data* data);