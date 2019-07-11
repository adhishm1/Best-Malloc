#include <unistd.h>	
#include <string.h>
#include <pthread.h>
#include <stdio.h>



union header {
	struct {
		size_t size;
		unsigned is_free;
		union header *next;
	} s;
	
	char ADS[16];
};
typedef union header header_t;

header_t *head = NULL, *tail = NULL;
pthread_mutex_t sema4;

header_t *get_free_block(size_t size)
{
size_t min=2000;
	header_t *curr = head;
	while(curr) {
		
		if (curr->s.is_free && curr->s.size >= size)
			{
				if(min>curr->s.size)
				min=curr->s.size;
			}
		curr = curr->s.next;
	}
curr=head;
while(curr) {
		
		if (curr->s.size == size)
			{
				return curr;
			}
		curr = curr->s.next;
	}


	return NULL;
}

void free(void *block)
{
	header_t *header, *tmp;

	void *programbreak;

	if (!block)
		return;
	pthread_mutex_lock(&sema4);
	header = (header_t*)block - 1;

	programbreak = sbrk(0);

	
	if ((char*)block + header->s.size == programbreak) {
		if (head == tail) {
			head = tail = NULL;
		} else {
			tmp = head;
			while (tmp) {
				if(tmp->s.next == tail) {
					tmp->s.next = NULL;
					tail = tmp;
				}
				tmp = tmp->s.next;
			}
		}
	
		sbrk(0 - header->s.size - sizeof(header_t));
	
		pthread_mutex_unlock(&sema4);
		return;
	}
	header->s.is_free = 1;
	pthread_mutex_unlock(&sema4);
}



void *b_malloc(size_t size)
{
	size_t total_size;
	void *block;
	header_t *header;
	if (!size)
		return NULL;
	pthread_mutex_lock(&sema4);
	header = get_free_block(size);
	if (header) {

		header->s.is_free = 0;
		pthread_mutex_unlock(&sema4);
		return (void*)(header + 1);
	}
	
	total_size = sizeof(header_t) + size;
	block = sbrk(total_size);
	if (block == (void*) -1) {
		pthread_mutex_unlock(&sema4);
		return NULL;
	}
	header = block;
	header->s.size = size;
	header->s.is_free = 0;
	header->s.next = NULL;
	if (!head)
		head = header;
	if (tail)
		tail->s.next = header;
	tail = header;
	pthread_mutex_unlock(&sema4);
	return (void*)(header + 1);
}

void *calloc(size_t num, size_t nsize)
{
	size_t size;
	void *block;
	if (!num || !nsize)
		return NULL;
	size = num * nsize;
	
	if (nsize != size / num)
		return NULL;
	block = b_malloc(size);
	if (!block)
		return NULL;
	memset(block, 0, size);
	return block;
}

void *realloc(void *block, size_t size)
{
	header_t *header;
	void *ret;
	if (!block || !size)
		return b_malloc(size);
	header = (header_t*)block - 1;
	if (header->s.size >= size)
		return block;
	ret = b_malloc(size);
	if (ret) {
		
		memcpy(ret, block, header->s.size);
		
		free(block);
	}
	return ret;
}
void block_checker()
{
	printf("\n");
	header_t *curr=head;
	while(curr)
	{
		printf("Size of block is %ld\t is_free? %d\n",curr->s.size,curr->s.is_free);
	curr=curr->s.next;
	}
	
}
int main()
{
//int a=10;
int *p=(int*) b_malloc(48);
p[0]=10;
p[1]=20;
p[3]=30;
p[4]=40;
p[5]=60;
//printf(sizeof(a));
printf("%d",p[0]);
printf("%d",p[1]);
printf("%d",p[3]);

int *a=(int*)b_malloc(30);
//free(a);
int *b=(int*)b_malloc(16);

b[0]=101;
printf("%d",b[0]);
int *c=(int*)b_malloc(20);
int *d=(int*)b_malloc(32);

/*after freeing a,b and c when we require memory of 20 bytes, then memory previous
ly allcoated to c should be allocated instead of a
according to fisrt fit a should get allocated4
but according to best fit c should get allcoated*/
free(a);
free(b);
free(c);
int *e=(int*)b_malloc(20);
block_checker();


}
