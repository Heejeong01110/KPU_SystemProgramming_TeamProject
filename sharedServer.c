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
static int SharedMemoryManageRead(int shmid,char *sMemory);
static int SharedMemoryFree(int shmid);
static int SharedMemoryclean(int shmid);

int clientCnt=0;

void* filerecv(void* arg);
pthread_t thread[3];

int main(void){
    printf("프린트가 안떠요");
    
    
    struct threadArg * argument;
    char recvbuffer[MEM_SIZE];
    char byebuffer[MEM_SIZE]="bye";
    
    printf("create 전");
    
    int ManageShmid = SharedMemoryManageCreate(KEY_NUM);
    
    printf("read 전");
    
    fflush(stdout);
    while(1){
        SharedMemoryManageRead(ManageShmid,recvbuffer);

        //if(strcmp(recvbuffer,"3hello")==0 ){
        if(recvbuffer[0]=='3'){
            clientCnt++;
            printf("%s is hello\n",recvbuffer);
            //printf("서버클 연결 시작\n");
            for (int i = 0; i < 3; i++) {
                //printf("%d번째 스레드 시작\n",i);
                argument = (struct threadArg *)malloc(sizeof(struct threadArg));
	    	    argument->tshmid = clientCnt*10 + i+1;
		        argument->number=i+1;
                //printf("%d번째 스레드 직전\n",i);
                pthread_create(&thread[i], NULL, (void*)filerecv, (void *)argument);
            }
            //SharedMemoryWrite(ManageShmid,byebuffer,sizeof(byebuffer));
            
        }else{
            
            printf("%s is not hello\n",recvbuffer);
        }
        sleep(3);
        /*
        if(SharedMemoryRead(ManageShmid,recvbuffer)>4){
            clientCnt++;
            //printf("서버클 연결 시작\n");
            for (int i = 0; i < 3; i++) {
                printf("%d번째 스레드 시작\n",i);
                argument = (struct threadArg *)malloc(sizeof(struct threadArg));
	    	    argument->tshmid = clientCnt*10 + i+1;
		        argument->number=i+1;
                printf("%d번째 스레드 직전\n",i);
                pthread_create(&thread[i], NULL, (void*)filerecv, (void *)argument);
            }

            //SharedMemoryWrite(ManageShmid,0x00,MEM_SIZE);
            //memset(recvbuffer,0x00,MEM_SIZE);
            //SharedMemoryclean(ManageShmid);
            
            //break; 
        }
        else{
            //printf(",");      
        }*/
    }
    
    printf("test\n");
    


    
    sleep(4);
    /*
    
    for(int i=0;i<=3;i++)
     {
          pthread_join(thread[i],0);
     }
     SharedMemoryFree(ManageShmid);
     */

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
    //printf("thread 진입\n");
	/*3. 각 쓰레드는 1개의 shm 생성.*/
    int tid = SharedMemoryCreate(threadShmid);
	//printf("thread 생성 %d\n",threadShmid);
    //printf("read 전 %d\n",threadShmid);
    //client에서 보내는 파일 받기
    while(1)
    {
        SharedMemoryRead(tid,recvbuf);
        if(recvbuf[0]==0){
        }else{
           // printf("받은거  %d: %s\n",threadShmid, recvbuf);
            break;
        }
    }

    //변형파일 전송할예정
    sprintf(sendbuf,"%s 에다가 변형.",recvbuf+30);
    //printf("보낼거 : %s\n",sendbuf);
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

static int SharedMemoryclean(int shmid)
{
    void *shmaddr;
    char sendtemp[MEM_SIZE];
    char* sendM = sendtemp;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){
        printf("error\n");
        return 0;
    }
    /*
    0 - 아무도안함
    1- 서버 write
    2- 서버 read
    3- 클라이언트 write
    4- 클라이언트 read
    */
    //sprintf(sendM,"%d%s",1,sMemory);
    //sprintf((char*)shmaddr,"%s",sendM);
    memset(shmaddr,0x00,MEM_SIZE);

    //printf("보내진거 :%s\n",shmaddr);
    if(shmdt(shmaddr) == -1)
    {
        //printf("error3");
        return 0;
    }
    return 1;
}

 
static int SharedMemoryWrite(int shmid, char *sMemory, int size)
{
    void *shmaddr;
    char sendtemp[MEM_SIZE];
    char* sendM = sendtemp;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){
        printf("error\n");
        return 0;
    }
    /*
    0 - 아무도안함
    1- 서버 write
    2- 서버 read
    3- 클라이언트 write
    4- 클라이언트 read
    */
    printf(" 보낼거:%s\n",sMemory);
    sprintf(sendM,"%d%s",1,sMemory);
    sprintf((char*)shmaddr,"%s",sendM);

    printf("보내진거 :%s\n",shmaddr);
    if(shmdt(shmaddr) == -1)
    {
        //printf("error3");
        return 0;
    }
    return 1;
}


static int SharedMemoryRead(int shmid,char *sMemory){
    void *shmaddr;
    char recvtemp[MEM_SIZE];
    char* recvM = recvtemp;

    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){
        //printf("error1\n");
        return 0;
    }
    //printf("%s",(char*)shmaddr);
    sprintf(recvM,"%s",(char*)shmaddr);
    //printf("%s",recvM);
    if(recvM[0]=='3'){
        //sprintf(recvM,"%s",shmtemp);
        strncpy(sMemory,recvM+1,sizeof(recvM)-1);
        //printf("read3\n");
        //printf("받은 문자열 :%s\n",sMemory);
    }else{
        //printf("error2\n");
        return 0;
    }
    sprintf(sMemory,"%s",recvM);
    

    if(shmdt(shmaddr) == -1)
    {
        printf("error4");
        
        return 0;
    }
    return sizeof(sMemory);   
}




 
static int SharedMemoryManageRead(int shmid,char *sMemory){
    void *shmaddr;
    char recvtemp[MEM_SIZE];
    char* recvM = recvtemp;
    
    
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){
        //printf("error1\n");
        return 0;
    }
    //printf("%s",(char*)shmaddr);
    sprintf(recvM,"%s",(char*)shmaddr);
    //printf("%s",recvM);
    if(recvM[0]=='3'){
        //sprintf(recvM,"%s",shmtemp);
        strncpy(sMemory,recvM+1,sizeof(recvM)-1);
        //printf("read3\n");
        //printf("받은 문자열 :%s\n",sMemory);
    }else{
        //printf("error2\n");
        return 0;
    }
    sprintf(sMemory,"%s",recvM);
    sprintf((char*)shmaddr,"1bye%d",1);
    //fflush(shmaddr);
    if(shmdt(shmaddr) == -1)
    {
        printf("error4");
        
        return 0;
    }
    return sizeof(sMemory);   
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

