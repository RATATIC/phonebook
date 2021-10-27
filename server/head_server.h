#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#define FIELD_SIZE 15

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

void add_record (struct thr_data* data);

void create_thread(struct thr_node** thr_top, struct thr_data* data);

void search_record (struct thr_data* data);

int record_cmp (struct record tmp, struct record tmp2, int field_count);

char* read_word_from_file (FILE** fp); 