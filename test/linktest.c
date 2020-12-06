#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

int main(void)
{
	int fd,fd2;
	char buf[4096];
	link("code.txt","code1.txt");
	link("code.txt","code2.txt");
	fd = open("code1.txt",O_RDONLY);
	read(fd,buf,4096);
	printf("%s",buf);
	fd2 = open("code1.txt",O_RDONLY);
	read(fd2,buf,4096);
	printf("%s",buf);
	unlink("code1.txt");
	unlink("code2.txt");
}

