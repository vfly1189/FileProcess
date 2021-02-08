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
	int d_byte;

	if(argc != 4)
	{
		fprintf(stderr, "usage : %s <file> <offset> <delete_byte>\n",argv[0]);
		exit(1);
	}

	if((fd1 = open(argv[1], O_RDONLY)) < 0)
	{
		fprintf(stderr, "open error for %s\n",argv[1]);
		exit(1);
	}

	if((fd2 = open("tmp.txt", O_RDWR|O_CREAT|O_TRUNC,0644)) < 0)
	{
		fprintf(stderr, "open error for %s\n", "tmp.txt");
		exit(1);
	}

	offset = atoi(argv[2]);
	d_byte = atoi(argv[3]);

	char tmp;
	int i;
	for(i=0; i<offset; i++)
	{
		read(fd1,&tmp,1);
		write(fd2,(char*)&tmp,1);
	}

	lseek(fd1, d_byte, SEEK_CUR);

	while((length = read(fd1, buf, BUFLEN)) > 0)
		write(fd2, buf, length);


	if((fd1 = open(argv[1], O_WRONLY|O_TRUNC)) < 0)
	{
		fprintf(stderr, "open error for %s\n",argv[1]);
		exit(1);
	}

	lseek(fd2, 0, SEEK_SET);
	while((length = read(fd2, buf, BUFLEN)) > 0)
		write(fd1, buf, length);
	



	close(fd1);
	close(fd2);

	exit(0);
}
