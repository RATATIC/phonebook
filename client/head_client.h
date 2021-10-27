#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void add_record (int sock);

void send_record (int sock, struct record rec);

void search_record (int sock);