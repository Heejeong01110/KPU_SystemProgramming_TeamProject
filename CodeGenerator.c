#include<stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BUFSIZE 4096
int main(void)
{
	char buf[BUFSIZE];
	
	int fd = open("code.txt",O_WRONLY);
	for(int i = 0 ; i < BUFSIZE-1;i++)
	{
		buf[i] = 'a';
		//buf[i] *= 2;
	}
	buf[BUFSIZE-1] = '\n';
	write(fd,buf,BUFSIZE);
	for(int i = 0 ; i < BUFSIZE-1;i++)
	{
		buf[i] = 'b';
		//buf[i] *= 2;
	}
	buf[BUFSIZE-1] = '\n';
	write(fd,buf,BUFSIZE);

	for(int i = 0 ; i < BUFSIZE-1;i++)
	{
		buf[i] = 'c';
		//buf[i] *= 2;
	}
	buf[BUFSIZE-1] = '\n';
	write(fd,buf,BUFSIZE);

	close(fd);

}
