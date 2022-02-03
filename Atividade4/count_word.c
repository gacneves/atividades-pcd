/**
 * C program to count occurrences of all words in a file.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/queue.h>

#define MAX_WORDS 100000

pthread_mutex_t queue_mutex;
pthread_mutex_t matrix_mutex;
pthread_mutex_t loop_mutex;

typedef struct node {
    char * word;
    TAILQ_ENTRY(node) nodes;
} node_type;

TAILQ_HEAD(head_s, node) head;

// List of distinct words
char words[MAX_WORDS][200];

// Count of distinct words
int count[MAX_WORDS];

int global_index, terminate;

void * processingWords(void * ptr){
   /*
    * Infinite loop that get the tail head, search if the word was already seen
    * if not, double-check if the word was written in the known words list by 
    * other thread, add it if it wasn't or just count the occurences.
    * If the word was already in the list, just count the occurency.
    * 
    * queue_mutex is used to controll the access of the queue.
    * matrix_mutex is used to monitor the access of the words matrix.
    * loop_mutex is used to protect the double-check for just one thread.
    */

    int i, j, len, is_unique, last_checked_index, unique_check;
    char * word;

    while(1){
        pthread_mutex_lock(&queue_mutex);
        while(TAILQ_EMPTY(&head)){
            if(terminate == 1){
                pthread_mutex_unlock(&queue_mutex);
                pthread_exit(0);
            }
        }
        node_type * rm_node = TAILQ_FIRST(&head);
        TAILQ_REMOVE(&head, rm_node, nodes);
        pthread_mutex_unlock(&queue_mutex);

        word = rm_node->word;
        // Remove last punctuation character
        len = strlen(word);
        if (ispunct(word[len - 1]))
            word[len - 1] = '\0';

        // Check if word exits in list of all distinct words
        is_unique = 1;
        for (i=0; i<global_index && is_unique; i++)
        {
            if (strcmp(words[i], word) == 0)
                is_unique = 0;
        }
        last_checked_index = i - 1;

        // If word is unique then add it to distinct words list
        // and increment global_index. Otherwise increment occurrence 
        // count of current word.
        if (is_unique) 
        {
            // Double check
            unique_check = 1;
            pthread_mutex_lock(&loop_mutex);
            for(j = last_checked_index; j < global_index && j > 0; j++)
            {
                if (strcmp(words[j], word) == 0){
                    pthread_mutex_lock(&matrix_mutex);
                    count[j]++;
                    pthread_mutex_unlock(&matrix_mutex);
                    unique_check = 0;
                    break;
                }
            }
            if(unique_check == 1)
            {
                pthread_mutex_lock(&matrix_mutex);
                strcpy(words[global_index], word);
                count[global_index]++;
                global_index++;
                pthread_mutex_unlock(&matrix_mutex);
            }
            pthread_mutex_unlock(&loop_mutex);
        }
        else
        {
            pthread_mutex_lock(&matrix_mutex);
            count[i - 1]++;
            pthread_mutex_unlock(&matrix_mutex);
        }

        free(rm_node);
    }
}

int main(int argc, char *argv[])
{
    FILE *fptr;
    char word[200];
    int i;
    clock_t t;

    /* Check number of arguments */
    if(argc != 3)
    {
        printf("Usage: ./count_word <filename> <number of threads>");

        exit(EXIT_FAILURE);
    }

    /* Try to open file */
    fptr = fopen(argv[1], "r");

    /* Exit if file not opened successfully */
    if (fptr == NULL)
    {
        printf("Unable to open file.\n");
        printf("Please check you have read previleges.\n");

        exit(EXIT_FAILURE);
    }

    /* Initialize words count to 0 */
    for (i=0; i<MAX_WORDS; i++)
        count[i] = 0;

    /* Create threads */
    int N = atoi(argv[2]);
    pthread_t tid[N];
    for (i = 0; i < N; i++)
        pthread_create(&tid[i], 0, processingWords, NULL);

    global_index = 0;
    terminate = 0;

    /* Initiate queue */
    TAILQ_INIT(&head);
    node_type * new_node;

    struct timeval start, final;
    gettimeofday(&start, NULL);
    t = clock();
    /* Read file and add words to queue */
    while (fscanf(fptr, "%s", word) != EOF)
    {
        new_node = malloc(sizeof(node_type));
        new_node->word = malloc(strlen(word) + 1 * sizeof(char));
        strcpy(new_node->word, word);
        TAILQ_INSERT_TAIL(&head, new_node, nodes);
        new_node = NULL;
    }

    /* Sign the threads that reached the end of the file */
    terminate = 1;
    for (i = 0; i < N; i++)
        pthread_join(tid[i], 0);

    gettimeofday(&final, NULL);
    double tsec = (double) (1000 * (final.tv_sec - start.tv_sec) + (final.tv_usec - start.tv_usec) / 1000) / 1000;
    t = clock() - t;
    
    /* Close file */
    fclose(fptr);

    /*
     * Print occurrences of all words in file. 
    */
    printf("Parallel execution time: %.3lf", tsec);
    printf("\nOccurrences of all distinct words in file: \n");
    for (i=0; i<global_index; i++)
    {
        /*
         * %-15s prints string in 15 character width.
         * - is used to print string left align inside
         * 15 character width space.
         */
        printf("%-15s => %d\n", words[i], count[i]);
    }    

    return 0;
}