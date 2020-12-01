#include <sys/types.h>
#include <sys/mman.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#define  KEY_NUM   60043 /*manage 공유메모리 키값 */
#define  MEM_SIZE  4096
#define THREADNUM 3 /*스레드 수*/
#define THREADPERWORK 3

struct threadArg{
	int tshmid;
	//long long fileSize;
	int number;
};

static int SharedMemoryCreate(int kNum);
static int SharedMemoryWrite(int shmid, char *shareddata, int size);
static int SharedMemoryRead(int shmid, char *sMemory);
static int SharedMemoryFree(int shmid);

int clientCnt=0;

void* filerecv(void* arg);
pthread_t thread[3];

int main(void){
    struct threadArg * argument;
    char recvbuffer[MEM_SIZE];
    //char sendbuffer[MEM_SIZE] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    /*1. manage sharedmemory 생성*/ //client 접속시 clientCnt 증가시킴s
    int ManageShmid = SharedMemoryCreate(KEY_NUM);
    
    //fork?
    while(1){
        SharedMemoryRead(ManageShmid,recvbuffer);
        if(recvbuffer[0]=='h'){
            clientCnt++;
            //값이 들어왔을 경우 스레드 생성

            for (int i = 0; i < 3; i++) {
                argument = (struct threadArg *)malloc(sizeof(struct threadArg));
	    	    argument->tshmid = clientCnt*10 + i+1;
		        argument->number=i+1;
		        pthread_create(&thread[i], NULL, (void*)filerecv, (void *)argument);
                
	        }
            sprintf(recvbuffer,"");
            break;    
        }
        //printf("X");
    }
    
    sleep(4);
    //SharedMemoryWrite(ManageShmid,sendbuffer, sizeof(sendbuffer));
    return 0;
}

void* filerecv(void * arg){
	int threadShmid; //11, 12, 13;
	char recvbuf[MEM_SIZE];
	char sendbuf[MEM_SIZE];
	int tempshmid;
	/*매개변수 저장*/
	struct threadArg * argument = (struct threadArg*)arg;
    /*
    int tshmid;
	int number;
    */
    
	/*3. 각 쓰레드는 1개의 shm 생성.*/
    int tid = SharedMemoryCreate(argument->tshmid);
	printf("thread 생성 %d\n",argument->tshmid);
    
    //client에서 보내는 파일 받기

    while(1)
    {
        SharedMemoryRead(tid,recvbuf);
        if(recvbuf[0]=='a'){
            printf("success %d: %s\n",tid, recvbuf);
            break;
        }
    }
    printf("while end\n");

	pthread_exit(0);
}





static int SharedMemoryCreate(int kNum) //성공시 shmid 리턴
{
    int shmid;
    if((shmid = shmget((key_t)kNum, MEM_SIZE, IPC_CREAT| IPC_EXCL | 0666)) == -1) {
        //IPC_CREATE - key에 해당하는 메모리가 없으면 공유메모리를 생성한다.
        //IPC_EXCL - 공유메모리가 있으면 실패로 반환 
        shmid = shmget((key_t)kNum, MEM_SIZE, IPC_CREAT| 0666);
        if(shmid == -1)
        {
            return -1;
        }
        else
        {
            SharedMemoryFree(shmid);
            shmid = shmget(kNum, MEM_SIZE, IPC_CREAT| 0666);
            if(shmid == -1)
            {
                //perror("Shared memory create fail");
                return 1;
            }
        }
    }
    return shmid;
}
 
static int SharedMemoryWrite(int shmid, char *shareddata, int size)
{
    void *shmaddr;
    
    //공유메모리 연결
    //shmctl(shmid, SHM_LOCK, 0);
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1) 
    {
        //perror("writeShmat failed");
        return 1;
    }
    //printf("in write test :%s\n",shareddata);
    strncpy(shmaddr, shareddata,sizeof(shareddata));
    //memcpy((char *)shmaddr, shareddata, sizeof(shareddata));
    //printf("in write test shmaddr :%s\n",(char *)shmaddr);
    if(shmdt(shmaddr) == -1) 
    //프로세스에서 공유메모리 분리
    {
        //perror("Shmdt failed");
        return 1;
    }
    //shmctl(shmid, SHM_UNLOCK, 0);
    return 0;
}

 
static int SharedMemoryRead(int shmid,char *sMemory)
{
    void *shmaddr;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1)
    {
        //perror("read Shmat failed");
        return 1;
    }
    //printf("%s ",(char*)shmaddr);
    sprintf(sMemory,shmaddr);
    //printf("%s",sMemory);
    if(shmdt(shmaddr) == -1)
    {
        //perror("Shmdt failed");
        return 1;
    }
    return 0;
}

 
static int SharedMemoryFree(int shmid)
{
    if(shmctl(shmid, IPC_RMID, 0) == -1) 
    {
        //perror("Shmctl failed");
        return 1;
    }
    //printf("Shared memory end\n");
    return 0;
}
