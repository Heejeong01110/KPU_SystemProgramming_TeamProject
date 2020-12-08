#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h> 
#include <sys/msg.h> 
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <semaphore.h>
#include <errno.h>
#define MSG_SIZE 800000
#define THREADNUM 3
#define BUF_SIZE 4096
#define FILENAMESIZE 255
void* filesend(void* arg);
pthread_t thread[3];
long mypid; /*main에서 한 번 write하고 쓰레드에서는 읽기만 함으로 동기화 필요없음 */
struct stat sb; /*code.txt 파일의 상태를 저장함, 한 번 저장되면 읽기만 함으로 동기화 필요없음 */
long long fileSize;
int cnt;
pthread_cond_t printer1;
pthread_cond_t printer2;
pthread_cond_t printer3;
pthread_mutex_t printlock;
int endNum;
struct msgbuf { 
    long mtype; /* message type, must be > 0 */ 
    char mtext[BUF_SIZE]; /* message data */
};

//sem_t printer;
//int codeFd; /*code.txt 파일의 파일디스크립터*/
/*
typedef struct tTHREAD
{
   char buff[BUF_SIZE];
   int cnt;
}THREAD;
*/
void error_handler(char* linkFileName) {
   //  unlink(linkFileName);
   perror("handler err\n");
   exit(1);
}
//THREAD sendbuf[3];
//THREAD recvbuf[3];
/*void open_Thread() {
   if ((filedesth = open(threadpathTemp, O_WRONLY)) < 0) {
      printf("fail to call fifo thread() %s\n", threadpathTemp);
      exit(1);
   }
}*/
/*void snd_Data() {
   if (msgsnd(key_id2, dmsg, MSG_SIZE) == -1) { //fifo rw
      printf("fail to call msgsnd()\n");
      exit(1);
   }
}*/
/*void read_Data() {
   if (msgrcv(key_id, msg, MSG_SIZE, getpid(), 0) == -1) {
      perror("msgrcv error");
      exit(1);
   }
}*/
//client
int main()
{
   char buf[BUF_SIZE];
   struct flock fifo_lock;
   int protocol;
   //int filedesmf;
   //int cnt;
   //int nread;
   fifo_lock.l_type = F_WRLCK;
   fifo_lock.l_whence = SEEK_SET;
   fifo_lock.l_start = 0;
   fifo_lock.l_len = 52254;
   memset(buf, 0x00, BUF_SIZE);
   pthread_cond_init(&printer1,NULL);
   pthread_cond_init(&printer2,NULL);
   pthread_cond_init(&printer3,NULL);
   pthread_mutex_init(&printlock,NULL);
   //sem_init(&printer, 0, 0);
   while((protocol = open("./managefifo", O_APPEND | O_WRONLY)) < 0){
        //{T} 시간을 넘었는지 확인하고 넘었다면 프로세스 종료 
   }
/*
   key_id = msgget((key_t)60041, IPC_CREAT | 0666); //recv queue
   key_id2 = msgget((key_t)60042, IPC_CREAT | 0666); //send queue
   if (key_id < 0 || key_id2 < 0) {
      perror("msgget error");
      exit(1);
   }
*/
   if (stat("./code.txt", &sb) == -1) {
      printf("fail to call stat(./code,txt)\n");
      exit(1);
   }
   fileSize = (long long)sb.st_size;
   mypid = (long)getpid();
   snprintf(buf,FILENAMESIZE-1, "request %ld %lld ", mypid, fileSize);
   // open managefifo file
   if (fcntl(protocol,F_SETLKW,&fifo_lock)==-1){
      printf("fail to call fcntl()\n");
      //error_handler(linkFileName);
      exit(0);
   }
   if(write(protocol, buf, strlen(buf)) < 0 ){ 
        printf("fail to call write(protocol,request)\n");
        exit(1);
    }
   fifo_lock.l_type = F_UNLCK;
   if(fcntl(protocol,F_SETLK,&fifo_lock)==-1){
      printf("fail to call write()\n");
              //error_handler(linkFileName);
   }
   write(1,buf,strlen(buf));
    close(protocol);
   /*if ((nread = read(filedesmf, msg, MSG_SIZE)) < 0) { //cnt
      printf("fail to call read manage()\n");
      exit(1);
   }
   */
   //thread 3
   for (int i = 0; i < THREADNUM; i++) {
      cnt = i + 1;//1,2,3 pass
      pthread_create(&thread[i], NULL, (void*)filesend, (void*)(cnt));
      printf("쓰레드 %d 생성 완료 \n", i);
      //sleep(1);
   }
   for (int i = 0; i <= THREADNUM; i++)
   {
      pthread_join(thread[i], NULL);
      //sem_destroy(&printer);
   }
   memset(buf, 0x00, BUF_SIZE);
   pthread_exit(1);
}
void* filesend(void* arg) { //n = file source
   int n = (int)arg;
   key_t sendkey, recvkey;
   char fileName[FILENAMESIZE];
   char linkFileName[FILENAMESIZE];
   int fifoFd;
   int linkFd;
   int readlen;
   char buf[BUF_SIZE];
   char buf2[BUF_SIZE];
   long long rwpointer; /* read write pointer */
   struct msgbuf mybuf;
   sendkey = msgget((key_t)60040 + (key_t)n, IPC_CREAT | 0666); //recv queue
   recvkey = msgget((key_t)60040 + (key_t)(n+THREADNUM), IPC_CREAT | 0666); //recv queue
   printf("sendkey: %d, recvkey: %d, thread: %d\n",sendkey,recvkey,n);
   if (sendkey < 0 || recvkey < 0) {
      perror("msgget error");
      exit(1);
   }
   printf("%d code read start()\n",n);
   snprintf(linkFileName, FILENAMESIZE-1,"codelink%ld_%d", mypid, n);
   if (link("code.txt", linkFileName) < 0) {
      printf("fail to call link(code.txt,%s)\n", linkFileName);
      error_handler(linkFileName);
   }
   if ((linkFd = open(linkFileName, O_RDONLY)) < 0) {
      printf("fail to call open(%s)\n", linkFileName);
      error_handler(linkFileName);
   }
   rwpointer = (fileSize / THREADNUM) * (n - 1);
   if (lseek(linkFd, rwpointer, SEEK_SET) < 0) {
      printf("fail to call lseek(%s)\n", linkFileName);
      error_handler(linkFileName);
   }
   printf("%d send buf start(), fileSize = %d, i size = %d\n",n,fileSize,(fileSize / THREADNUM) / BUF_SIZE);
   for (int i = 0; i < (fileSize / THREADNUM) / BUF_SIZE; i++) {
      if ((readlen=read(linkFd, buf, BUF_SIZE)) < 0) {
         printf("fafil to call read()\n");
         error_handler(linkFileName);
      }
      //printf("%s\n", buf);
      mybuf.mtype = 1;
      sprintf(mybuf.mtext,"%s",buf);
   mybuf.mtext[readlen]='\0';
      if (msgsnd(sendkey, (void*)&mybuf, readlen, 0) == -1) { //fifo rw
         printf("fail to call msgsnd()\n");
         printf("errno :%d\n",errno);
         exit(1);
      }
      printf("\nthread%d send success\n", n);
      printf("%s\n", mybuf.mtext);
   }
   int writeLen = (fileSize/THREADNUM)%BUF_SIZE;
   if(writeLen>0){
       if ((readlen=read(linkFd, buf,writeLen)) < 0) {
         printf("fafil to call read()\n");
         error_handler(linkFileName);
      }
      mybuf.mtype = 1;
      sprintf(mybuf.mtext,"%s",buf);
   mybuf.mtext[readlen]='\0';
      if (msgsnd(sendkey, (void*)&mybuf, writeLen, 0) == -1) { //fifo rw
         printf("fail to call msgsnd()\n");
         printf("errno :%d\n",errno);
         exit(1);
      }
      printf("\nthread%d send success\n", n);
      //printf("%s\n", mybuf.mtext);
   }
   
   printf("%d send buf end()\n",n);
   
      pthread_mutex_lock(&printlock);
   if(n==1)
      printf("\n\nThread1 %dkey msg Print Start\n\n",recvkey);
   if((n==2)){
      if(endNum!=1)
         pthread_cond_wait(&printer2,&printlock);
      printf("\n\nThread2 %dkey Print Start\n\n",recvkey);
   }
   else if((n==3)){
      if((endNum!=2))
         pthread_cond_wait(&printer3,&printlock);
      printf("\n\nThread3 %dkey Print Start\n\n",recvkey);
   }
   int totallen=0;
   /*while((readlen=msgrcv(recvkey, (void*)& buf2, BUF_SIZE, 0, 0))>0){
      totallen+=write(1,buf2,readlen);
   }*/
   int readNum = (fileSize/THREADNUM)/BUF_SIZE;
   printf("filesize : %d , readNum : %d\n",fileSize,readNum);

   for(int i = 0; i <readNum; i++ ){
   if((readlen= msgrcv(recvkey, (void*)&mybuf, BUF_SIZE, 0, 0))>0){
      totallen+=write(1,mybuf.mtext,readlen);
      }
   }
   for (int i = 0; i < (fileSize / 3) / BUF_SIZE; i++) {
      if ((readlen=msgrcv(recvkey, (void*)&mybuf, BUF_SIZE, 0, MSG_NOERROR)) < 0) {
         printf("fafil to call read()\n");
   exit(0);
      }
   mybuf.mtext[BUF_SIZE-1]='\0';
         write(1, mybuf.mtext, readlen);      
   }
   
   readlen = (fileSize/3)%BUF_SIZE;
   if (msgrcv(recvkey, (void*)&mybuf, readlen, 0, MSG_NOERROR) == -1) {
      perror("msgrcv error");
      exit(1);
   }
   mybuf.mtext[readlen-1]='\0';
   write(1, mybuf.mtext, readlen);

   fflush(stdout);
   if(n==1){
      endNum=1;
      pthread_cond_signal(&printer2);
   }
   if(n==2){
      endNum=2;
      pthread_cond_signal(&printer3);
   }
   pthread_mutex_unlock(&printlock);
      unlink(linkFileName);
      
   msgctl(recvkey,IPC_RMID, 0);
   msgctl(sendkey,IPC_RMID, 0);
}