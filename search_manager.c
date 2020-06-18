#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>
#include <signal.h>
#include <unistd.h>
#include "longest_word_search.h"
#include "queue_ids.h"
#include <pthread.h>
#include <semaphore.h>

//global data with status information on search requests, initialized in main and accessed during rcv and by sigint handler
char **prefixes;
size_t num_prefixes;
size_t num_passages;
sem_t current_prefix;
sem_t current_response;

void sigint_status_handler(int signal)
{
    int curr_prefix_index;
    int curr_response_index;
    sem_getvalue(&current_prefix, &curr_prefix_index);
    sem_getvalue(&current_response, &curr_response_index);

    printf("\n");
    for (size_t i = 0; i < num_prefixes; i++)
    {
        if (i < curr_prefix_index - 1)
        {
            printf("%s - done\n", prefixes[i]);
        }
        else if (i > curr_prefix_index - 1)
        {
            printf("%s - pending\n", prefixes[i]);
        }
        else if (curr_response_index == num_passages)
        { //implied count does not equal 0. this could happen if we raise the signal right when we finish a prefix but before the next prefix.
            printf("%s - done\n", prefixes[i]);
        }
        else
        {
            printf("%s - %d of %d\n", prefixes[i], curr_response_index, num_passages);
        }
    }
    printf("\n");
}

void sigint_status_handler_start(int signal)
{
    printf("\n");
    for (size_t i = 0; i < num_prefixes; i++)
    {
        printf("%s - pending\n", prefixes[i]);
    }
    printf("\n");
}

//string copy function to make life easier
size_t strlcpy(char *dst, const char *src, size_t size)
{
    size_t srclen;
    size--;

    srclen = strlen(src);

    if (srclen > size)
        srclen = size;

    memcpy(dst, src, srclen);
    dst[srclen] = '\0';

    return (srclen);
}

int isStringAlpha(char *s)
{
    char *c = s;
    while (*c)
    { //will break when reached null character
        if (!isalpha(*c))
        {
            return 0;
        }
        c++;
    }
    return 1;
}

void send_prefix_request(char *prefix, int msqid)
{
    prefix_buf request;
    size_t request_length;

    //sent prefix messages are of type 1 (while the responses from the passage processor are of type 2)
    request.mtype = 1;
    strlcpy(request.prefix, prefix, WORD_LENGTH);
    sem_getvalue(&current_prefix, &(request.id));
    request_length = strlen(request.prefix) + sizeof(int) + 1; //struct size without long int type

    //now send the request
    // Send a message.
    if ((msgsnd(msqid, &request, request_length, IPC_NOWAIT)) < 0)
    {
        int errnum = errno;
        fprintf(stderr, "%d, %ld, %s, %d\n", msqid, request.mtype, request.prefix, (int)request_length);
        perror("(msgsnd)");
        fprintf(stderr, "Error sending msg: %s\n", strerror(errnum));
        exit(1);
    }
    else
        fprintf(stderr, "\nMessage(%d): \"%s\" Sent (%d bytes)\n", request.id, request.prefix, (int)request_length);
}

void send_exit_request(int msqid)
{
    prefix_buf request;
    size_t request_length;

    //sent prefix messages are of type 1 (while the responses from the passage processor are of type 2)
    request.mtype = 1;
    strlcpy(request.prefix, "", WORD_LENGTH);
    request.id = 0;
    request_length = strlen(request.prefix) + sizeof(int) + 1; //struct size without long int type

    //now send the request
    // Send a message.
    if ((msgsnd(msqid, &request, request_length, IPC_NOWAIT)) < 0)
    {
        int errnum = errno;
        fprintf(stderr, "%d, %ld, %s, %d\n", msqid, request.mtype, request.prefix, (int)request_length);
        perror("(msgsnd)");
        fprintf(stderr, "Error sending msg: %s\n", strerror(errnum));
        exit(1);
    }
    else
        fprintf(stderr, "\nMessage(%d): \"%s\" Sent (%d bytes)\n", request.id, request.prefix, (int)request_length);
}

int receive_prefix_response(response_buf *response, int msqid)
{
    //the 0 in the last parameter of msgrcv makes it blocking... until it gets a message, this process will not continue.
    int ret;
    do
    {
        ret = msgrcv(msqid, response, sizeof(response_buf), 2, 0); //receive type 2 message
        int errnum = errno;
        if (ret < 0 && errno != EINTR)
        {
            fprintf(stderr, "Value of errno: %d\n", errno);
            perror("Error printed by perror");
            fprintf(stderr, "Error receiving msg: %s\n", strerror(errnum));
        }
    } while ((ret < 0) && (errno == 4));
    return ret; //this is the number of bytes recieved in the message minus the required long mtype. this is in longest_word_search.h
}

//reasons for parameters:
//msqid: msgrcv needs to know the queue id to receive from.
void get_all_responses(int msqid)
{
    response_buf first_response;
    receive_prefix_response(&first_response, msqid);

    //list of responses in order of passage index. as the messages are recieved,
    //the responses are updated. when all are updated, report is printed.
    num_passages = first_response.count;
    response_buf responses[first_response.count];
    responses[first_response.index] = first_response;
    sem_post(&current_response);

    //now that we have actually begun receiving responses, change the signal handler to the real one
    //only issue i could see with this here is that this method is called num_prefix number of times.
    //i don't think this has repercussions besides being slightly redeuntant.
    signal(SIGINT, sigint_status_handler);

    for (size_t i = 1; i < first_response.count; i++)
    {
        response_buf temp;
        receive_prefix_response(&temp, msqid);
        responses[temp.index] = temp;
        //current passage number stored in the current_passage semaphore since it is updated atomically.
        sem_post(&current_response);
        if(i==3) sleep(5);
        
    }

    int curr_prefix_index;
    sem_getvalue(&current_prefix, &curr_prefix_index);
    printf("\nReport \"%s\"\n", prefixes[curr_prefix_index - 1]);
    for (size_t i = 0; i < first_response.count; i++)
    {
        response_buf response = responses[i];
        if (response.present == 0)
        {
            printf("Passage %d - %s - no word found\n", response.index, response.location_description);
        }
        else
        {
            printf("Passage %d - %s - %s\n", response.index, response.location_description, response.longest_word);
        }
    }
}

int main(int argc, char **argv)
{
    int msqid;
    int msgflg = IPC_CREAT | 0666;
    key_t key;

    //check command syntax
    if (argc <= 2)
    {
        printf("Usage: %s <secs between sending prefix requests> <prefix1> <prefix2> ...\n", argv[0]);
        exit(-1);
    }

    num_passages = 0;
    num_prefixes = argc - 2;

    //confirm all prefixes have at least 3 characters.
    //POTENTIAL TODO: make sure that prefixes are only alphabetic.
    size_t check_prefix_args;
    for (check_prefix_args = 2; check_prefix_args < argc; check_prefix_args++)
    {
        size_t curr_len = strlen(argv[check_prefix_args]);
        if (curr_len < 3 || curr_len > 20 || isStringAlpha(argv[check_prefix_args]) == 0)
        {
            fprintf(stderr, "Error: Invalid Prefix Found. Will continue with valid prefixes.\n");
            argv[check_prefix_args] = "";
            num_prefixes--;
        }
    }

    if (num_prefixes == 0)
    {
        fprintf(stderr, "No valid prefixes... Exiting.\n");
        exit(-1);
    }

    //allocate prefix array for global use
    size_t prefix_index = 0;
    prefixes = malloc(num_prefixes * sizeof(char *));
    for (check_prefix_args = 2; check_prefix_args < argc; check_prefix_args++)
    {
        if (argv[check_prefix_args] != "" && argv[check_prefix_args][0] != '\0') //is it empty or is the first character a null character. these might do the same thing but if it aint broke dont fix am i right
        {
            prefixes[prefix_index++] = argv[check_prefix_args];
        }
    }

    //init the semaphores for atomic status keeping. 
    sem_init(&current_prefix, 0, 0);

    //now that prefix array has been allocated and assigned, we can start checking status with signals.
    signal(SIGINT, sigint_status_handler_start);

    //create the message queue
    key = ftok(CRIMSON_ID, QUEUE_NUMBER);
    if ((msqid = msgget(key, msgflg)) < 0)
    {
        int errnum = errno;
        fprintf(stderr, "Value of errno: %d\n", errno);
        perror("(msgget)");
        fprintf(stderr, "Error msgget: %s\n", strerror(errnum));
    }

    //now loop through the prefixes, generate a request for each one, wait for the response
    for (size_t i = 0; i < num_prefixes; i++)
    {
        //increment our prefix index semaphore and reinit our passage response semaphore to 0
        sem_post(&current_prefix);
        sem_init(&current_response, 0, 0);

        send_prefix_request(prefixes[i], msqid);
        get_all_responses(msqid);
        sleep(atoi(argv[1]));
    }

    //exit request
    send_exit_request(msqid);

    //free memory
    printf("\nExiting ...\n");
    free(prefixes);

    exit(0);
}
