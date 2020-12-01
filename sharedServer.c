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

static int SharedMemoryManageCreate(int kNum);
static int SharedMemoryCreate(int kNum);
static int SharedMemoryWrite(int shmid, char *sMemory, int size);
static int SharedMemoryRead(int shmid,char *sMemory);
static int SharedMemoryFree(int shmid);

int clientCnt=0;

void* filerecv(void* arg);
pthread_t thread[3];

//공유메모리  매핑
/*
management 에서 

*/


int main(void){
    struct threadArg * argument;
    char recvbuffer[MEM_SIZE];
    //char sendbuffer[MEM_SIZE] = "aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
    /*1. manage sharedmemory 생성*/ //client 접속시 clientCnt 증가시킴s
    int ManageShmid = SharedMemoryManageCreate(KEY_NUM);
    
    //fork?recvbuf[0]==0)
    
    while(1){
        SharedMemoryRead(ManageShmid,recvbuffer);
        if(recvbuffer[0]==0){}
        else{
            clientCnt++;
            //값이 들어왔을 경우 스레드 생성
            for (int i = 0; i < 3; i++) {
                argument = (struct threadArg *)malloc(sizeof(struct threadArg));
	    	    argument->tshmid = clientCnt*10 + i+1;
		        argument->number=i+1;
		        pthread_create(&thread[i], NULL, (void*)filerecv, (void *)argument);
                
	        }
            //SharedMemoryWrite(ManageShmid,"\0",sizeof("\0"));
        }
    }
    
    //sleep(4);
    
    for(int i=0;i<=3;i++)
     {
          pthread_join(thread[i],0);
     }
     SharedMemoryFree(ManageShmid);

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
    threadShmid = argument->tshmid;
    
	/*3. 각 쓰레드는 1개의 shm 생성.*/
    int tid = SharedMemoryCreate(threadShmid);
	//printf("thread 생성 %d\n",threadShmid);
    
    //client에서 보내는 파일 받기
    while(1)
    {
        SharedMemoryRead(tid,recvbuf);
        if(recvbuf[0]==0){
            
        }else{
            printf("받은거  %d: %s\n",threadShmid, recvbuf);
            break;
        }
    }

    //변형파일 전송할예정
    
    sprintf(sendbuf,"%s 에다가 변형된 문장입니다.",recvbuf);
    printf("보낼거 : %s\n",sendbuf);
    SharedMemoryWrite(tid,sendbuf,sizeof(sendbuf));
    

//    SharedMemoryFree(tid);

}

static int SharedMemoryManageCreate(int kNum) //manage는 무조건 서버에서 새로생성할 필요가 있는거같다
{
    int shmid;
    if((shmid = shmget((key_t)kNum, MEM_SIZE, IPC_CREAT| IPC_EXCL | 0666)) == -1) {
        //IPC_CREATE - key에 해당하는 메모리가 없으면 공유메모리를 생성한다.
        //IPC_EXCL - 공유메모리가 있으면 실패로 반환 
        //클라이언트 측에서 이미 생성
        shmid = shmget((key_t)kNum, MEM_SIZE, IPC_CREAT| 0666);
        if(shmid == -1)
        {
            printf("error1");
            return -1;
        }
        else
        {   
            SharedMemoryFree(shmid);
            shmid = shmget(kNum, MEM_SIZE, IPC_CREAT| 0666);
            if(shmid == -1)
            {
                printf("error2");
                return 1;
            }
            return shmid;
        }
    }
    return shmid;
}





static int SharedMemoryCreate(int kNum) //성공시 shmid 리턴
{
    int shmid;
    if((shmid = shmget((key_t)kNum, MEM_SIZE, IPC_CREAT| IPC_EXCL | 0666)) == -1) {
        //IPC_CREATE - key에 해당하는 메모리가 없으면 공유메모리를 생성한다.
        //IPC_EXCL - 공유메모리가 있으면 실패로 반환 
        //클라이언트 측에서 이미 생성
        if((shmid = shmget((key_t)kNum, 0, 0))==-1)
        {
            //perror("write Shmat failed");
        }else{
            //printf("클측 생성 연결 성공%d\n",kNum);
            return shmid;
        }
    }
    //서버측에서 정상적으로 만든경우
    //printf("서버측 생성 연결 성공%d\n",kNum);
    return shmid;
}
 
static int SharedMemoryWrite(int shmid, char *sMemory, int size)
{
    void *shmaddr;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){
        //연결 될때까지 무한루프
    }else{
        sprintf((char*)shmaddr,sMemory);
        printf("보내진거 :%s\n",shmaddr);
        if(shmdt(shmaddr) == -1)
        {
            printf("error3");
        }
        return 0;
    }
    return 0;
}

 
static int SharedMemoryRead(int shmid,char *sMemory)
{
    void *shmaddr;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){}
    else{
        sprintf(sMemory,shmaddr);
        if(shmdt(shmaddr) == -1)
        {
            printf("error4");
        }
        return 0;
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

