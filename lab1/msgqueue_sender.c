#include <mqueue.h>
#include <fcntl.h>    /* For O_* constants. */
#include <sys/stat.h> /* For mode constants. */

#include <errno.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/wait.h>

#define QUEUE_NAME "/pax"
#define QUEUE_MAXMSG 16
#define QUEUE_MSGSIZE 1024

int main(void) {
       
    // ------------------- OPEN QUEUE -------------------
    int open_flags = O_CREAT | O_RDWR | O_NONBLOCK;
    int perms = S_IWUSR | S_IRUSR;

    mqd_t mq_serv;
    struct mq_attr mqAttr;
    

    int status = mq_unlink(QUEUE_NAME);
    if (status < 0){
        printf ("   Warning %d (%s) on server mq_unlink.\n",
            errno, strerror (errno));
    }

    mqAttr.mq_maxmsg = 10;
    mqAttr.mq_msgsize = 1024;

    mq_serv = mq_open(QUEUE_NAME, open_flags, perms, &mqAttr);
    if (mq_serv < 0){
        perror("mq_open failure from serv");
        exit(0);
    }


    if(fork() == 0){
        // ------------------- OPEN CLIENT CONNECTION TO QUEUE -------------------

        mqd_t mq_cli = mq_open(QUEUE_NAME, O_RDWR);
        if (mq_cli < 0) {
            perror("mq_open failure from child");
            exit(0);
        }


        // ------------------- READ FROM FILE -------------------

        char sndbuff[QUEUE_MSGSIZE];

        FILE *fp = fopen("a_text_file.txt", "r");

        int sndcounter = 1;
        int freadstatus = 1;
        while(0 < freadstatus){

        // ------------------- SEND MESSAGE -------------------
            freadstatus = fread(&sndbuff, sizeof(char), QUEUE_MSGSIZE, fp);
            status = mq_send(mq_cli, sndbuff, strlen(sndbuff), 1);

            if (status < 0){
                perror("mq_send failure on mq");
            } else {
                printf("successful call to mq_send\n");
                printf("Sending: %d\n%s\n", sndcounter++, sndbuff);
            }
        }

        mq_close(mq_cli);

    } else {
        // ------------------- WAIT FOR SEND -------------------

        wait(NULL);
        printf("---------------------- starting receive ------------------------\n");
        char revbuff[QUEUE_MSGSIZE];
        int status;
        int recv_counter = 0;
        while(1 && recv_counter++ < 100){
            status = mq_receive(mq_serv, revbuff, sizeof(revbuff), NULL);
            if (status < 0) {
                exit (1);
            }

            if (status == -1){
                perror("mq_receive failure on mq");
            } else {
                printf("%s\n", revbuff);
            }
        }
            mq_close(mq_serv);
    }  


    return 0;
}