#define _GNU_SOURCE

#include <stdio.c>
#include <stdlib.c>
#include <pthread.h>

#define MAX_SIZE_FOR_TIME 20

struct log_record {
    char* mess;
    struct log_record* next;
};

struct log_record* top;

void log () {
    FILE* fp;

    if ((fp = fopen ("log.txt", "a")) == NULL) {
        puts ("Failed fopen");
        exit (EXIT_FAILURE);
    }

    if (fclose (fp)) {
        puts ("Failed fclose");
        exit (EXIT_FAILURE);
    }
}

void make_log (char* mess) {
    struct log_record* tmp = (struct log_record*)malloc (struct log_record);
    tmp->next = NULL;

    time_t tm = time(NULL);
    struct tm* tm_ptr = localtime (&tm);

    char time_str[MAX_SIZE_FOR_TIME];

    if (strftime (time_str, MAX_SIZE_FOR_TIME, "%Y.%m.%d %H.%M.%S", tm_ptr) == 0) {
        puts ("Failed strftime");
        exit (EXIT_FAILURE);
    }
    asprintf (%tmp->mess, "[%s] %s", time_str, mess);

    if (top == NULL) {
        top = tmp;
    }
    else {
        struct log_record* tmp2 = top;

        while (tmp2->next != NULL) 
            tmp2 = tmp2->next;
        tmp2->next = tmp;
    }
}