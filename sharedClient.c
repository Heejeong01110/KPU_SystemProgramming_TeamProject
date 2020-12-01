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

    sprintf(sendbuf,"abcdefghijklmnopqrstuvwzyz%d",threadShmid);
    int tid = SharedMemoryInit(threadShmid);
    SharedMemoryWrite(tid,sendbuf,sizeof(sendbuf));
    printf("%d 전송\n",threadShmid);

    //변형된 파일 받을 예정
    //변형된 파일이 안받아지는 이유
    /*
    1. 공유메모리가 작성하고 버퍼를 뽑아가는 느낌이 아니라
    말그대로 공유하기 때문에 위에서 write한 정보가 그대로 써있고,
    서버에서 수정되지 않은 상태에서 받으니까 그냥 원래있던게 그대로 출력되는걸로 보임
    해결방법
    1. 맨 앞에 수정했다는 커맨드를 확인할 수 있도록 recvbuf[0] 자리에 어떠한 신호를 준다.
        프로그램은 단순하지만 확인하려면 계속 while문을 돌려야하므로 부하가 클수있음
        다른 기법이랑 통일감이 있는 편
    2. 시그널 이용해서 write가 실행되었을 때 반대쪽에서 read할 수 있도록 쏴주기 만들어보기 
        시그널 찾아봐야하지만 적용하면 훨씬 구조적으로 완벽함
        다른 기법이랑 통일감 없음
        2-1. while무한루프 붙는 부분
            read할 때 상대방이 작성 했나 안했나 무한정 대기 이거 하나
    */
    sleep(2);
    while(1)
    {
        SharedMemoryRead(tid,recvbuf);
        if(recvbuf[0]==0){
            
        }else{
            printf("success recv %d: %s\n",threadShmid, recvbuf);
            break;
        }
    }

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
    //printf("before write :%s",sMemory);
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
    return 0;
}


static int SharedMemoryRead(int shmid,char *sMemory)
{
    void *shmaddr;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){}
        else{
            printf("받은버퍼 :%s\n",(char*)shmaddr);
            sprintf(sMemory,shmaddr);
            printf("받은 문자열 :%s\n",sMemory);
            if(shmdt(shmaddr) == -1)
            {
                perror("close write Shmdt failed");
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