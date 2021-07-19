 #include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>



#define MSG_SIZE 80

void* filesend(void* th);
pthread_t thread[3];

typedef struct tTHREAD
{
   char ddmsg[MSG_SIZE];
   int cCnt;
}THREAD;


//client
int main()
{
    char msg[MSG_SIZE]; //num count
    char msg2[MSG_SIZE];//after
    int filedesmf;
    int cnt;
    int nread;

    
    // open managefifo file
    if((filedesmf = open("./managefifo", O_RDONLY| O_TRUNC)) < 0){
        printf("fail to open fifo manage()\n");
        exit(1);
    }
    
    if((nread = read(filedesmf, msg, MSG_SIZE)) < 0 ){ //cnt
        printf("fail to call read manage()\n");
        exit(1);
    }

    
    //file read & save in msg
    
    //divide three file
    THREAD th[3];
    strcpy(th[0].ddmsg,"hello");
    strcpy(th[1].ddmsg,"two");
    strcpy(th[2].ddmsg,"three");

    
    //thread 3
    for(int i=0;i<3;i++){
        th[i].cCnt=atoi(msg)+i; //1,2,3 pass
        pthread_create(&thread[i],NULL,(void *)filesend, (void *)&th[i]);
        //sleep(1);
    }

    for(int i=0;i<=3;i++)
     {
          pthread_join(thread[i],NULL);
     }


    
}

void* filesend(void* th){ //n = file source
    
    THREAD* pp = (THREAD*)th;
    int filedesth;
    char dmsg[MSG_SIZE];
    strcpy(dmsg,pp->ddmsg);
    //int cCnt = pp->cCnt;
    char cCnt[MSG_SIZE];
    sprintf(cCnt,"%d",pp->cCnt);

    int len = strlen(dmsg);
    dmsg[len]='\n';
    //printf(dmsg);


    char threadpath[MSG_SIZE] = "./threadfifo";
    char threadpathTemp[MSG_SIZE];

    strcpy(threadpathTemp,threadpath);
    strcat(threadpathTemp,cCnt); //./threadfifo2

    //thread fifo open
    while(1){
        if (access(threadpathTemp, F_OK) == 0) {
		    if((filedesth = open(threadpathTemp, O_WRONLY)) < 0){
                printf("fail to call fifo thread() %s\n",threadpathTemp);
                exit(1);
            }
            break;
	    }
    }
    
    //send msg
    if(write(filedesth, dmsg, MSG_SIZE)==-1){ //fifo rw
            printf("fail to call write()\n");
            exit(1);
        }
    //sleep(1);


        /*
        if(write(filedes, msg, MSG_SIZE)==-1){ //fifo rw
            printf("fail to call write()\n");
            exit(1);
        }

        //before receive decoding file, wait

        //receive
        if((nread = read(filedes, msg2, MSG_SIZE)) < 0 ){
            printf("fail to call read()\n");
            exit(1);
        }
        */
        //print file
        
        //file sum
        
    
}


/*
    for(cnt=0; cnt<3; cnt++){
        printf("input a message : ");
        gets(msg);
        msg[MSG_SIZE-1] = '\0';
        if(write(filedes, msg, MSG_SIZE)==-1){ //fifo
            printf("fail to call write()\n");
            exit(1);
        }
        sleep(1);
    }
    */