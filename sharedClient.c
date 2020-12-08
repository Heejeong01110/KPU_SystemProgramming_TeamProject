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
#include <fcntl.h>
#include <sys/sem.h>
#include <sys/stat.h>

#define  KEY_NUM   60043
#define THREADNUM 3
#define MAX_SIZE 4096
#define FILENAMESIZE 255
#define null NULL



void* filesend(void* th);
pthread_t thread[3];
long mypid; /*main에서 한 번 write하고 쓰레드에서는 읽기만 함으로 동기화 필요없음 */
struct stat sb; /*code.txt 파일의 상태를 저장함, 한 번 저장되면 읽기만 함으로 동기화 필요없음 */
long long afileSize; /*code.txt의 전체 파일 크기*/
pthread_cond_t printer1;
pthread_cond_t printer2;
pthread_cond_t printer3;
pthread_mutex_t printlock;
int endNum;

static int SharedMemoryInit(int kNum,long long fileSize);
static int SharedMemoryWrite(int shmid,char *sMemory);
static int SharedMemoryRead(int shmid,char *sMemory);
static int SharedMemoryFree(int shmid);

void error_handler(char * FileName){
    if(FileName!=null)
        unlink(FileName);
   pthread_exit(0);
}
struct threadArg{
    long long fileSize; //파일 사이즈
    int cCnt; //클라이언트 수
};

int main() {
    struct threadArg * argument;
    char buf[MAX_SIZE];/*서버에게 request를 전달하기위한 버퍼*/
    int protocol;
    long long fileS; /*스레드 하나 당 읽을 파일 크기*/

   memset(buf,0x00,MAX_SIZE);
   pthread_cond_init(&printer1,NULL);
   pthread_cond_init(&printer2,NULL);
   pthread_cond_init(&printer3,NULL);
   pthread_mutex_init(&printlock,NULL);
    /*1. open managefifo file 만약 파일이 없다면 {T}시간 동안 스핀하며 대기한다.*/
    
    while((protocol = open("./managefifo", O_APPEND | O_WRONLY)) < 0){
        //{T} 시간을 넘었는지 확인하고 넘었다면 프로세스 종료 
    }
    
    /*2. managefifo 파일에 "request {mypid} {파일 크기}"를 write */
    /*code.txt의 상태 읽기*/
    if (stat("./codeqq.txt", &sb) == -1){ 
        printf("fail to call stat(./code,txt)\n");
        exit(1);
    }

    afileSize = (long long)sb.st_size;
    mypid = (long)getpid();
    sprintf(buf,"request %ld %lld \n",mypid,afileSize);
    fileS = afileSize/3 + 1;

    
    if(write(protocol, buf, strlen(buf)) < 0 ){ 
        printf("fail to call write(protocol,request)\n");
        exit(1);
    }

    write(1,buf,strlen(buf)); //확인용 출력
    close(protocol);

    /*3. 3개의 쓰레드를 할당*/
    for(int i=0;i<THREADNUM;i++){
        argument = (struct threadArg *)malloc(sizeof(struct threadArg));
        argument->fileSize = fileS;
        argument->cCnt=i+1;
        pthread_create(&thread[i],NULL,(void *)filesend, (void *)argument);
    }


    for(int i=0;i<THREADNUM;i++){
        pthread_join(thread[i],NULL);
    }
    pthread_exit(1);
}

    

void* filesend(void* arg){ 
    struct threadArg * argument = (struct threadArg*)arg;
    //int n = argument->cCnt; //스레드
    int cCnt = argument->cCnt; //스레드
    long long fileSize = argument->fileSize; //파일사이즈
    int threadId = (int)mypid;
    int tshmId = threadId*100 + cCnt*10;

    char linkFileName[FILENAMESIZE];
    int linkFd;
    char buf[fileSize];
    int readlen; /*code.txt 를 읽은 길이*/
    int totalfilereadlen = 0; /*서버에게 받은 파일 길이*/
    long long rwpointer; /* read write pointer */
    int recvkey = 60040 + cCnt+THREADNUM;
    int sendkey = 60040 + cCnt;
    fflush(stdout);
    
    /*3. 각 스레드는 전송용, 수신용 sharedmemory를 생성.*/
    int stoctid = SharedMemoryInit(recvkey,fileSize);
    int ctostid = SharedMemoryInit(sendkey,fileSize);

    /*4. client의 각 쓰레드는 "code.txt"파일을 3분할하여 병행적으로 읽은 후 "{자신의 PID}FIFO{n}2ser" 파일에 writelock를 건 후 write한다.*/
    /* 파일을 병행적으로 읽는 방법 
        1. link()를 통해 링크파일을 만든다(링크파일은 read write pointer를 공유하지 않지만 원본 파일의 내용은 공유하기 때문에 파일을 병행적으로 읽는 것에 적절하다)
        2. 링크파일을 open한다.
        3. lseek()를 통해 적절한 위치로 read write pointer 이동시킨다.
    */
    
    sprintf(linkFileName,"./channel/codelink%ld_%d",mypid,cCnt);
    if(link("codeqq.txt",linkFileName)<0){
        printf("fail to call link(codeqq.txt,%s)\n",linkFileName);
        error_handler(linkFileName);
        exit(0);
    }
    
    if((linkFd=open(linkFileName,O_RDONLY))<0){
        printf("fail to call open(%s)\n",linkFileName);
        error_handler(linkFileName);
        exit(0);
    }

    rwpointer = (fileSize)*(cCnt-1);
    if(lseek(linkFd,rwpointer,SEEK_SET)<0){
        printf("fail to call lseek(%s)\n",linkFileName);
        error_handler(linkFileName);
        exit(0);
    }

    if((readlen=read(linkFd,buf,fileSize))<0){
        printf("fail to call read()\n");
        error_handler(linkFileName);
        exit(0);
    }
    /*5. 읽은 파일을 서버에게 전송*/
    SharedMemoryWrite(ctostid,buf);
    
    pthread_mutex_lock(&printlock);
    if(cCnt==1)
        printf("Thread1 Print Start\n");
    if((cCnt==2)){
        if(endNum!=1)
            pthread_cond_wait(&printer2,&printlock);
        printf("Thread2 Print Start\n");
    }
    else if((cCnt==3)){
        if((endNum!=2))
            pthread_cond_wait(&printer3,&printlock);
        printf("Thread3 Print Start\n");
    }
    /*6. 파일 read하기*/
    int totallen=0;
    while(1){
        if(SharedMemoryRead(stoctid,buf)){
            break;
        }
    }
    /*7. 파일 출력 */
    totallen+=write(1,buf,readlen);
    fflush(stdout);

    if(cCnt==1){
      endNum=1;
      pthread_cond_signal(&printer2);
    }
    if(cCnt==2){
      endNum=2;
      pthread_cond_signal(&printer3);
    }
    pthread_mutex_unlock(&printlock);


    printf("\nfilesize = %lld Thread %d, fileread: %d byte, rcv: %d byte\n",afileSize,cCnt,readlen, totallen);
    fflush(stdout);
    
    SharedMemoryFree(stoctid);
    SharedMemoryFree(ctostid);
    unlink(linkFileName);
}



static int SharedMemoryInit(int kNum,long long fileSize)
{
    int shmid;
    if((shmid = shmget((key_t)kNum, fileSize, IPC_CREAT| IPC_EXCL | 0666)) == -1) {
        //IPC_CREATE - key에 해당하는 메모리가 없으면 공유메모리를 생성한다.
        //IPC_EXCL - 공유메모리가 있으면 실패로 반환 
        /*서버 측에서 이미 생성했을 경우, 클라이언트는 attach만 실행*/
        if((shmid = shmget((key_t)kNum, 0, 0))==-1)
        {
            //perror("write Shmat failed");
            return shmid;
        }
    }
    /*클라이언트측에서 정상적으로 만든경우*/
    SharedMemoryWrite(shmid,"0");
    return shmid;
}


static int SharedMemoryWrite(int shmid, char *sMemory) //성공시 1 리턴
{
    void *shmaddr;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){

    }else{
        //write
        sprintf((char*)shmaddr,sMemory);

        if(shmdt(shmaddr) == -1){
            perror("close write Shmdt failed");
            return 0;
        }
        return 1;

    }
    return 0;
}


static int SharedMemoryRead(int shmid,char *sMemory) //성공시 1 리턴
{
    void *shmaddr;
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1){
    }
    else{
        //read
        sprintf(sMemory,shmaddr);
        //오류 방지를 위해 값을 0으로 초기화해준다.
        if(sMemory[0]=='0'){
            return 0;
        }else{
            sprintf(shmaddr,'\0');
            if(shmdt(shmaddr) == -1)
            {
                perror("close write Shmdt failed");
                return 0;
            }
            return 1;
        }
    }
    return 0;
}


static int SharedMemoryFree(int shmid)
{
    if(shmctl(shmid, IPC_RMID, 0) == -1) 
    {
        perror("Shmctl failed");
        return 1;
    }
    return 0;
}