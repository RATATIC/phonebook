#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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
// Fucn sends server record and server add record in phonebook
void add_record (int sock);

// Func sends server record
void send_record (int sock, struct record rec);

// Fucn sends server fields and server search records in phonebook
void search_record (int sock);

// Func prints mess about send response and recv request
void print_mess(int conditional);

// Func sends server fields and server delete record in phonebook
void delete_record (int sock);