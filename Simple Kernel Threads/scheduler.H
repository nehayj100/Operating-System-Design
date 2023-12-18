/* 
    Author: R. Bettati, Joshua Capehart
            Department of Computer Science
            Texas A&M University
			
	    A thread scheduler.

*/
#ifndef SCHEDULER_H
#define SCHEDULER_H

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "thread.H"
#include "interrupts.H"
/*--------------------------------------------------------------------------*/
/* !!! IMPLEMENTATION HINT !!! */
/*--------------------------------------------------------------------------*/
/*
    One way to proceed is to implement the FIFO scheduling policy inside
    class 'Scheduler'. 

    If you plan to implement a Round-Robin Scheduler, derive it from class
    'Scheduler', say as class 'RRScheduler'. The class 'RRScheduler' is 
    really just a FIFO scheduler with THREE MODIFICATIONS:
    1. It manages a timer, which fires at the end-of-quantum (EOQ). 
    (For details on how to set up a timer and how to handle timer interrupts 
    see the 1-second timer in 'kernel.C'.)  The timer is set up in the
    constructor.
    2. It uses an additional function, the EOQ handler. This function gets
    called whenever an EOQ timer event fires. The EOQ handler forces the 
    current thread to call the scheduler's 'yield' function.
    3. The 'yield' function must be modified to account for unused quantum
    time. If a thread voluntarily yields, the EOQ timer must be reset in order
    to not penalize the next thread.
 
    (Note that this qualifies as programming at about the level of a baboon.
     Much better woudl be to have the abstract class 'Scheduler' implement 
     the basic scheduling MECHANISMS and provide abstract funtions to define
     the queue management POLICIES in derived classes, 
     such as 'FIFOScheduler'.)
    
 */
// defining NULL

#ifndef NULL
#define NULL 0
#endif

// declaring the queue class and its member functions enqueue and dequeue
class Queue
{
   private:
   // Queue will have two members - the current thread pointer and the pointer to the next thread in teh queue
   Thread *currThread;
   Queue *next;
   // we now declare the queue methods- constructors- in order to initialize and then add the nodes to queue 
   public:
   Queue()
   {
      currThread = NULL;
      next = NULL;
   }
   Queue(Thread * th)
   {
      currThread = th;
      next = NULL;
   }
   // enqueue defined to add a thread to the ready queue
   void enqueue(Thread * add_th)
   {
      if(currThread == NULL)
         currThread = add_th; // if no node in queue then add this as the first node of the queue
      else{
         if(next == NULL) // if we reached the end thenn add this node/thread
            next = new Queue(add_th);
         else
            next->enqueue(add_th); // call enqueue till you get the end of the queue
      }
   }
   // dequeue defined to remove threads from the queue to yeild or to terminate
   Thread* dequeue()
   {
      if(currThread == NULL) // if the queue is empty
         return NULL;

      if (next)
         {
            Thread* th_to_del = currThread; // getting the current thread 
            currThread = next->currThread; // moving the pounter to next thread as the current thread is to be dequeued
            Queue* used = next; 
            next = next->next; // traverse
            delete used; // delete the dequeued thread
            return th_to_del;  // return the dequeued thread - now this is deleted from ready queue but itn will be passed to yield and it gets CPU next in that case
         }
            
         Thread* t = currThread;
         currThread = NULL;
         return t;
   }

   };


/*--------------------------------------------------------------------------*/
/* SCHEDULER */
/*--------------------------------------------------------------------------*/

class Scheduler {
   Queue readyQ; // define the ready queue
   int size_q; // define siz eof the queue so that you can know the number of thready ready in the queue and also comeshandy while traversal 
  /* The scheduler may need private members... */
  // Linked list to maintain the threads in ready queue
  private:
   
public:

   Scheduler();
   /* Setup the scheduler. This sets up the ready queue, for example.
      If the scheduler implements some sort of round-robin scheme, then the 
      end_of_quantum handler is installed in the constructor as well. */

   /* NOTE: We are making all functions virtual. This may come in handy when
            you want to derive RRScheduler from this class. */
  
   virtual void yield();
   /* Called by the currently running thread in order to give up the CPU. 
      The scheduler selects the next thread from the ready queue to load onto 
      the CPU, and calls the dispatcher function defined in 'Thread.H' to
      do the context switch. */

   virtual void resume(Thread * _thread);
   /* Add the given thread to the ready queue of the scheduler. This is called
      for threads that were waiting for an event to happen, or that have 
      to give up the CPU in response to a preemption. */

   virtual void add(Thread * _thread);
   /* Make the given thread runnable by the scheduler. This function is called
      after thread creation. Depending on implementation, this function may 
      just add the thread to the ready queue, using 'resume'. */

   virtual void terminate(Thread * _thread);
   /* Remove the given thread from the scheduler in preparation for destruction
      of the thread. 
      Graciously handle the case where the thread wants to terminate itself.*/
  
};
	
// creating RR scheduler which inherits from the scheduler and interruptn handler
class RoundRobinScheduler : public Scheduler, public InterruptHandler
{
   // new queue for RR
   Queue RoundRobinQ;
   int RRQ_size; // size 
   int sec; // ticks as done in system timer
   int hz; // frequency
   void init_freq(int _hz); // function that sets frequency as per the required quantum 

   public:
   RoundRobinScheduler(); // constuctor
   virtual void yield(); // yileding the next thread
   virtual void resume(Thread * _thread); // resume the blocked thread
   virtual void add(Thread * _thread); // adding ot the ready queue
   virtual void terminate(Thread * _thread); // terminate the thread in RR
   virtual void handle_interrupt(REGS *_r); // handle the timer or end of quantum interrupt!
};



#endif
