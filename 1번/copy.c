#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#define BUFLEN 1024


int main(int argc, char* argv[])
{
	char buf[BUFLEN];
	int fd1, fd2;
	int length;

	if(argc != 3)
	{
		fprintf(stderr, "usage : %s <filein> <fileout>\n",argv[0]);
		exit(1);
	}

	if((fd1 = open(argv[1], O_RDONLY)) < 0)
	{
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}
	if((fd2 = open(argv[2], O_WRONLY|O_CREAT, 0644)) < 0)
	{
		fprintf(stderr, "open error for %s\n",argv[2]);
		exit(1);
	}

	while((length = read(fd1, buf, 100)) > 0)
		write(fd2, buf, length);
	exit(0);

}

