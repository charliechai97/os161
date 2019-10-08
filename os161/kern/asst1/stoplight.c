/* 
 * stoplight.c
 *
 * 31-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: You can use any synchronization primitives available to solve
 * the stoplight problem in this file.
 */


/*
 * 
 * Includes
 *
 */

#include <types.h>
#include <lib.h>
#include <test.h>
#include <thread.h>


/*
 *
 * Constants
 *
 */

/*
 * Number of cars created.
 */

#define NCARS 20

//locks for lanes
struct lock *north, *east, *south, *west;
//locks for intersections
struct lock *nw, *ne, *se, *sw;
//semaphore to finish
struct semaphore *finish;

/*
 *
 * Function Definitions
 *
 */

static const char *directions[] = { "N", "E", "S", "W" };

static const char *msgs[] = {
        "approaching:",
        "region1:    ",
        "region2:    ",
        "region3:    ",
        "leaving:    "
};

/* use these constants for the first parameter of message */
enum { APPROACHING, REGION1, REGION2, REGION3, LEAVING };

static void
message(int msg_nr, int carnumber, int cardirection, int destdirection)
{
        kprintf("%s car = %2d, direction = %s, destination = %s\n",
                msgs[msg_nr], carnumber,
                directions[cardirection], directions[destdirection]);
}
 
/*
 * gostraight()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement passing straight through the
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
gostraight(unsigned long cardirection,
           unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */
        
        (void) cardirection;
        (void) carnumber;

	//from north
	if(cardirection == 0){
		lock_acquire(north);
		message(0,carnumber,cardirection,2);
		lock_acquire(nw);
		lock_acquire(sw);
		lock_release(north);
		message(1,carnumber,cardirection,2);
		message(2,carnumber,cardirection,2);
		message(4,carnumber,cardirection,2);
		lock_release(nw);
		lock_release(sw);
	}
	//from east
	if(cardirection == 1){
		lock_acquire(east);
		message(0,carnumber,cardirection,3);
		lock_acquire(nw);
		lock_acquire(ne);
		lock_release(east);
		message(1,carnumber,cardirection,3);
		message(2,carnumber,cardirection,3);
		message(4,carnumber,cardirection,3);
		lock_release(nw);
		lock_release(ne);
	}
	//from south
	if(cardirection == 2){
		lock_acquire(south);
		message(0,carnumber,cardirection,0);
		lock_acquire(ne);
		lock_acquire(se);
		lock_release(south);
		message(1,carnumber,cardirection,0);
		message(2,carnumber,cardirection,0);
		message(4,carnumber,cardirection,0);
		lock_release(ne);
		lock_release(se);
	}
	//from west
	if(cardirection == 3){
		lock_acquire(west);
		message(0,carnumber,cardirection,1);
		lock_acquire(se);
		lock_acquire(sw);
		lock_release(west);
		message(1,carnumber,cardirection,1);
		message(2,carnumber,cardirection,1);
		message(4,carnumber,cardirection,1);
		lock_release(se);
		lock_release(sw);
	}
}


/*
 * turnleft()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a left turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnleft(unsigned long cardirection,
         unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) cardirection;
        (void) carnumber;

	//from north
	if(cardirection == 0){
		lock_acquire(north);
		message(0,carnumber,cardirection,1);
		lock_acquire(nw);
		lock_acquire(se);
		lock_acquire(sw);
		lock_release(north);
		message(1,carnumber,cardirection,1);
		message(2,carnumber,cardirection,1);
		message(3,carnumber,cardirection,1);
		message(4,carnumber,cardirection,1);
		lock_release(nw);
		lock_release(se);
		lock_release(sw);
	}
	//from east
	if(cardirection == 1){
		lock_acquire(east);
		message(0,carnumber,cardirection,2);
		lock_acquire(nw);
		lock_acquire(ne);
		lock_acquire(sw);
		lock_release(east);
		message(1,carnumber,cardirection,2);
		message(2,carnumber,cardirection,2);
		message(3,carnumber,cardirection,2);
		message(4,carnumber,cardirection,2);
		lock_release(nw);
		lock_release(ne);
		lock_release(sw);
	}
	//from south
	if(cardirection == 2){
		lock_acquire(south);
		message(0,carnumber,cardirection,3);
		lock_acquire(nw);
		lock_acquire(ne);
		lock_acquire(se);
		lock_release(south);
		message(1,carnumber,cardirection,3);
		message(2,carnumber,cardirection,3);
		message(3,carnumber,cardirection,3);
		message(4,carnumber,cardirection,3);
		lock_release(nw);
		lock_release(ne);
		lock_release(se);
	}
	//from west
	if(cardirection == 3){
		lock_acquire(west);
		message(0,carnumber,cardirection,0);
		lock_acquire(ne);
		lock_acquire(se);
		lock_acquire(sw);
		lock_release(west);
		message(1,carnumber,cardirection,0);
		message(2,carnumber,cardirection,0);
		message(3,carnumber,cardirection,0);
		message(4,carnumber,cardirection,0);
		lock_release(ne);
		lock_release(se);
		lock_release(sw);
	}
}


/*
 * turnright()
 *
 * Arguments:
 *      unsigned long cardirection: the direction from which the car
 *              approaches the intersection.
 *      unsigned long carnumber: the car id number for printing purposes.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      This function should implement making a right turn through the 
 *      intersection from any direction.
 *      Write and comment this function.
 */

static
void
turnright(unsigned long cardirection,
          unsigned long carnumber)
{
        /*
         * Avoid unused variable warnings.
         */

        (void) cardirection;
        (void) carnumber;

	//from north
	if(cardirection == 0){
		lock_acquire(north);
		message(0,carnumber,cardirection,3);
		lock_acquire(nw);
		lock_release(north);
		message(1,carnumber,cardirection,3);
		message(4,carnumber,cardirection,3);
		lock_release(nw);
	}
	//from east
	if(cardirection == 1){
		lock_acquire(east);
		message(0,carnumber,cardirection,0);
		lock_acquire(ne);
		lock_release(east);
		message(1,carnumber,cardirection,0);
		message(4,carnumber,cardirection,0);
		lock_release(ne);
	}
	//from south
	if(cardirection == 2){
		lock_acquire(south);
		message(0,carnumber,cardirection,1);
		lock_acquire(se);
		lock_release(south);
		message(1,carnumber,cardirection,1);
		message(4,carnumber,cardirection,1);
		lock_release(se);
	}
	//from west
	if(cardirection == 3){
		lock_acquire(west);
		message(0,carnumber,cardirection,2);
		lock_acquire(sw);
		lock_release(west);
		message(1,carnumber,cardirection,2);
		message(4,carnumber,cardirection,2);
		lock_release(sw);
	}
}


/*
 * approachintersection()
 *
 * Arguments: 
 *      void * unusedpointer: currently unused.
 *      unsigned long carnumber: holds car id number.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Change this function as necessary to implement your solution. These
 *      threads are created by createcars().  Each one must choose a direction
 *      randomly, approach the intersection, choose a turn randomly, and then
 *      complete that turn.  The code to choose a direction randomly is
 *      provided, the rest is left to you to implement.  Making a turn
 *      or going straight should be done by calling one of the functions
 *      above.
 */
 
static
void
approachintersection(void * unusedpointer,
                     unsigned long carnumber)
{
        int cardirection;

        /*
         * Avoid unused variable and function warnings.
         */

        (void) unusedpointer;
        (void) carnumber;
		(void) gostraight;
		(void) turnleft;
		(void) turnright;

        /*
         * cardirection is set randomly.
         */

        //0 north, 1 east, 2 south, 3 west
        cardirection = random() % 4;
		//0 right, 1 straight, 2 left
		int turndirection = random() % 3;

		//from north
		if(cardirection == 0){
			if(turndirection == 0){
				turnright(cardirection,carnumber);
			}
			if(turndirection == 1){
				gostraight(cardirection,carnumber);
			}
			if(turndirection == 2){
				turnleft(cardirection,carnumber);
			}
		}
		//from east
		if(cardirection == 1){
			if(turndirection == 0){
				turnright(cardirection,carnumber);
			}
			if(turndirection == 1){
				gostraight(cardirection,carnumber);
			}
			if(turndirection == 2){
				turnleft(cardirection,carnumber);
			}
		}
		//from south
		if(cardirection == 2){
			if(turndirection == 0){
				turnright(cardirection,carnumber);
			}
			if(turndirection == 1){
				gostraight(cardirection,carnumber);
			}
			if(turndirection == 2){
				turnleft(cardirection,carnumber);
			}
		}
		//from west
		if(cardirection == 3){
			if(turndirection == 0){
				turnright(cardirection,carnumber);
			}
			if(turndirection == 1){
				gostraight(cardirection,carnumber);
			}
			if(turndirection == 2){
				turnleft(cardirection,carnumber);
			}
		}
		V(finish);
}


/*
 * createcars()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up the approachintersection() threads.  You are
 *      free to modiy this code as necessary for your solution.
 */

int
createcars(int nargs,
           char ** args)
{
        int index, error;

        /*
         * Avoid unused variable warnings.
         */

        (void) nargs;
        (void) args;

		north = lock_create("north");
		east = lock_create("east");
		south = lock_create("south");
		west = lock_create("west");
		nw = lock_create("nw");
		ne = lock_create("ne");
		se = lock_create("se");
		sw = lock_create("sw");
		
		finish = sem_create("finish",0);

        /*
         * Start NCARS approachintersection() threads.
         */

        for (index = 0; index < NCARS; index++) {

                error = thread_fork("approachintersection thread",
                                    NULL,
                                    index,
                                    approachintersection,
                                    NULL
                                    );

                /*
                 * panic() on error.
                 */

                if (error) {
                        
                        panic("approachintersection: thread_fork failed: %s\n",
                              strerror(error)
                              );
                }
        }

		for(index = 0; index < NCARS; index++) {
			P(finish);
		}

		lock_destroy(north);
		lock_destroy(east);
		lock_destroy(south);
		lock_destroy(west);
		lock_destroy(nw);
		lock_destroy(ne);
		lock_destroy(se);
		lock_destroy(sw);

        return 0;
}
