#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#define BUFLEN 1024

int main(int argc, char* argv[])
{
	char buf[BUFLEN];
	int fd1, fd2;
	int length;

	if(argc != 3)
	{
		fprintf(stderr, "usage : %s <file1> <file2>\n",argv[0]);
		exit(1);
	}

	if((fd1 = open(argv[1], O_WRONLY|O_APPEND)) < 0)
	{
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	if((fd2 = open(argv[2], O_RDONLY)) < 0)
	{
		fprintf(stderr, "open error for %s\n", argv[2]);
		exit(1);
	}

	while((length = read(fd2, buf, BUFLEN)) > 0)
		write(fd1, buf, length);
	exit(0);





}
