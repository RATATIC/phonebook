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
    int id;
    int sock;
    FILE* fp;
};

// Func takes name operation and call func which does this operation
void client_accept (struct thr_data* data);

// Func add record in phonebook
void add_record (struct thr_data* data);

// when new client  connects create new thread 
void create_thread(struct thr_node** thr_top, struct thr_data* data);

// Func search records in phonebook
void search_record (struct thr_data* data);

// Func cmp two records
int record_cmp (struct record tmp, struct record tmp2);

// read word from file
char* read_word_from_file (FILE** fp);

// create record tmp with data from client
struct record make_record (struct thr_data* data);

// Func writes log in file
void log_m (void* ptr);

// Func creates new log and call log_m
void make_log (char* mess, int id);

void record_null_memset(struct record* tmp);

// Func deletes record from phonebook
void delete_record (struct thr_data* data);

// Func owerwrites phonebook
void delete_from_file (FILE* fp, struct record tmp);