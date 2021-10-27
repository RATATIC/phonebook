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

#define RECORD_SIZE 98

#define CHAR_SIZE 8
#define SEARCH_RECORD_FIELD_SIZE 30

#define RECORD_FIELD_FROM_BUFF(buff, field, offset, size) ({ for (int i = 0; i < size; i++, offset++) field[i] = buff[offset];})

#define RECORD_FIELD_TO_BUFF(buff, field, offset, size) ({ for (int i = 0; i < size; i++, offset++) buff[offset] = field[i];})

#define INT_FROM_BUFFER(buff, offset) (((int)buff[(offset)] << 0) | ((int)buff[(offset) + 1] << 8) | ((int)buff[(offset) + 2] << 16) | ((int)buff[(offset) + 3] << 24))

#define GET_INT16(buff, offset) ((uint16t)buff[(offset)] | ((uint16t)buff[(offset) + 1] << 8))

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
    if ((fp = fopen ("phonebook.txt", "a+")) == NULL) {
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

    int size_searching_records = 0;
    char** searching_records = (char**)malloc (sizeof (char*));

    while (1) {
        if (recv (data->sock, buff, BUFFER_SIZE, 0) <= 0) {
            puts ("Failed recv");
            exit (EXIT_FAILURE);
        }
        if (strcmp (buff, "end\n") == 0) {
            break;
        }
        if (size_searching_records > 0) {
            searching_records = realloc (searching_records, sizeof (char*) * (size_searching_records + 1));
            if (searching_records == NULL) {
                puts ("Failed realloc");
                exit (EXIT_FAILURE);
            }
        }
        searching_records[size_searching_records] = (char*)malloc (sizeof (char) * (strlen (buff) + 1));
        strncat (searching_records[size_searching_records], buff, strlen (buff) - 1);
        memset (buff, '\0', BUFFER_SIZE);
        size_searching_records++;
    }

    struct record tmp;
    char field_name[FIELD_SIZE];
    char number[CHAR_SIZE];
    char* ptr = NULL;

    for (int i = 0; i < size_searching_records; i++) {
        ptr = strchr (searching_records[i], ' ');
        strncat (field_name, searching_records[i], ptr - searching_records[i]);
        
        if (strcmp (field_name, "second_name") == 0) {
            strcat (tmp.second_name, ptr + 1);
        }
        else if (strcmp (field_name, "name") == 0) {
            strcat (tmp.name, ptr + 1);
        }
        else if (strcmp (field_name, "patronymic") == 0) {
            strcat (tmp.patronymic, ptr + 1);
        }
        else if (strcmp (field_name, "country") == 0) {
            strcat (tmp.country, ptr + 1);
        }
        else if (strcmp (field_name, "city") == 0) {
            strcat (tmp.city, ptr + 1);
        }
        else if (strcmp (field_name, "street") == 0) {
            strcat (tmp.street, ptr + 1);
        } 
        else if (strcmp (field_name, "house") == 0) {
            strcat (number, ptr + 1);
            tmp.house = atoi (number);
            memset (number, '\0', CHAR_SIZE);
        }
        else if (strcmp (field_name, "flat") == 0) {
            strcat (number, ptr + 1);
            tmp.flat = atoi (number);
            memset (number, '\0', CHAR_SIZE);
        }
        memset (field_name, '\0', FIELD_SIZE);
    }
    puts (tmp.name);

    if (pthread_mutex_lock (&mtx)) {
        puts ("Failed lock mutex");
        exit (EXIT_FAILURE);
    }
    if (fseek (data->fp, 0, SEEK_SET)) {
        puts ("Failed fseek");
        exit (EXIT_FAILURE);
    }
    struct record tmp2;

    int size_found_records = 0;

    struct record* found_records = (struct record*)malloc  (sizeof (struct record));
    if (found_records == NULL) {
        puts ("Failed malloc");
        exit (EXIT_FAILURE);
    }
    char* str;

    while (!feof (data->fp)) {
        str = read_word_from_file (&data->fp);
        memset (tmp2.second_name, '\0', FIELD_SIZE);
        strcat (tmp2.second_name, str);
        free (str);
        str = read_word_from_file (&data->fp);
        memset (tmp2.name, '\0', FIELD_SIZE);
        strcat (tmp2.name, str);
        free (str);
        str = read_word_from_file (&data->fp);
        memset (tmp2.patronymic, '\0', FIELD_SIZE);
        strcat (tmp2.patronymic, str);
        free (str);
        str = read_word_from_file (&data->fp);
        memset (tmp2.country, '\0', FIELD_SIZE);
        strcat (tmp2.country, str);
        free (str);
        str = read_word_from_file (&data->fp);
        memset (tmp2.city, '\0', FIELD_SIZE);
        strcat (tmp2.city, str);
        free (str);
        str = read_word_from_file (&data->fp);
        memset (tmp2.street, '\0', FIELD_SIZE);
        strcat (tmp2.street, str);
        free (str);
        str = read_word_from_file (&data->fp);
        tmp2.house = atoi (str);
        free (str);
        str = read_word_from_file (&data->fp);
        tmp2.flat = atoi (str);
        free (str);

        if (record_cmp (tmp, tmp2, size_searching_records) == 0) {
            if (size_found_records > 0) {
                found_records = realloc (found_records, sizeof (struct record) * (size_found_records + 1));
                
                if (found_records == NULL) {
                    puts ("Failed realloc");
                    exit (EXIT_FAILURE);
                }
            }
            strcat (found_records[size_found_records].second_name ,tmp2.second_name);
            strcat (found_records[size_found_records].name, tmp2.name);
            strcat (found_records[size_found_records].patronymic, tmp2.patronymic);
            strcat (found_records[size_found_records].country, tmp2.country);
            strcat (found_records[size_found_records].city, tmp2.city);
            strcat (found_records[size_found_records].street, tmp2.street);
            found_records[size_found_records].house = tmp2.house;
            found_records[size_found_records].flat = tmp2.flat;
            
            printf ("%s %s %s %s %s %s %d %d\n", found_records[size_found_records].second_name, found_records[size_found_records].name, found_records[size_found_records].patronymic, found_records[size_found_records].country, found_records[size_found_records].city, found_records[size_found_records].street, found_records[size_found_records].house, found_records[size_found_records].flat);

            size_found_records++;
        }
    }
    if (pthread_mutex_unlock (&mtx)) {
        puts ("Failed unlocc mutex");
        exit (EXIT_FAILURE);
    }
    for (int i = 0; i < size_found_records; i++) {
        puts ("dasdadasdasdasdsda");
        printf ("%s %s %s %s %s %s %d %d", found_records[i].second_name, found_records[i].name, found_records[i].patronymic, found_records[i].country, found_records[i].city, found_records[i].street, found_records[i].house, found_records[i].flat);
    }

    int offset = 0;
    char record_buff [RECORD_SIZE];

    for (int i = 0; i < size_found_records; i++) {
        offset = 0;
        RECORD_FIELD_TO_BUFF (buff, found_records[i].second_name, offset, FIELD_SIZE);
        RECORD_FIELD_TO_BUFF (buff, found_records[i].name, offset, FIELD_SIZE);
        RECORD_FIELD_TO_BUFF (buff, found_records[i].patronymic, offset, FIELD_SIZE);
        RECORD_FIELD_TO_BUFF (buff, found_records[i].country, offset, FIELD_SIZE);
        RECORD_FIELD_TO_BUFF (buff, found_records[i].city, offset, FIELD_SIZE);
        RECORD_FIELD_TO_BUFF (buff, found_records[i].street, offset, FIELD_SIZE);

        for (int i = 0; i < sizeof (int); i++, offset++) {
            record_buff[offset] = (0xff & (found_records[i].house >> (i * CHAR_SIZE)));
        }
        for (int i = 0; i < sizeof (int); i++, offset++) {
            record_buff[offset] = (0xff & (found_records[i].flat >> (i * CHAR_SIZE)));
        }

        if (send (data->sock, record_buff, RECORD_SIZE, 0) < 0) {
            puts ("Failed send");
            exit (EXIT_FAILURE);
        }
    }
    record_buff[0] = '0';

    if (send (data->sock, record_buff, RECORD_SIZE, 0) < 0) {
        puts ("Failed send");
        exit (EXIT_FAILURE);
    }
}

char* read_word_from_file (FILE** fp) {
    char c;
    char* str = "\0";

    c = fgetc (*fp);
    asprintf (&str, "%c", c);

    while (1) {
        c = fgetc (*fp);
        if (c == '\n' || c == ' ' || c == EOF) {
            break;
        }
        asprintf (&str, "%s%c", str, c);
    }

    return strdup (str);
}

int record_cmp (struct record tmp, struct record tmp2, int field_count) {
    int count = 0;

    if (strcmp (tmp.second_name, tmp2.second_name) == 0) {
        count++;
    }
    if (strcmp (tmp.name, tmp2.name) == 0) {
        count++;
    }
    if (strcmp (tmp.patronymic, tmp2.patronymic) == 0) {
        count++;
    }
    if (strcmp (tmp.country, tmp2.country) == 0) {
        count++;
    }
    if (strcmp (tmp.city, tmp2.city) == 0) {
        count++;
    }
    if (strcmp (tmp.street, tmp2.street) == 0) {
        count++;
    }
    if (tmp.house == tmp2.house) {
        count++;
    }
    if (tmp.flat == tmp2.flat) {
        count++;
    }
    if (count == field_count) {
        return 0;
    }
    return 1;
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
        //print_bits (rec.house, 32);
        //print_bits (buff[offset], 8);
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