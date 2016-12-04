#include <stdlib.h>
#include <stdio.h>

unsigned long __slab_max_size = 4096;

static int print_json(void * ignore,const char * buffer,size_t len)
{
	fputs(buffer, stdout);

	return(0);
}

int
main(int argc,char ** argv)
{
	int num_allocations = 1000;
	int max_allocation_size = 8192;
	int random_free_percentage = 33;
	char ** allocation_table;
	char * allocation;
	int i;

	allocation_table = malloc(sizeof(*allocation_table) * num_allocations);
	if(allocation_table == NULL)
		exit(EXIT_FAILURE);

	srand(1);

	printf("/* Allocating %d random length fragments of memory (maximum size = %ld bytes). */\n", num_allocations, max_allocation_size);

	for(i = 0 ; i < num_allocations ; i++)
	{
		allocation = malloc(1 + (rand() % max_allocation_size));
		if(allocation == NULL)
			exit(EXIT_FAILURE);

		allocation_table[i] = allocation;
	}

	__get_slab_stats(NULL, print_json);

	printf("\n/* Changing all allocations to different random lengths. */\n");

	for(i = 0 ; i < num_allocations ; i++)
	{
		allocation = realloc(allocation_table[i], 1 + (rand() % max_allocation_size));
		if(allocation == NULL)
			exit(EXIT_FAILURE);

		allocation_table[i] = allocation;
	}

	__get_slab_stats(NULL, print_json);

	printf("\n/* Freeing %d%% of all allocations. */\n", random_free_percentage);

	for(i = 0 ; i < num_allocations ; i++)
	{
		if((rand() % 100) < 33)
		{
			free(allocation_table[i]);
			allocation_table[i] = NULL;
		}
	}

	__get_slab_stats(NULL, print_json);

	printf("\n/* Marking unused slabs for reuse; reallocating memory/changing allocation lengths. */\n");

	__decay_unused_slabs();

	for(i = 0 ; i < num_allocations ; i++)
	{
		allocation = realloc(allocation_table[i], 1 + (rand() % max_allocation_size));
		if(allocation == NULL)
			exit(EXIT_FAILURE);

		allocation_table[i] = allocation;
	}

	__get_slab_stats(NULL, print_json);

	printf("\n/* Freeing all unused slabs. */\n");

	__free_unused_slabs();

	__get_slab_stats(NULL, print_json);

	return(EXIT_SUCCESS);
}
