#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#define MAX_SIZE 4096 /*파일 버퍼의 최대 사이즈*/
#define FILENAMESIZE 255 /*파일 이름의 최대 사이즈*/
#define THREADNUM 3 /*스레드 수*/
#define MKEY_NUM 60043 /*manage 공유메모리 키값 */

/* 이 함수를 사용하면 에러처리를 위해 프로그램을 exit() 시키기 전에 FIFO파일이나 link파일을 삭제시켜줄 수 있다.*/
void error_handler(char * linkFileName){
    unlink(linkFileName);
    exit(1);
}

void* filesend(void* th);
pthread_t thread[3];
static int ShmCreate(key_t key);
static int ShmWrite(char *shareddata);
static int ShmRead(char *sMemory);
static int ShmFree(void);


int main(){
    key_t manageKey = MKEY_NUM;
    int shmid;
    int shmaddr;
    struct stat sb; /*code.txt 파일의 상태를 저장함, 한 번 저장되면 읽기만 함으로 동기화 필요없음 */
    long long fileSize;
    int codeFd; /*code.txt 파일의 파일디스크립터*/
    

    /*1. manage sharedmemory 생성*/
    ShmCreate(manageKey);


    /*2. code.txt의 상태 읽기*/
    if (stat("./code.txt", &sb) == -1){ 
        printf("fail to call stat(./code,txt)\n");
        exit(1);
    }   
    fileSize = (long long)sb.st_size;

    /*3. 파일 내용 복사*/
    mypid = (long)getpid();
    sprintf(buf,"request %ld %lld ",mypid,fileSize);


    /*4. 3개의 쓰레드를 할당*/
    for(int i=0;i<THREADNUM;i++){
	//실패 시 에러처리 추가 
        pthread_create(&thread[i],NULL,(void *)filesend, (void *)(i+1));
    }

    /*5. 스레드 종료*/
    for(int i=0;i<THREADNUM;i++){
	//실패 시 에러처리 추가
          pthread_join(thread[i],NULL);
    }

    pthread_exit(1);
    return 0;
}



void* filesend(void* arg){
    int n = (int)arg; /*스레드 고유번호*/
    key_t threadKey;  /*스레드가 사용할 shared memory*/
    char fileName[FILENAMESIZE];
    char linkFileName[FILENAMESIZE];
    int shmid;
    int linkFd;
    char buf[MAX_SIZE];
    
    long long rwpointer; /* read write pointer */
    /*server1. 각 스레드마다 사용할 공유메모리 생성*/
    /*server2. threadSharedMemory 연결*/
    sprintf(fileName,"%ld%d",mypid,n);
    threadKey = atoi(fileName);

    ShmCreate(threadKey);

    /*2. 파일 send*/
    //ShmWrite(char *shareddata, int size)


    /* 파일을 병행적으로 읽는 방법 
	1. link()를 통해 링크파일을 만든다(링크파일은 read write pointer를 공유하지 않지만 원본 파일의 내용은 공유하기 때문에 파일을 병행적으로 읽는 것에 적절하다)
	2. 링크파일을 open한다.
	3. lseek()를 통해 적절한 위치로 read write pointer 이동시킨다.
    */

    /*2-1. 파일 디스크립터 연결*/
    sprintf(linkFileName,"codelink%ld_%d",mypid,n);
    if(link("code.txt",linkFileName)<0){
        printf("fail to call link(code.txt,%s)\n",linkFileName);
        error_handler(linkFileName);
    }

    /*2-2. 파일 디스크립터 read로 오픈*/
    if((linkFd=open(linkFileName,O_RDONLY))<0){
        printf("fail to call open(%s)\n",linkFileName);
        error_handler(linkFileName);
    }

    rwpointer = (fileSize/THREADNUM)*(n-1); /*파일을 스레드의 크기로 나누어 스레드 고유번호 부분을 point한다.*/
    if(lseek(linkFd,rwpointer,SEEK_SET)<0){
        printf("fail to call lseek(%s)\n",linkFileName);
        error_handler(linkFileName);
    }

    for(int i = 0 ;i < (fileSize/THREADNUM)/BUF_SIZE;i++){
        if(read(linkFd,buf,BUF_SIZE)<0){
            printf("fail to call read()\n");
            error_handler(linkFileName);
        }
        /*4. 암호화 파일 보내기*/
        /*
	    if(write(fifoFd,buf,BUF_SIZE)<0)
        {
            printf("fail to call write()\n");
            error_handler(linkFileName);
        }else
        {
		    printf("send to %s: %s\n",fileName,buf);
	    }
        */
    }
    /*5. 복호화된 파일 읽기*/
    /*
    if(read(linkFd,buf,(fileSize/THREADNUM)%BUF_SIZE)<0){
        printf("fail to call read()\n");
        error_handler(linkFileName);
    }
    */

    /*병렬적으로 보내기(?)*/
    /*
    if(write(fifoFd,buf,(fileSize/THREADNUM)%BUF_SIZE)<0)
    {
        printf("fail to call read()\n");
        error_handler(linkFileName);
    }
    else{
	    printf("send to %s: %s\n",fileName,buf);
    }
    */

    /*6. request END 추가*/
    if(write(fifoFd,"request END",11)<0){
        printf("fail to call read()\n");
        error_handler(linkFileName);
    }
    else{
	    printf("send to %s: request END\n",fileName,buf);
    }



    if(unlink(linkFileName)<0){
        printf("fail to call unlink(%s)\n",linkFileName);
        error_handler(linkFileName);
    }

}


static int ShmCreate(key_t key)
{
    if((shmid = shmget(key, MAX_SIZE, IPC_CREAT| IPC_EXCL | 0666)) == -1) {
        
        //IPC_CREATE - key에 해당하는 메모리가 없으면 공유메모리를 생성한다.
        //IPC_EXCL - 공유메모리가 있으면 실패로 반환
        printf("이미 존재하는 공유메모리 key입니다\n");  

        shmid = shmget(key, MAX_SIZE, IPC_CREAT| 0666);
        if(shmid == -1)
        {
            printf("공유메모리 할당 오류\n");
            return 1;
        }
        else
        {
            ShmFree();
            shmid = shmget(key, MAX_SIZE, IPC_CREAT| 0666);
            
            if(shmid == -1)
            {
                perror("Shared memory create fail");
                return 1;
            }
        }
    }
    
    return 0;
}
 
static int ShmWrite(char *shareddata) //고치기
{
    void *shmaddr;
    
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1) 
    //공유메모리 연결
    {
        perror("Shmat failed");
        return 1;
    }
    

    memcpy((char *)shmaddr, shareddata, size);//메모리에 값 입력
    
    if(shmdt(shmaddr) == -1) 
    //프로세스에서 공유메모리 분리
    {
        perror("Shmdt failed");
        return 1;
    }
    return 0;
}
 
static int ShmRead(char *sMemory)
{
    void *shmaddr;
    char mess[MEM_SIZE] = {0};
    
    if((shmaddr = shmat(shmid, (void *)0, 0)) == (void *)-1)
    //공유메모리 연결
    {
        perror("Shmat failed");
        return 1;
    }
    
    memcpy(sMemory, (char *)shmaddr, sizeof(mess)); //메모리에서 값 읽기
    
    if(shmdt(shmaddr) == -1)
    //프로세스에서 공유메모리 분리
    {
        perror("Shmdt failed");
        return 1;
    }
    return 0;
}
 
static int ShmFree(void)
{
    if(shmctl(shmid, IPC_RMID, 0) == -1) //공유메모리 제거
    {
        perror("Shmctl failed");
        return 1;
    }
    
    printf("Shared memory end");
    return 0;
}

