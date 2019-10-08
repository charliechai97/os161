/*
 * catsem.c
 *
 * 30-1-2003 : GWA : Stub functions created for CS161 Asst1.
 *
 * NB: Please use SEMAPHORES to solve the cat syncronization problem in 
 * this file.
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
 * Number of food bowls.
 */

#define NFOODBOWLS 2

/*
 * Number of cats.
 */

#define NCATS 6

/*
 * Number of mice.
 */

#define NMICE 2

typedef int bool;
#define false 0
#define true 1

//exclusive cat or mouse
struct semaphore *c_or_m;

int volatile catsEating = 0;

int volatile miceEating = 0;

int volatile waitb1;

int volatile waitb2;

bool volatile bowl1Use = false;

bool volatile bowl2Use = false;

//mutual exclusion to cat
struct semaphore *cats;

//mutual exclusion to mice
struct sempahore *mice;

struct semaphore *bowl1;

struct semaphore *bowl2;

struct semaphore *areTheyDone;




/*
 * 
 * Function Definitions
 * 
 */

/* who should be "cat" or "mouse" */
static void
sem_eat(const char *who, int num, int bowl, int iteration) {
    kprintf("%s: %d starts eating: bowl %d, iteration %d\n", who, num,
            bowl, iteration);
    clocksleep(1);
    kprintf("%s: %d ends eating: bowl %d, iteration %d\n", who, num,
            bowl, iteration);
}

/*
 * catsem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long catnumber: holds the cat identifier from 0 to NCATS - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
catsem(void * unusedpointer,
        unsigned long catnumber) {
    /*
     * Avoid unused variable warnings.
     */

    (void) unusedpointer;
    (void) catnumber;

    int i;
    for (i = 0; i < 4; i++) {
        P(cats); //lock catsEating
        catsEating += 1;
        if (catsEating == 1) {
            P(c_or_m); //locks out mice
        }
        V(cats); //unlocks catsEating

    //do some cat stuff
         //choose bowl to eat from
        int bowl;
        if (waitb1 <= waitb2) {
            bowl = 1;
            waitb1 += 1;
        } else {
            bowl = 2;
            waitb2 += 1;
        }
        
        
        //eat
        if (bowl == 1) {
            P(bowl1); //lock bowl 1
            bowl1Use = true;
            sem_eat("cat", catnumber, 1, i);
            bowl1Use = false;
            waitb1 -= 1;
            V(bowl1); //unlock bowl 1		
        } else if (bowl == 2) {
            P(bowl2); //lock bowl 2
            bowl2Use = true;
            sem_eat("cat", catnumber, 2, i);
            bowl2Use = false;
            waitb2 -= 1;
            V(bowl2); //unlock bowl 2		
        }
        //done eating
        
        
        P(cats); //locks catsEating
        catsEating -= 1;
        if (catsEating == 0) {
            V(c_or_m); //up for grabs
        }
        V(cats); //unlock catsEating

    }
    V(areTheyDone);
}

/*
 * mousesem()
 *
 * Arguments:
 *      void * unusedpointer: currently unused.
 *      unsigned long mousenumber: holds the mouse identifier from 0 to 
 *              NMICE - 1.
 *
 * Returns:
 *      nothing.
 *
 * Notes:
 *      Write and comment this function using semaphores.
 *
 */

static
void
mousesem(void * unusedpointer,
        unsigned long mousenumber) {
    /*
     * Avoid unused variable warnings.
     */
    (void) unusedpointer;
    (void) mousenumber;
    int i;
    for (i = 0; i < 4; i++) {
        P(mice); //lock miceEating
        miceEating += 1;
        if (miceEating == 1) {
            P(c_or_m); //locks out cats
        }
        V(mice); //unlocks miceEating

    //do some mice stuff
        
        //choose bowl to eat from
        int bowl;
        if (waitb1 <= waitb2) {
            bowl = 1;
            waitb1 += 1;
        } else {
            bowl = 2;
            waitb2 += 1;
        }
        
        //eat
        if (bowl == 1) {
            P(bowl1); //lock bowl 1
            bowl1Use = true;
            sem_eat("mouse", mousenumber, 1, i);
            bowl1Use = false;
            waitb1 -= 1;
            V(bowl1); //unlock bowl 1		
        } else if (bowl == 2) {
            P(bowl2); //lock bowl 2
            bowl2Use = true;
            sem_eat("mouse", mousenumber, 2, i);
            bowl2Use = false;
            waitb2 -= 1;
            V(bowl2); //unlock bowl 2		
        }
        //done eating
        
        P(mice); //locks miceEating
        miceEating -= 1;
        if (miceEating == 0) {
            V(c_or_m); //up for grabs
        }
        V(mice); //unlock miceEating
    }
    V(areTheyDone);


}

/*
 * catmousesem()
 *
 * Arguments:
 *      int nargs: unused.
 *      char ** args: unused.
 *
 * Returns:
 *      0 on success.
 *
 * Notes:
 *      Driver code to start up catsem() and mousesem() threads.  Change this 
 *      code as necessary for your solution.
 */

int
catmousesem(int nargs,
        char ** args) {
    int index, error;

    /*
     * Avoid unused variable warnings.
     */

    (void) nargs;
    (void) args;

    //initialize sempahores
    cats = sem_create("cats", 1);
    mice = sem_create("mice", 1);
    bowl1 = sem_create("bowl 1", 1);
    bowl2 = sem_create("bowl 2", 1);
    c_or_m = sem_create("cat or mouse", 1);
    areTheyDone = sem_create("areTheyDone", 0);

    /*
     * Start NCATS catsem() threads.
     */

    for (index = 0; index < NCATS; index++) {

        error = thread_fork("catsem Thread",
                NULL,
                index,
                catsem,
                NULL
                );

        /*
         * panic() on error.
         */

        if (error) {

            panic("catsem: thread_fork failed: %s\n",
                    strerror(error)
                    );
        }
    }

    /*
     * Start NMICE mousesem() threads.
     */

    for (index = 0; index < NMICE; index++) {

        error = thread_fork("mousesem Thread",
                NULL,
                index,
                mousesem,
                NULL
                );

        /*
         * panic() on error.
         */

        if (error) {

            panic("mousesem: thread_fork failed: %s\n",
                    strerror(error)
                    );
        }
    }

    //destroy semaphores
    int i;
    for (i = 0; i < 8; i++)
        P(areTheyDone);
    sem_destroy(c_or_m);
    sem_destroy(cats);
    sem_destroy(mice);
    sem_destroy(bowl1);
    sem_destroy(bowl2);
    sem_destroy(areTheyDone);
    return 0;
}


/*
 * End of catsem.c
 */
