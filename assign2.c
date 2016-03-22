/*
 * assign2.c
 *
 * Name:
 * Student Number:
 */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h> 
#include <pthread.h>
#include <semaphore.h>
#include "train.h"

/*
 * If you uncomment the following line, some debugging
 * output will be produced.
 *
 * Be sure to comment this line out again before you submit 
 */

/* #define DEBUG	1 */

void ArriveBridge (TrainInfo *train);
void CrossBridge (TrainInfo *train);
void LeaveBridge (TrainInfo *train);

int *eastTrainArray;
int *westTrainArray;
int eastInsertTracker = 0;
int eastRemoveTracker = 0;
int westInsertTracker = 0;
int westRemoveTracker = 0;
int incW = 0;
int incE = 0;
int firstTrain = 1;
int trainCount = 0;
int mostRecentArrival = 0;
int tmpBool = 0;

int lastTrainsDir[2] = {-1, -1}; 

pthread_mutex_t	mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t	noTrainsMut = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t	doSomething = PTHREAD_COND_INITIALIZER;
pthread_cond_t	noTrainsCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t queueLock;

sem_t *done;


int turn = -1;
int afterTurn = -1;


void lock(pthread_mutex_t item)
{
	int returnVal = pthread_mutex_lock(&item);
	if(returnVal != 0)
	{
		printf("There was an error locking a mutex: %d.", returnVal);
		fflush(stdout);
	}
	fflush(stdout);
}

/*
* A function to make it easier to unlock the bridge
*/
void unlock(pthread_mutex_t item)
{
	if(pthread_mutex_unlock(&item) != 0)
	{
		printf("There was an error unlocking a mutex..");
		fflush(stdout);
	}
}


/*
 * This function is started for each thread created by the
 * main thread.  Each thread is given a TrainInfo structure
 * that specifies information about the train the individual 
 * thread is supposed to simulate.
 */
void * Train ( void *arguments )
{
	TrainInfo	*train = (TrainInfo *)arguments;

	/* Sleep to simulate different arrival times */
	usleep (train->length*SLEEP_MULTIPLE);

	ArriveBridge (train);
	CrossBridge  (train);
	LeaveBridge  (train); 

	/* I decided that the paramter structure would be malloc'd 
	 * in the main thread, but the individual threads are responsible
	 * for freeing the memory.
	 *
	 * This way I didn't have to keep an array of parameter pointers
	 * in the main thread.
	 */
	free (train);
	return NULL;
}

/*
 * You will need to add code to this function to ensure that
 * the trains cross the bridge in the correct order.
 */
void ArriveBridge ( TrainInfo *train )
{
	//printf ("Train %2d arrives going %s\n", (train->trainId -1), 
	//		(train->direction == DIRECTION_WEST ? "West" : "East"));
	pthread_mutex_lock(&noTrainsMut);
	lock(queueLock);

	
	if(train->direction == DIRECTION_WEST){
		westTrainArray[westInsertTracker++] = train->trainId;

		if(firstTrain == 1)
		{
			firstTrain--;
			turn = train->trainId;
			westRemoveTracker++;
			//lastTrainsDir[0] = train->direction;
		}
	}
	else if(train->direction == DIRECTION_EAST){
		eastTrainArray[eastInsertTracker++] = train->trainId;

		if(firstTrain == 1)
		{
			firstTrain--;
			turn = train->trainId;
			eastRemoveTracker++;
			//lastTrainsDir[0] = train->direction;
		}
	}
	else
	{
		printf("HUGE ERROR\n");
	}
	mostRecentArrival = train->trainId;
	pthread_mutex_unlock(&noTrainsMut);
	pthread_cond_broadcast (&noTrainsCond);

	//printf("Inside Lock, added: %d\n", train->trainId);
	fflush(stdout);
	unlock(queueLock);
	//printf("Outside lock\n");

	int myid = train->trainId;

	pthread_mutex_lock(&mutex);  
	while (turn != myid ) {        
		//printf("Not my turn: myid = %d turn = %d\n", myid, turn);
		pthread_cond_wait(&doSomething, &mutex); 
	}

	//printf ("My turn: %d %d\n", turn, myid);
	pthread_mutex_unlock(&mutex);
	sem_post (done);



	/* Your code here... */
}

/*
 * Simulate crossing the bridge.  You shouldn't have to change this
 * function.
 */
void CrossBridge ( TrainInfo *train )
{
	//printf ("Train %2d is ON the bridge (%s)\n", (train->trainId-1),
	//		(train->direction == DIRECTION_WEST ? "West" : "East"));
	fflush(stdout);
	
	/* 
	 * This sleep statement simulates the time it takes to 
	 * cross the bridge.  Longer trains take more time.
	 */
	usleep (train->length*SLEEP_MULTIPLE);

	printf ("Train %2d is OFF the bridge(%s)\n", (train->trainId - 1), 
			(train->direction == DIRECTION_WEST ? "West" : "East"));
	fflush(stdout);
}

/*
 * Add code here to make the bridge available to waiting
 * trains...
 */
void LeaveBridge ( TrainInfo *train )
{

	lock(queueLock);
	pthread_mutex_lock(&mutex);

	pthread_mutex_lock(&noTrainsMut);

	lastTrainsDir[1] = lastTrainsDir[0];
	lastTrainsDir[0] = train->direction;
/*
	printf("PRE Turn: %d, AfterTurn: %d\n", turn-1, afterTurn-1);
	incE = 0;
	incW = 0;

	if(eastRemoveTracker+westRemoveTracker != trainCount)
	{
		while (eastTrainArray[eastRemoveTracker] == 0 && westTrainArray[westRemoveTracker] == 0) 
		{        
			pthread_cond_wait(&noTrainsCond, &noTrainsMut);
		}
	}
	pthread_mutex_unlock(&noTrainsMut);
	if(afterTurn != -1)
	{
		turn = afterTurn;
		afterTurn = -1;
	}
	else if(eastTrainArray[eastRemoveTracker] != 0 && westTrainArray[westRemoveTracker] != 0)
	{	//Both sides waiting
		printf("YES\n");
		if(turn == eastTrainArray[eastRemoveTracker-1])
		{ // Current train is an east train
			turn = eastTrainArray[eastRemoveTracker];
			afterTurn = westTrainArray[westRemoveTracker];
			incE = 1;
			incW = 1;
		}
		else if(turn == westTrainArray[westRemoveTracker-1])
		{ // Current train is a west train
			turn = eastTrainArray[eastRemoveTracker];
			incE = 1;
			if(eastTrainArray[eastRemoveTracker] != 0)
			{
				afterTurn = eastTrainArray[eastRemoveTracker + 1];
				incE = 2;
				//gotta add a ++
			}
			else
			{
				afterTurn = -1;
			}
		}
		else
		{
			printf("Shits fucked yo\n");
		}
	}
	else if(eastTrainArray[eastRemoveTracker] != 0)
	{
		printf("YES2\n");
		turn = eastTrainArray[eastRemoveTracker];
		incE = 1;
		afterTurn = -1;
	}
	else if(westTrainArray[westRemoveTracker] != 0)
	{
		printf("YES3\n");
		turn = westTrainArray[westRemoveTracker];
		incW = 1;
		afterTurn = -1;
	}
	else
	{
		printf("Shouldnt ever hit this\n");
	}

	if(incE == 1)
	{
		eastRemoveTracker++;
	}
	if(incE == 2)
	{
		eastRemoveTracker++;
		eastRemoveTracker++;
	}
	if(incW)
	{
		westRemoveTracker++;
	}
	
	
	printf("POST Turn: %d, AfterTurn: %d\n", turn-1, afterTurn-1);
*/
/*

ALTERNATE SOLUTION IF I MISUNDERSTOOD THE PROBLEM DEFINITION

	printf("PRE Turn: %d, AfterTurn: %d\n", turn-1, afterTurn-1);

	//good one?
	if(eastRemoveTracker+westRemoveTracker != trainCount)
	{
		while (eastTrainArray[eastRemoveTracker] == 0 && westTrainArray[westRemoveTracker] == 0) 
		{        
			pthread_cond_wait(&noTrainsCond, &noTrainsMut);
		}
	}
	if(afterTurn == -1)
	{
		if(lastTrainsDir[0] == DIRECTION_EAST && lastTrainsDir[1] == DIRECTION_EAST)
		{
			if(westTrainArray[westRemoveTracker] != 0)
			{
				turn = westTrainArray[westRemoveTracker++];
				afterTurn = -1;
			}
			else
			{
				turn = eastTrainArray[eastRemoveTracker++];
				if(eastTrainArray[eastRemoveTracker] != 0)
				{
					afterTurn = eastTrainArray[eastRemoveTracker++];
				}
				else
				{
					afterTurn = -1;
				}
			}
		}
		else
		{
			if(eastTrainArray[eastRemoveTracker] != 0)
			{
				turn = eastTrainArray[eastRemoveTracker++];
				if(eastTrainArray[eastRemoveTracker] != 0)
				{
					printf("Hit this\n");
					afterTurn = eastTrainArray[eastRemoveTracker++];
				}
				else
				{
					printf("Hit this - 2\n");
					if(westTrainArray[westRemoveTracker] != 0)
					{
						afterTurn = westTrainArray[westRemoveTracker++];
					}
					else
					{
						afterTurn = -1;
					}
				}
			}
			//definitely no easts in queue
			else
			{
				if(westTrainArray[westRemoveTracker] != 0)
				{
					turn = westTrainArray[westRemoveTracker++];
					afterTurn = -1;
				}
			}
		}
	}
	else
	{
		turn = afterTurn;
		afterTurn = -1;
	}
	printf("POST Turn: %d, AfterTurn: %d\n", (turn-1), (afterTurn-1));
	*/

	
	//Kinda old
	if(eastRemoveTracker+westRemoveTracker != trainCount)
	{
		while (eastTrainArray[eastRemoveTracker] == 0 && westTrainArray[westRemoveTracker] == 0) 
		{        
			//printf("Bridge is free, no trains waiting\n");
			tmpBool = 1;
			pthread_cond_wait(&noTrainsCond, &noTrainsMut);
			//turn = mostRecentArrival;
		}
	}

	pthread_mutex_unlock(&noTrainsMut);
	if(tmpBool)
	{
		turn = mostRecentArrival;
		tmpBool = 0;
		//printf("Most recent\n");
	}
	else if(lastTrainsDir[0] != DIRECTION_EAST || lastTrainsDir[1] != DIRECTION_EAST)
	{
		//if one of the previous two directions was west
		//printf("SHOULD HIT\n");
		//printf("Last two directions: %d, %d\n", lastTrainsDir[0], lastTrainsDir[1]);

		if(eastTrainArray[eastRemoveTracker] != 0)
		{
			turn = eastTrainArray[eastRemoveTracker++];
		}
		else
		{
			turn = westTrainArray[westRemoveTracker++];
		}
		
		//let east go
	}
	else
	{
		if(westTrainArray[westRemoveTracker] != 0)
		{
			//printf("Last two directions: %d, %d\n", lastTrainsDir[0], lastTrainsDir[1]);
			turn = westTrainArray[westRemoveTracker++];
		}
		else
		{
			turn = eastTrainArray[eastRemoveTracker++];
		}
		
		//let west go
	}
	/*
	if(turn == 4)
	{
		printf("Direction: %d\n",train->direction);
		printf("Last two directions WEIRD: %d, %d\n", lastTrainsDir[0], lastTrainsDir[1]);
		//print("Direction: %d\n",train->direction);
	}
*/
	//printf("Turn is: %d\n", turn);

	//printf("Should be this persons turn: %d\n", turn);
	//turn = eastTrainArray[eastRemoveTracker];
	//eastRemoveTracker += 1;
	pthread_mutex_unlock(&mutex);
	unlock(queueLock);
	pthread_cond_broadcast (&doSomething);
	sem_wait(done);
	//printf("Train left the bridge.\n");
}

int main ( int argc, char *argv[] )
{
	eastTrainArray = calloc(100, sizeof(int));
	westTrainArray = calloc(100, sizeof(int));

	char 		*filename = NULL;
	pthread_t	*tids;
	int		i;

		
	//create a mutex
	if (pthread_mutex_init(&queueLock, NULL) != 0)
    {
        printf("\n mutex init failed\n");
        return 1;
    }

	/* Parse the arguments */
	if ( argc < 2 )
	{
		printf ("Usage: part1 n {filename}\n\t\tn is number of trains\n");
		printf ("\t\tfilename is input file to use (optional)\n");
		exit(0);
	}
	
	if ( argc >= 2 )
	{
		trainCount = atoi(argv[1]);
	}
	if ( argc == 3 )
	{
		filename = argv[2];
	}	
	
	initTrain(filename);
	
	/*
	 * Since the number of trains to simulate is specified on the command
	 * line, we need to malloc space to store the thread ids of each train
	 * thread.
	 */
	tids = (pthread_t *) malloc(sizeof(pthread_t)*trainCount);
	
	/*
	 * Create all the train threads pass them the information about
	 * length and direction as a TrainInfo structure
	 */
	for (i=0;i<trainCount;i++)
	{
		TrainInfo *info = createTrain();
		
		/*printf ("Train %2d headed %s length is %d\n", info->trainId,
			(info->direction == DIRECTION_WEST ? "West" : "East"),
			info->length );*/

		if ( pthread_create (&tids[i],0, Train, (void *)info) != 0 )
		{
			printf ("Failed creation of Train.\n");
			exit(0);
		}
	}

	/*
	 * This code waits for all train threads to terminate
	 */
	for (i=0;i<trainCount;i++)
	{
		pthread_join (tids[i], NULL);
	}
	for(int x = 0; x < 4; x++)
	{
		lock(queueLock);
		//printf("Train IDs: %d \n", eastTrainArray[x]);
		unlock(queueLock);
	}
	
	free(tids);
	return 0;
}

