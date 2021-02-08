// 주의사항
// 1. sectormap.h에 정의되어 있는 상수 변수를 우선적으로 사용해야 함
// 2. sectormap.h에 정의되어 있지 않을 경우 본인이 이 파일에서 만들어서 사용하면 됨
// 3. 필요한 data structure가 필요하면 이 파일에서 정의해서 쓰기 바람(sectormap.h에 추가하면 안됨)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include "sectormap.h"
#include "fdevicedriver.c"

// 필요한 경우 헤더 파일을 추가하시오.

//
// flash memory를 처음 사용할 때 필요한 초기화 작업, 예를 들면 address mapping table에 대한
// 초기화 등의 작업을 수행한다. 따라서, 첫 번째 ftl_write() 또는 ftl_read()가 호출되기 전에
// file system에 의해 반드시 먼저 호출이 되어야 한다.
//

int mapping_table[DATAPAGES_PER_DEVICE];
int page_number_status[PAGES_PER_BLOCK * BLOCKS_PER_DEVICE];
int free_block_number;

typedef struct Sector
{
	int psn;
	struct Sector *next;
}S_Node;

S_Node *root = NULL;
S_Node *garbage_root = NULL;

void ftl_open()
{
	//
	// address mapping table 초기화
	// free block's pbn 초기화
    	// address mapping table에서 lpn 수는 DATABLKS_PER_DEVICE 동일
	
	for(int i=0; i<DATAPAGES_PER_DEVICE; i++)
		mapping_table[i] = -1;
	free_block_number = BLOCKS_PER_DEVICE - 1;

	S_Node *move = root;
	for(int i=0; i<DATAPAGES_PER_DEVICE; i++)
	{
		S_Node *temp = (S_Node *)calloc(1,sizeof(S_Node));
		temp->psn = i;

		if(root == NULL)
		{
			root = temp;
			move = temp;
		}
		else
		{
			move->next = temp;
			move = temp;
		}
	}

	for(int i=0; i<PAGES_PER_BLOCK * BLOCKS_PER_DEVICE; i++)
		page_number_status[i] = 0;
}

//
// 이 함수를 호출하기 전에 이미 sectorbuf가 가리키는 곳에 512B의 메모리가 할당되어 있어야 한다.
// 즉, 이 함수에서 메모리를 할당받으면 안된다.
//
void ftl_read(int lsn, char *sectorbuf)
{
	char *pagebuf = (char *)calloc(PAGE_SIZE, sizeof(char));
	dd_read(mapping_table[lsn], pagebuf);
	memcpy(sectorbuf, pagebuf, SECTOR_SIZE);
	return;
}


void ftl_write(int lsn, char *sectorbuf)
{	
	char *pagebuf = (char *)calloc(PAGE_SIZE, sizeof(char));
	memset(pagebuf, (char)0xff, PAGE_SIZE);

	SpareData temp;
	temp.lpn = lsn;
	char *p = pagebuf + SECTOR_SIZE;
	memcpy(p, &temp, SPARE_SIZE);

	if(mapping_table[lsn] == -1)
	{
		S_Node *tmp;

		char *buf = (char *)calloc(PAGE_SIZE, sizeof(char));
		if(root == NULL)
		{
			if(garbage_root == NULL)
			{
				printf("flash memory is full\n");
				return;
			}
			while(garbage_root != NULL)
			{
				int garbage_block = garbage_root->psn;

				for(int i=0; i<PAGES_PER_BLOCK; i++)
				{
					memset(buf, 0, PAGE_SIZE);
					if(page_number_status[garbage_block * PAGES_PER_BLOCK + i] == 1)
					{
						page_number_status[garbage_block * PAGES_PER_BLOCK + i] = 0;

						S_Node *new_page = (S_Node *)calloc(1,sizeof(S_Node));
						new_page->psn = free_block_number * PAGES_PER_BLOCK + i;
						new_page->next = NULL;

						if(root == NULL)
							root = new_page;
						else
						{
							new_page->next = root;
							root = new_page;
						}

						continue;
					}
					else
					{
						dd_read(garbage_block * PAGES_PER_BLOCK + i, buf);
						dd_write(free_block_number * PAGES_PER_BLOCK + i, buf);
						for(int j=0; j<DATAPAGES_PER_DEVICE; j++)
						{
							if(mapping_table[j] == garbage_block * PAGES_PER_BLOCK + i)
							{
								mapping_table[j] = free_block_number * PAGES_PER_BLOCK + i;
								break;
							}
						}
					}
				}
				dd_erase(garbage_block);
				free_block_number = garbage_block;
				S_Node *tmp2 = garbage_root;
				garbage_root = garbage_root->next;
			}

			S_Node *move = root;
		}


		int can_use_number = root->psn;
		tmp = root;
		root = root->next;
		free(tmp);

		mapping_table[lsn] = can_use_number;
		memcpy(pagebuf, sectorbuf, SECTOR_SIZE);
		
		dd_write(can_use_number, pagebuf);
	}
	else
	{
		char *buf = (char *)calloc(PAGE_SIZE, sizeof(char));
		if(root == NULL)
		{
			if(garbage_root == NULL)
			{
				printf("flash memory is full\n");
				return;
			}
			while(garbage_root != NULL)
			{
				int garbage_block = garbage_root->psn;

				for(int i=0; i<PAGES_PER_BLOCK; i++)
				{
					memset(buf, 0, PAGE_SIZE);
					if(page_number_status[garbage_block * PAGES_PER_BLOCK + i] == 1)
					{
						page_number_status[garbage_block * PAGES_PER_BLOCK + i] = 0;

						S_Node *new_page = (S_Node *)calloc(1,sizeof(S_Node));
						new_page->psn = free_block_number * PAGES_PER_BLOCK + i;
						new_page->next = NULL;

						if(root == NULL)
							root = new_page;
						else
						{
							new_page->next = root;
							root = new_page;
						}

						continue;
					}
					else
					{
						dd_read(garbage_block * PAGES_PER_BLOCK + i, buf);
						dd_write(free_block_number * PAGES_PER_BLOCK + i, buf);
						for(int j=0; j<DATAPAGES_PER_DEVICE; j++)
						{
							if(mapping_table[j] == garbage_block * PAGES_PER_BLOCK + i)
							{
								mapping_table[j] = free_block_number * PAGES_PER_BLOCK + i;
								break;
							}
						}
					}
				}
				dd_erase(garbage_block);
				free_block_number = garbage_block;
				S_Node *tmp2 = garbage_root;
				garbage_root = garbage_root->next;
			}

			S_Node *move = root;
		}

		S_Node *garbage = (S_Node *)calloc(1, sizeof(S_Node));
		page_number_status[mapping_table[lsn]] = 1;
		garbage->psn = mapping_table[lsn]/PAGES_PER_BLOCK;

		if(garbage_root == NULL)
			garbage_root = garbage;
		else 
		{
			S_Node *move = garbage_root;
			int flag = 0;
			while(move != NULL)
			{
				if(move->psn == garbage->psn) 
				{
					flag = 1; break;
				}
				move = move->next;
			}
			if(flag == 0)
			{
				garbage->next = garbage_root;
				garbage_root = garbage;
			}
		}

		S_Node *tmp;
		int can_use_number = root->psn;
		tmp = root;
		root = root->next;
		free(tmp);

		mapping_table[lsn] = can_use_number;
		memcpy(pagebuf, sectorbuf, SECTOR_SIZE);

		dd_write(can_use_number, pagebuf);
	}
	return;
}

void ftl_print()
{
	printf("lpn ppn\n");
	for(int i=0; i<DATAPAGES_PER_DEVICE; i++)
		printf("%d %d\n", i, mapping_table[i]);
	printf("free block's pbn : %d\n", free_block_number);
	return;
}
