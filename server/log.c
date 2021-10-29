#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define MAX_SIZE_FOR_TIME 20

struct log_record {
    char* mess;
    struct log_record* next;
};

struct log_record* top = NULL;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void log_m (void* ptr) {
    FILE* fp;
    struct log_record* tmp = NULL;

    if ((fp = fopen ("log.txt", "a")) == NULL) {
        puts ("Failed fopen");
        exit (EXIT_FAILURE);
    }

    while (1) {
        if (pthread_mutex_lock (&mutex)) {
            puts ("Failed lock mutex");
            exit (EXIT_FAILURE);
        }

        while (top == NULL) {
            if (pthread_cond_wait (&cond, &mutex)) {
                puts ("Failed cond wait");
                exit (EXIT_FAILURE);
            }
        }

        while (top != NULL) {
            fprintf(fp, "%s\n", top->mess);
            tmp = top;
            top = top->next;
            free (tmp->mess);
            free (tmp);
        }
        fflush (fp);
        if (pthread_mutex_unlock (&mutex)) {
            puts ("Failed unlock");
            exit (EXIT_FAILURE);
        }
    }

    if (fclose (fp)) {
        puts ("Failed fclose");
        exit (EXIT_FAILURE);
    }
}

void make_log (char* mess, int id) {
    struct log_record* tmp = (struct log_record*)malloc (sizeof (struct log_record));
    tmp->next = NULL;

    time_t tm = time(NULL);
    struct tm* tm_ptr = localtime (&tm);

    char time_str[MAX_SIZE_FOR_TIME];

    if (strftime (time_str, MAX_SIZE_FOR_TIME, "%Y.%m.%d %H.%M.%S", tm_ptr) == 0) {
        puts ("Failed strftime");
        exit (EXIT_FAILURE);
    }
    asprintf (&tmp->mess, "[%s] thread id : %d | mess : %s", time_str, id, mess);
    
    if (pthread_mutex_lock (&mutex)) {
        puts ("Failed mutex lock");
        exit (EXIT_FAILURE);
    }
    
    if (top == NULL) {
        top = tmp;
    }
    else {
        struct log_record* tmp2 = top;

        while (tmp2->next != NULL) 
            tmp2 = tmp2->next;
        tmp2->next = tmp;
    }
    if (pthread_mutex_unlock (&mutex)) {
        puts ("Failed mutex unlock");
        exit (EXIT_FAILURE);
    }
    if (pthread_cond_signal (&cond)) {
        puts ("Failed cond signal");
        exit (EXIT_FAILURE);
    }
}