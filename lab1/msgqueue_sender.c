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
#define QUEUE_MAXMSG 10     // Can not be more than 10
#define QUEUE_MSGSIZE 1024

typedef struct {
    char msg[QUEUE_MSGSIZE];
} msg_struct;


int main(void) {
    // ------------------- UNLINK IF ANY PREVIOUS MQ WAS OPEN -------------------

    int status = mq_unlink(QUEUE_NAME);
    if (status < 0){
        printf ("   Warning %d (%s) on server mq_unlink.\n",
            errno, strerror (errno));
    }

    // ------------------- OPEN QUEUE -------------------

    mqd_t mq_serv;
    struct mq_attr mqAttr;

    int open_flags = O_CREAT | O_RDWR | O_NONBLOCK;     // Flags
    int perms = S_IWUSR | S_IRUSR;                      // Permissions

    mqAttr.mq_maxmsg = QUEUE_MAXMSG;        // Set max amount of msg in queue
    mqAttr.mq_msgsize = QUEUE_MSGSIZE;      // Set max size on msg

    mq_serv = mq_open(QUEUE_NAME, open_flags, perms, &mqAttr);
    if (mq_serv < 0){
        perror("mq_open failure from serv");
        exit(0);
    }

    // ------------------- FORK TO HAVE DIFFERENT PROCESSES -------------------


    if(fork() == 0){
        // ------------------- OPEN CLIENT CONNECTION TO QUEUE -------------------

        mqd_t mq_cli = mq_open(QUEUE_NAME, O_RDWR);
        if (mq_cli < 0) {
            perror("mq_open failure from child");
            exit(0);
        }

        // ------------------- READ FROM FILE -------------------

        msg_struct mq_msg;

        FILE *fp = fopen("a_text_file.txt", "r");

        while(fread(&mq_msg.msg, sizeof(char), QUEUE_MSGSIZE, fp)){
        // ------------------- SEND MESSAGE -------------------
            status = mq_send(mq_cli, (char *)&mq_msg, sizeof(mq_msg), 1);

            if (status < 0){
                perror("mq_send failure on mq");
            } 
            memset(mq_msg.msg, 0, sizeof(mq_msg.msg));      // Reset msg so there is no chars left in the msg after the read string
        }

        mq_close(mq_cli);

    } else {
        // ------------------- WAIT FOR SEND -------------------

        wait(NULL);

        // ------------------- START RECEIVE -------------------

        msg_struct recv_msg;
        int status;
        long word_counter = 0; 
        
        while(1){
            status = mq_receive(mq_serv, (char *)&recv_msg, sizeof(msg_struct), NULL);      // wanted to use mq_timedreceive(5) but it was not deterministic 

            if (status < 0) {   // Nothing left to read, or something gone wrong
                break;
            }

            int i = 0;
            while(recv_msg.msg[i]!='\0'){
                /* check whether the current character is white space or new line or tab character*/
                if(recv_msg.msg[i]==' ' || recv_msg.msg[i]=='\n' || recv_msg.msg[i]=='\t') {
                    word_counter++;
                }
                i++;
            }
        }   

        // ------------------- END -------------------

        printf("Amount of words: %ld\n", word_counter);
        mq_close(mq_serv);
    }  
    return 0;
}