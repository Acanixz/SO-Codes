#include <stdio.h>
#include <unistd.h>

char aux = ' ';

int main()
{
	fork();

	aux = getchar();
	fork();

	aux = getchar();
	fork();
	
	aux = getchar();
	return 0;
}