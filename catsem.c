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
#include <synch.h>


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

typedef enum { false, true } bool;

volatile int cats_wait_count = 0;
volatile int mice_wait_count = 0;
volatile bool all_dishes_available = true;
volatile bool another_cat_eat = false;
volatile bool first_cat_eat = false;
volatile bool another_mouse_eat = false;
volatile bool first_mouse_eat = false;

struct semaphore* done;
struct semaphore* mutex;
struct semaphore* dish_mutex;

struct semaphore* cats_queue;
volatile bool no_cat_eat = true;

struct semaphore* mice_queue;
volatile bool no_mouse_eat = true;

int mydish = 0;
bool dish1_busy = false;
bool dish2_busy = false;

/*
 * 
 * Function Definitions
 * 
 */

static
void
catsem(void * unusedpointer, 
       unsigned long catnumber)
{
        (void) unusedpointer;
	(void) catnumber;
	P(mutex);
	if (all_dishes_available) {
		all_dishes_available = false;
		V(cats_queue);
	}
	cats_wait_count++;
	V(mutex);
	P(cats_queue);
	if (no_cat_eat == true) {
		no_cat_eat = false;
		first_cat_eat = true;
	}
	else {
		first_cat_eat = false;
	}

	if (first_cat_eat == true) {
		P(mutex);
		if (cats_wait_count > 1) {
			another_cat_eat = true;
			V(cats_queue);
		}
		V(mutex);
	}
	kprintf("Cat in the kitchen.\n");

	P(dish_mutex);
	if (dish1_busy == false) {
		dish1_busy = true;
		mydish = 1;
	}
	else {
		assert(dish2_busy == false);
		dish2_busy = true;
		mydish = 2;
	}
	V(dish_mutex);
	kprintf("Cat eating.\n");
	clocksleep(1);
	kprintf("Finished Eating.\n");

	P(dish_mutex);
	if (mydish == 1) {
		dish1_busy = false;
	}
	else {
		dish2_busy = false;
	}
	V(dish_mutex);

	P(mutex);
	cats_wait_count--;
	V(mutex);
	
	if (first_cat_eat == true) {
		if (another_cat_eat == true) {
			P(done);
		}
		kprintf("First cat is leaving.\n");
		no_cat_eat = true;
		
		P(mutex);
		if (mice_wait_count > 0) {
			V(mice_queue);
		}
		else if (cats_wait_count > 0) {
			V(cats_queue);
		}
		else {
			all_dishes_available = true;
		}
	}
	else {
		kprintf("Non-first cat is leaving.\n");
		V(done);
	}
	
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
         unsigned long mousenumber)
{
        (void) unusedpointer;
	(void) mousenumber;
	P(mutex);
	if (all_dishes_available) {
		all_dishes_available = false;
		V(mice_queue);
	}
	mice_wait_count++;
	V(mutex);
	P(mice_queue);
	if (no_mouse_eat == true) {
		no_mouse_eat = false;
		first_mouse_eat = true;
	}
	else {
		first_mouse_eat = false;
	}
	
	if (first_mouse_eat == true) {
		P(mutex);
		if (mice_wait_count > 1) {
			another_cat_eat = true;
			V(mice_queue);
		}
		V(mutex);
	}

	kprintf("Mouse in the kitchen.\n");
	
	P(dish_mutex);
	if (dish1_busy == false) {
		dish1_busy = true;
		mydish = 1;
	}
	else {
		assert(dish2_busy == false);
		dish2_busy = true;
		mydish = 2;
	}
	V(dish_mutex);
	kprintf("Mouse eating.\n");
	clocksleep(1);
	kprintf("Finished eating.\n");

	P(mutex);
	mice_wait_count--;
	V(mutex);

	if (first_mouse_eat == true) {
		if (another_mouse_eat == true) {
			P(done);
		}
		kprintf("First mouse is leaving.\n");
		no_mouse_eat = true;
		
		P(mutex);
		if (cats_wait_count > 0) {
			V(cats_queue);
		}
		else if (mice_wait_count > 0) {
			V(mice_queue);
		}
		else {
			all_dishes_available = true;
		}
		V(mutex);
	}
	else {
		kprintf("Non-first mouse is leaving.\n");
		V(done);
	}
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
            char ** args)
{
	int index, error;
   
        /*
         * Avoid unused variable warnings.
         */
        (void) nargs;
        (void) args;

	if (cats_queue == NULL) {
		cats_queue = sem_create("cats_queue", 0);
		if (cats_queue == NULL) {
			panic("cats_queue create failed.\n");
		}
	}
	if (mice_queue == NULL) {
		mice_queue = sem_create("mice_queue", 0);
		if (mice_queue == NULL) {
			panic("mice_queue create failed.\n");
		}
	}
	if (done == NULL) {
		done = sem_create("done", 0);
		if (done == NULL) {
			panic("done create failed.\n");
		}
	}
	if (mutex == NULL) {
		mutex = sem_create("mutex", 2);
		if (mutex == NULL) {
			panic("mutex create failed.\n");
		}
	}
	if (dish_mutex == NULL) {
		dish_mutex = sem_create("dish_mutex", 2);
		if (dish_mutex == NULL) {
			panic("dish_mutex create failed.\n");
		}
	}
   
        /*
         * Start NCATS catsem() threads.
         *
         */
	index = 0;
        while (index < NCATS) {
           
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
		index++;
        }
        
        /*
         * Start NMICE mousesem() threads.
         */
	index = 0;
        while (index < NMICE) {
   
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
		index++;
        }
	
	sem_destroy(cats_queue);
	sem_destroy(mice_queue);
	sem_destroy(mutex);
	sem_destroy(dish_mutex);
	sem_destroy(done);

	

        return 0;
}


/*
 * End of catsem.c
 */
