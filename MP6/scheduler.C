/*
 File: scheduler.C
 
 Author:
 Date  :
 
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "scheduler.H"
#include "thread.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "simple_timer.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */
/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* METHODS FOR CLASS   S c h e d u l e r  */
/*--------------------------------------------------------------------------*/

Scheduler::Scheduler() {
    size_q = 0; // initializing the ready queue size as 0
    Console::puts("Constructed Scheduler\n");
    this->disk = NULL;
}

void Scheduler::yield() {
  Console::puts("into yield\n");

    if (Machine::interrupts_enabled())
    Machine::disable_interrupts();
    
  if(disk != NULL && disk->is_ready() && disk->size_blk_q > 0)
    {
      Console::puts("came to check disk\n");
      //disk->get_q_element();
      Thread* blk_th = disk->blockedThreadsQ->dequeue();
      disk->size_blk_q--;
      Console::puts("dequeued blocked thread\n");
      readyQ.enqueue(blk_th);
      size_q++; // queue size increases
    }

    Thread * next_thread = readyQ.dequeue();
    size_q--;
    Console::puts("deQed non blocked thread\n");
    Thread::dispatch_to(next_thread);

    //decrementing the size of the queueu as we rermoved one element
    
    Console::puts("ready Q size\n");
    Console::puti(size_q);

    // dequeue is done - now we re-enable the interrrupts
    if (!Machine::interrupts_enabled())
      Machine::enable_interrupts();
    // dispatch the dequeued thread to the CPU 
    Console::puts("\n");

}

void Scheduler::resume(Thread * _thread) {
    // disabling the interrupts before yeilding the next thread to CPU
  if (Machine::interrupts_enabled())
    Machine::disable_interrupts();
    // adding the new thread to ready queue
      Console::puts("came to resume\n");

       Scheduler::add(_thread);
  // resume done- re-enable interrupts
  if (!Machine::interrupts_enabled())
      Machine::enable_interrupts();
}

void Scheduler::add(Thread * _thread) {
      // disabling the interrupts before yeilding the next thread to CPU
  if (Machine::interrupts_enabled())
    Machine::disable_interrupts();
    // enqueue the thread
  Console::puts("came to add\n");
  readyQ.enqueue(_thread);
  size_q++; // queue size increases
  Console::puts("ready queue size::\n");
  Console::puti(size_q);
  Console::puts("\n");

    // add done- re-enable interrupts
  if (!Machine::interrupts_enabled())
   Machine::enable_interrupts();
}

void Scheduler::terminate(Thread * _thread) {
// find the thread in the lsit
// if the thread is present then dequeue it and remove it
// if not found then enqueue it back
// we had to dequeue becasue there is no other way to traverse
 if (Machine::interrupts_enabled())
    Machine::disable_interrupts();
  int found_th = 0;
  for (int i = 0; i < size_q; i++) {
    Thread * th_this = readyQ.dequeue();
    if(th_this->ThreadId() == _thread->ThreadId())
    {
      found_th = 1;
      break;
    }
    else{
      readyQ.enqueue(th_this);
    }
  }
  if(found_th == 1)
  {
    size_q--;
    Console::puts("Thread terminated!\n");
  }  
   if (!Machine::interrupts_enabled())
      Machine::enable_interrupts();
}

void Scheduler::assign_disk(BlockingDisk * diskt)
{
  disk = diskt;
}


