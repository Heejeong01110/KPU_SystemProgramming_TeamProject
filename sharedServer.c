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
#include <sys/sem.h>
#include <signal.h>
#include <sys/stat.h>

#define KEY_NUM   60043 /*manage 공유메모리 키값 */
#define MAX_SIZE  4096
#define THREADPERWORK 3
#define null NULL


struct threadArg{
   int tshmid; //스레드 순서
    long long fileSize; //파일 사이즈
   int cCnt; //클라이언트 수
};
int worknum=0;
void requestPasing(char * request[3],char buf[])
{
    int cnt=0;
    request[0] = &buf[0];
    for(int i = 0;i<MAX_SIZE;i++){
        if(cnt>2)
           break;
       if(buf[i]==' '){
          buf[i]='\0';
          cnt++;
          request[cnt]=&buf[i+1];
       }
    }
}
void signalhandler(int sig) {
   unlink("./managefifo");
   exit(0);
}

void decoding(char code[],int len)
{
   for(int i = 0 ; i<len;i++){
      code[i] = code[i]-1;
   }
}
static int SharedMemoryCreate(int kNum,long long fileSize);
static int SharedMemoryWrite(int shmid, char *sMemory);
static int SharedMemoryRead(int shmid,char *sMemory);

void* filerecv(void* arg);
pthread_t thread[3000];
int readRequest(int fd, char buf[])
{
   int i = 0;
   int readlen;
   while((readlen=read(fd, &buf[i++], 1))>0){
      if(buf[i-1]=='\n'){
         return i-1;
      }
   }
   //printf("i = %d\n",i);
   return i-1;
}


int main(void){
    char buf[MAX_SIZE];
    int protocol;
    char * request[3];
   int readlen=0;
   struct threadArg * argument;
    printf("서버를 시작합니다.\n");
    /*1. server가 managefifo file을 생성한다.*/
    if(mkfifo("./managefifo",0666) == -1){ //fifo init
        printf("fail to make fifo manage()\n");
        exit(0);
    }
    signal(SIGINT, signalhandler);
    if((protocol = open("./managefifo", O_RDONLY)) < 0){ //waiting in client
       printf("fail to call open manage()\n");
        exit(0);
    }
   while(1){
        /*3. server는 managefifo에 있는 요청 메시지("request {자신의 PID} {파일 크기}")를 확인하면 응답을 위해 3개의 쓰레드를 할당하고*/
       
        
        if((readlen=readRequest(protocol, buf)) < 0 ){ 
          printf("fail to call read()");
            exit(0);
        }
       if(readlen<7) /*읽은 값이 "request"의 길이인 7보다 작으면 while문 재시작*/
          continue;
       requestPasing(request,buf);

        long long fileS = atoll(request[2])/3 + 1;
        
   
       for(int i = 0 ;i<THREADPERWORK;i++){
          argument = (struct threadArg *)malloc(sizeof(struct threadArg));
          argument->tshmid=atoi(request[1]);
          argument->fileSize=fileS;
            argument->cCnt= i+1;
          pthread_create(&thread[i+worknum*3],NULL,filerecv, (void *)argument); //실패 시 에러처리 추가
       }
        worknum++;
       memset(buf,0x00,MAX_SIZE);
       lseek(protocol,0,SEEK_SET);
    }
    close(protocol);
    unlink("./managefifo");
    pthread_exit(0);
}

void* filerecv(void * arg){
    struct threadArg * argument = (struct threadArg*)arg;
   int threadId = argument->tshmid; //프로세스
    long long fileSize = argument->fileSize;
    int cCnt = argument->cCnt; //스레드 

    int tshmId = threadId*100 +cCnt*10;
    
   int tempfd;
    char buf[fileSize];
   int readlen;

   /*3. 각 쓰레드는 1개의 shm 생성.*/
    int stoctid = SharedMemoryCreate(tshmId+1,fileSize);
    int ctostid = SharedMemoryCreate(tshmId+2,fileSize);

    int i=0;
    
    while(1){
        if(SharedMemoryRead(ctostid,buf)){
            break;
        }
    }
    decoding(buf,fileSize);
    
    SharedMemoryWrite(stoctid,buf); 

    if(cCnt==3){
        printf("...decoding 완료\n",cCnt);
        fflush(stdout);
    }
    pthread_exit(0);
}

static int SharedMemoryCreate(int kNum,long long fileSize) //성공시 shmid 리턴
{
    int shmid;
    if((shmid = shmget((key_t)kNum, fileSize, IPC_CREAT| IPC_EXCL | 0666)) == -1) {
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
    SharedMemoryWrite(shmid,"0");
    //서버측에서 정상적으로 만든경우
    return shmid;
}
 
static int SharedMemoryWrite(int shmid, char *sMemory)
{
    void *shmaddr;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){
    }else{
        //sprintf((char*)&shmaddr[MAX_SIZE*i],sMemory);
        sprintf((char*)shmaddr,sMemory);
        //printf("보내진거 :%s\n",shmaddr);
        if(shmdt(shmaddr) == -1)
        {
            printf("error\n");
            return 0;
        }
        return 1;
    }
    return 0;
}

 
static int SharedMemoryRead(int shmid,char *sMemory)
{
    void *shmaddr;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){}
    else{
        sprintf(sMemory,shmaddr);
        if(sMemory[0]=='0'){
            return 0;
        }else{
            sprintf(shmaddr,'\0');
            if(shmdt(shmaddr) == -1)
            {
                printf("error4");
                return 0;
            }
            return 1;
        }
    }
    return 0;
}