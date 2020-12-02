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

	char sendbuffer[MEM_SIZE] = "hello";
    char recvbuffer[MEM_SIZE];
    int ManageShmid;
    printf("init 전\n");
    ManageShmid= SharedMemoryManageInit(KEY_NUM);
    printf("write 전\n");
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
    int shmbufSize = 0;
    printf("thread 진입\n");
    int threadShmid = 10 + (int)arg;//10을 서버에서 받아오게 수정
    char recvbuf[MEM_SIZE];
	char sendbuf[MEM_SIZE];

    sprintf(sendbuf,"abcdefghijklmnopqrstuvwzyz%d",threadShmid);
    int tid = SharedMemoryInit(threadShmid);
    printf("write 전\n");
    SharedMemoryWrite(tid,sendbuf,sizeof(sendbuf));
    printf("%d 전송\n",threadShmid);

    //변형된 파일 받을 예정
    //변형된 파일이 안받아지는 이유
    /*
    해결방법
    1. 맨 앞에 수정했다는 커맨드를 확인할 수 있도록 recvbuf[0] 자리에 어떠한 신호를 준다.
    이게 더 간단한듯
    */
    sleep(2);
    printf("read 전\n");
    while(!SharedMemoryRead(tid,recvbuf))
    {
        //다시 시도    
    }
    printf("success recv %d: %s\n",threadShmid, recvbuf);

 //   sleep(2);    
}

static int SharedMemoryManageInit(int kNum)
{
    int shmid; 
    if((shmid = shmget((key_t)kNum, 0, 0))==-1)
    {
        //perror("write Shmat failed");
    }else{
        return shmid;
    }
    return 0;
}


static int SharedMemoryInit(int kNum)
{
    int shmid;
    if((shmid = shmget((key_t)kNum, MEM_SIZE, IPC_CREAT| IPC_EXCL | 0666)) == -1) {
        //새로 만드는 경우
        //서버 측에서 이미 공유메모리 생성
        if((shmid = shmget((key_t)kNum, 0, 0))==-1)
        {
            //perror("write Shmat failed");
            return shmid;
        }
    }//클에서 만듦
    //printf("클 생성해서 연결 성공%d\n",kNum);  
    return shmid;
}


static int SharedMemoryWrite(int shmid, char *sMemory, int size)
{
    void *shmaddr;
    char sendtemp[MEM_SIZE];
    char* sendM = sendtemp;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){
        //printf("error\n");
        return 0;
    }

    sprintf(sendM,"%d%s",3,sMemory);
    sprintf((char*)shmaddr,"%s",sendM);
    printf("%s",(char*)shmaddr);

    //printf("after write :%s",(char*)shmaddr);
    if(shmdt(shmaddr) == -1)
    {
        perror("close write Shmdt failed");
        return 0;
    }
    return 1;
    
    
}


static int SharedMemoryRead(int shmid,char *sMemory)
{
    void *shmaddr;
    char recvtemp[MEM_SIZE];
    char* recvM = recvtemp;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){
        //printf("error\n");
        return 0;
    }
            /*
            0 - 아무도안함
            1- 서버 write
            2- 서버 read
            3- 클라이언트 write
            4- 클라이언트 read
            */
/*
strncpy(문자열저장변수, 잘라야할문자열 + 시작위치값, 자를사이즈);

요렇게 하면 된다.
포인트는 '+ 시작위치값'이다.
*/
    sprintf(recvM,"%s",(char*)shmaddr);
    if(recvM[0]=='1'){
        //sprintf(recvM,"%s",shmtemp);
        strncpy(sMemory,recvM+1,sizeof(recvM)-1);
    }else{
        return 0;
    }
    //sprintf(sMemory,"%s",recvM);
    printf("받은 문자열 :%s\n",sMemory);

    if(shmdt(shmaddr) == -1)
    {
        perror("close write Shmdt failed");
        return 0;
    }
    return 1;
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