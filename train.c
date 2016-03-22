/*
 * train.c
 */
 
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include "train.h"
#include "queue.h"
 
/* A global to assign IDs to our trains */ 
int idNumber = 1;

/* If this value is set to 1, trains lengths
 * etc will be generated randomly.
 * 
 * If it is set to 0, the lengths etc will be
 * input from a file.
 */
int doRandom = 0;

/* The file to input train data from */
FILE *inputFile;

/* You can assume that no more than 80 characters
 * will be on any line in the input file
 */
#define MAXLINE		80

void	initTrain ( char *filename )
{
	doRandom = 0;
	
	/* If no filename is specified, generate randomly */
	if ( !filename )
	{
		doRandom = 1;
		srandom(getpid());
	}
	else
	{
		/* remove this line and add your code here */

		inputFile = fopen(filename,"r");
		//printf ("File input not implemented.\n");
	}
}
 
/*
 * Allocate a new train structure with a new trainId, trainIds are
 * assigned consecutively, starting at 0
 *
 * Either randomly create the train structures or read them from a file
 *
 * This function malloc's space for the TrainInfo structure.  
 * The caller is responsible for freeing it.
 */
TrainInfo *createTrain ( void )
{
	TrainInfo *info = (TrainInfo *)malloc(sizeof(TrainInfo));

	/* I'm assigning the random values here in case
	 * there is a problem with the input file.  Then
	 * at least we know all the fields are initialized.
	 */	 
	info->trainId = idNumber++;
	info->arrival = 0;
	info->direction = (random() % 2 + 1);
	info->length = (random() % MAX_LENGTH) + MIN_LENGTH;

	if (!doRandom)
	{
		//inputFile = fopen("folder/file.txt","r");

		char theBuffer[10];
		fgets( theBuffer, 10, inputFile);
		if(theBuffer[0] == 'w' || theBuffer[0] == 'W')
		{
			info->direction = DIRECTION_WEST;
		}
		else if(theBuffer[0] == 'e' || theBuffer[0] == 'E')
		{
			info->direction = DIRECTION_EAST;
		}
		char temp = theBuffer[0];
		theBuffer[0] = 0;
		char str[9];
		for(int num = 0; num < 9; num++)
		{
			str[num] = theBuffer[num+1];
		}
		int length = atoi (str);
		info->length = length;
		//printf("Length is: %d, Direction is: %c\n", length, temp);
		/* Your code here to read a line of input
		 * from the input file 
		 */
	}
	return info;
}


