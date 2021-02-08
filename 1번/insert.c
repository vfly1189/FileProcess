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
	int fd1,fd2;
	int length;
	int offset;

	if(argc != 4)
	{
		fprintf(stderr, "usage : %s <file> <offset> <data>\n",argv[0]);
		exit(1);
	}

	if((fd1 = open(argv[1], O_RDWR)) < 0)
	{
		fprintf(stderr, "open error for %s\n",argv[1]);
		exit(1);
	}

	

	offset = atoi(argv[2]);
	lseek(fd1, offset, SEEK_SET);

	if((fd2 = open("tmp.txt", O_RDWR|O_CREAT|O_TRUNC, 0644)) < 0)
	{
		fprintf(stderr, "open error for temp.txt\n");
		exit(1);
	}

	while((length = read(fd1, buf, BUFLEN)) > 0)
		write(fd2, buf, length);

	lseek(fd1, offset, SEEK_SET);
	int i=0;
	for(i=0; argv[3][i] != '\0'; i++)
		buf[i] = argv[3][i];
	buf[i] = 0;
	write(fd1, buf, i);

	lseek(fd2, 0, SEEK_SET);
	while((length = read(fd2, buf, BUFLEN)) > 0)
		write(fd1, buf, length);


	close(fd1);
	close(fd2);
	exit(0);
}
