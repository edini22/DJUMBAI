#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>



int main() {

    // message queue, djumbai-send, djumbai-lspawn

    // create message queue
    int mq = msgget(0x1234, IPC_CREAT | 0666); //TODO: 666
    if (mq == -1) {
        perror("msgget");
        return 1;
    }





    return 0;
}
