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

static int SharedMemoryInit(int kNum);
static int SharedMemoryWrite(int shmid,char *sMemory, int size);
static int SharedMemoryRead(int shmid,char *sMemory);



void* filesend(void* th);
pthread_t thread[3];

int main() {

	 char sendbuffer[MEM_SIZE] = "hello";
    char recvbuffer[MEM_SIZE];
    int ManageShmid;
    
    ManageShmid= SharedMemoryInit(KEY_NUM);
    
    

    SharedMemoryWrite(ManageShmid,sendbuffer,sizeof(sendbuffer));
    
    printf("manage 연결 성공\n");

    /*4. 3개의 쓰레드를 할당*/
    for(int i=0;i<3;i++){
	//실패 시 에러처리 추가 
        printf("%d thread 시작 전\n",i);
        pthread_create(&thread[i],NULL,(void *)filesend, (void *)(i+1));
        sleep(1);
        
    }

    /*
    //recv
    while(1)
    {

        SharedMemoryRead(ManageShmid,recvbuffer);
        if(recvbuffer[0]=='a')
        {
            printf("Receive data from shared memory! : %s\n", recvbuffer);
            
            break;
        }    
    }
    */
        
    
    return 0;
}

void* filesend(void* arg){ 
    int threadShmid = 10 + (int)arg;//10을 서버에서 받아오게 수정
    char recvbuf[MEM_SIZE];
	char sendbuf[MEM_SIZE];
    printf("%d 스레드 시작 \n",threadShmid);

    sprintf(sendbuf,"abcdefghijklmnopqrstuvwzyz%d\n",threadShmid);
    int tid = SharedMemoryInit(threadShmid);
    SharedMemoryWrite(tid,sendbuf,sizeof(sendbuf));
    printf("%d 전송",threadShmid);

    pthread_exit(0);
}



static int SharedMemoryInit(int kNum)
{
    int shmid;
    void *shmaddr;
    printf("연결 시도");
    shmid = shmget((key_t)kNum, 0, 0);
    if(shmid ==-1){
        printf("init 대기중");
        return 0;
    }
    
    return shmid;
}


static int SharedMemoryWrite(int shmid, char *sMemory, int size)
{
    void *shmaddr;
    printf("before write :%s\n",sMemory);
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1)
    {
        perror("write Shmat failed");
    }
    
    //sprintf
    //memcpy((char *)shmaddr, sMemory, size);
    strncpy(shmaddr, sMemory,size);
    printf("after write :%s\n",(char*)shmaddr);

    //printf("after write :%s\n",shmaddr);
    if(shmdt(shmaddr) == -1)
    {
        perror("close write Shmdt failed");
    }
    return 0;
}


static int SharedMemoryRead(int shmid,char *sMemory)
{
    void *shmaddr;
    shmctl(shmid, SHM_LOCK, 0);
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1)
    {
        perror("read Shmat failed");
    }
    sprintf(sMemory,shmaddr);
    if(shmdt(shmaddr) == -1)
    {
        perror("close read Shmdt failed");
    }
    shmctl(shmid, SHM_LOCK, 0);
    return 0;
}


