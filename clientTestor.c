#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <errno.h>
#include<sys/times.h>

#define CLK_TCK sysconf(_SC_CLK_TCK)

void error_handling(char * message)
{
	puts(message);
	exit(1);
}
int main(char argc, char * argv[])
{
	int i,num;
	pid_t pid;
	pid_t wpid;
	char filename[255];
	int status=0;

	clock_t start, end;
	struct tms starttms,endtms;
	time_t t;
	
	if(argc != 3)
	{
		printf("Usage: %s <program> <num>",argv[0]);
		exit(1);
	}
	//생성할 프로세스의 수
	num = atoi(argv[2]);
	
	if((start=times(&starttms))==-1)
	{
		printf("time error");
		perror("times 1");
		exit(1);
	}
	//프로세스 생성
	for(i=0;i<num;i++)
	{
		pid = fork();
		if(pid == -1){
			i--;
			continue;
		}
		else if(pid == 0){//클라이언트 프로세스로 변경
			printf("%d Child process: %d\n",i,getpid());
			
			close(0);
			close(1);
			
			execlp((const char *)argv[1],(const char *)argv[1],(char *)0);
			printf("Child execlp() faile\n");
			return 0;
		}
		else{
			continue;
		}

	}
	
	while ((wpid = wait(&status)) >= 0){}

	if((end=times(&endtms))==-1)
	{
		printf("time error");
		perror("times 1");
		exit(1);
	}
	printf("multi thread IPC real time: %.2f\n", ((double)(end-start)/CLK_TCK));
	printf("multi thread IPC child user time: %.2f\n", ((double)(endtms.tms_cutime-starttms.tms_cutime)/CLK_TCK));
	printf("multi thread IPC child sys time: %.2f\n", ((double)(endtms.tms_cstime-starttms.tms_cstime)/CLK_TCK));
  	printf("return Parent\n");
	//system("rm ./channel/* -i"); //절대 조심 파일 다 삭제됨
	return 0;

}

