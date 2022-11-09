# bin/bash

gcc msgqueue_sender.c -o msgqueue_sender -Wall -lrt
# gcc msgqueue_receiver.c -o msgqueue_receiver -Wall -lrt
./msgqueue_sender
# ./msgqueue_receiver