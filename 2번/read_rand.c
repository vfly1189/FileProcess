#include <stdio.h>
#include <sys/types.h>
#include <time.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

#define B_LEN 100
#define SUFFLE_NUM 10000

void GenRecordSequence(int *list, int n);
void swap(int *a, int *b);

int main(int argc, char **argv)
{
	struct timeval startTime, endTime;
	int fd;
	int *read_order_list;
	char buf[B_LEN];
	int num_of_records;

	if((fd = open(argv[1], O_RDONLY)) < 0)
	{
		fprintf(stderr, "open error for %s\n", argv[1]);
		exit(1);
	}

	num_of_records = SUFFLE_NUM;
	read_order_list = (int *)calloc(SUFFLE_NUM, sizeof(int));

	GenRecordSequence(read_order_list, num_of_records);

	gettimeofday(&startTime, NULL);
	for(int i=0; i<SUFFLE_NUM; i++)
	{
		lseek(fd, (off_t)read_order_list[i]*100, SEEK_SET);
		read(fd, buf, B_LEN);
	}

	gettimeofday(&endTime, NULL);
	long diffTime = endTime.tv_usec - startTime.tv_usec;

	printf("#records: %d timecost: %ld us\n", SUFFLE_NUM, diffTime);
	return 0;
}

void GenRecordSequence(int *list, int n)
{
	int i, j, k;

	srand((unsigned int)time(0));

	for(i=0; i<n; i++)
	{
		list[i] = i;
	}
	
	for(i=0; i<SUFFLE_NUM; i++)
	{
		j = rand() % n;
		k = rand() % n;
		swap(&list[j], &list[k]);
	}

	return;
}

void swap(int *a, int *b)
{
	int tmp;

	tmp = *a;
	*a = *b;
	*b = tmp;

	return;
}
