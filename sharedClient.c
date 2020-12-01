#include <sys/types.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/select.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

#define  KEY_NUM   60043
#define  MEM_SIZE  4096
#define THREADNUM 3
#define BUF_SIZE 4096
#define MAX_SIZE 4096
#define T 5
#define FILENAMESIZE 255

static int SharedMemoryManageInit(int kNum);
static int SharedMemoryInit(int kNum);
static int SharedMemoryWrite(int shmid,char *sMemory, int size);
static int SharedMemoryRead(int shmid,char *sMemory);
static int SharedMemoryFree(int shmid);



void* filesend(void* th);
pthread_t thread[3];

int main() {

	 char sendbuffer[MEM_SIZE] = "hello\n";
    char recvbuffer[MEM_SIZE];
    int ManageShmid;
    
    ManageShmid= SharedMemoryManageInit(KEY_NUM);
    
    SharedMemoryWrite(ManageShmid,sendbuffer,sizeof(sendbuffer));
    

    /*4. 3개의 쓰레드를 할당*/
    for(int i=0;i<3;i++){
	//실패 시 에러처리 추가 
        printf("thread %d\n",i);
        
        pthread_create(&thread[i],NULL,(void *)filesend, (void *)(i+1));        
    }
    for(int i=0;i<=3;i++)
     {
          pthread_join(thread[i],0);
     }
    return 0;
}

void* filesend(void* arg){ 

    
    int threadShmid = 10 + (int)arg;//10을 서버에서 받아오게 수정
    char recvbuf[MEM_SIZE];
	char sendbuf[MEM_SIZE];

    sprintf(sendbuf,"abcdefghijklmnopqrstuvwzyz%d\n",threadShmid);
    int tid = SharedMemoryInit(threadShmid);
    SharedMemoryWrite(tid,sendbuf,sizeof(sendbuf));
    printf("%d 전송\n",threadShmid);

 //   sleep(2);    
}

static int SharedMemoryManageInit(int kNum)
{
    int shmid; 
    while(1){ 
        if((shmid = shmget((key_t)kNum, 0, 0))==-1)
        {
            //perror("write Shmat failed");
        }else{
            return shmid;
        }
    }
    return 0;
}


static int SharedMemoryInit(int kNum)
{
    int shmid;
    if((shmid = shmget((key_t)kNum, MEM_SIZE, IPC_CREAT| IPC_EXCL | 0666)) == -1) {
        //새로 만드는 경우
        //서버 측에서 이미 공유메모리 생성
        while(1){ 
            if((shmid = shmget((key_t)kNum, 0, 0))==-1)
            {
                //perror("write Shmat failed");
            }else{
                //printf("서버측 생성 연결 성공%d\n",kNum);
                return shmid;
            }
        }
    }//클에서 만듦
    //printf("클 생성해서 연결 성공%d\n",kNum);  
    return shmid;
}


static int SharedMemoryWrite(int shmid, char *sMemory, int size)
{
    void *shmaddr;
    //printf("before write :%s",sMemory);
    //서버에서 생성 안해서 오류날 경우 계속 반복
    while(1){ 
        //연결 될때까지 무한루프
        if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){}
        else{
            
            sprintf((char*)shmaddr,sMemory);
            //printf("after write :%s",(char*)shmaddr);
            if(shmdt(shmaddr) == -1)
            {
                perror("close write Shmdt failed");
            }
            return 0;
        }
    }
    return 0;
}


static int SharedMemoryRead(int shmid,char *sMemory)
{
    void *shmaddr;
    while(1){ 
        //연결 될때까지 무한루프
        if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){}
        else{
            sprintf(sMemory,shmaddr);
            //printf("after write :%s",(char*)shmaddr);
            if(shmdt(shmaddr) == -1)
            {
                perror("close write Shmdt failed");
            }
            return 0;
        }
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