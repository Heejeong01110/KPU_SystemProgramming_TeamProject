 #include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <semaphore.h>

#define MSG_SIZE 80
int clientCnt = 0;
void *filerecv(int threadnum);
pthread_t thread[3];
sem_t threadrock;

//server
int main()
{
    char msg[MSG_SIZE];
    int filedesmf;
    int nread;
    char cnt[MSG_SIZE];

    unlink("./managefifo");
    unlink("./threadfifo1");
    unlink("./threadfifo2");
    unlink("./threadfifo3");

    
    printf("시작 전\n");
    //total fifo manage
    if(mkfifo("./managefifo",0666) == -1){ //fifo init
        printf("fail to make fifo manage()\n");
        exit(1);
    }

    if((filedesmf = open("./managefifo", O_WRONLY| O_TRUNC)) < 0){ //waiting in client
        printf("fail to call open manage()\n");
        exit(1);
    }
    
    // add process???????????? fork()??

    //wait client
    sprintf(cnt,"%d",++clientCnt);

    if(write(filedesmf,cnt , MSG_SIZE)==-1){
        printf("fail to call write cnt()\n");
        exit(1);
    }
    
    //semapore
    sem_init(&threadrock,0,1); 

    //thread 3
    for(int i=0;i<3;i++){
        //th[i].cCnt=clientCnt+i; //1,2,3 pass
        pthread_create(&thread[i],NULL,(void *)filerecv, (int *)(clientCnt+i));
        
    }

    for(int i=0;i<=3;i++)
     {
          pthread_join(thread[i],NULL);
     }
     sem_destroy(&threadrock);

    /*

    if(mkfifo(threadpathTemp,0666) == -1){ //fifo init
        printf("fail to make threadfifo()\n");
        exit(1);
    }

    //here??
    if((filedesmf = open(threadpathTemp, O_RDWR| O_TRUNC)) < 0){
        printf("fail to call threadfifo()\n");
        exit(1);
    }

    */

    /*
    for(cnt=0; cnt<3; cnt++){
        if((nread = read(filedes, msg, MSG_SIZE)) < 0 ){
            printf("fail to call read()\n");
            exit(1);
        }
        printf("recv: %s\n", msg);
    }
    */
    unlink("./managefifo");
    

    return 0;
}

void* filerecv(int threadnum){

    char threadpath[MSG_SIZE] = "./threadfifo";
    char threadpathTemp[MSG_SIZE];
    char threadCnt[MSG_SIZE];
    int filedesth, nread;
    char arriveMsg[MSG_SIZE];
    sprintf(threadCnt,"%d",threadnum);


    strcpy(threadpathTemp,threadpath);
    strcat(threadpathTemp,threadCnt); //./threadfifo2
    printf("mkfifo %s\n",threadpathTemp);
    if(mkfifo(threadpathTemp,0666) == -1){ //fifo init
        printf("fail to make threadfifo() %s\n",threadpathTemp);
        exit(1);
    }


    if((filedesth = open(threadpathTemp, O_RDONLY)) < 0){
        printf("fail to call threadfifo()\n");
        exit(1);
    }

    //read msg
    if((nread = read(filedesth, arriveMsg, MSG_SIZE)) < 0 ){
            printf("fail to call read()\n");
            exit(1);
    }
    printf("%s",arriveMsg);
    printf("\n");

    unlink(threadpathTemp);






    printf("fifo %s destroy\n",threadpathTemp);
    
}