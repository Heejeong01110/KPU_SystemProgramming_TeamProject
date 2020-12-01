#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 4096
int main(void)
{
	char buf[BUFSIZE];
	int orifd;
	int codefd = open("code.txt",O_WRONLY|O_TRUNC);
	int readlen;

	orifd = open("original.txt",O_RDONLY);
	codefd = open("code.txt",O_WRONLY|O_TRUNC);

	while((readlen=read(orifd,buf,BUFSIZE))>0){
		for(int i = 0 ; i < readlen;i++)
		{
			buf[i] += 1;
		}
		write(codefd,buf,readlen);
	}

	close(orifd);
	close(codefd);

}
