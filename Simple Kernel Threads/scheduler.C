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
    Console::puts("Constructed Scheduler.\n");
}

void Scheduler::yield() {
  //assert(false);
  // get next thread in ready queue
  // disabling the interrupts before yeilding the next thread to CPU
  if (Machine::interrupts_enabled())
    Machine::disable_interrupts();
  // get the thread in the froint of the queue to give it a change to get the CPU as per the expectation of FIFO scheduler
  Thread * next_thread = readyQ.dequeue();
  //decrementing the size of the queueu as we rermoved one element
  size_q--;
  // dequeue is done - now we re-enable the interrrupts
  if (!Machine::interrupts_enabled())
    Machine::enable_interrupts();
  // dispatch the dequeued thread to the CPU 
  Thread::dispatch_to(next_thread);
}

void Scheduler::resume(Thread * _thread) {
    // disabling the interrupts before yeilding the next thread to CPU
  if (Machine::interrupts_enabled())
    Machine::disable_interrupts();
    // adding the new thread to ready queue
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
  readyQ.enqueue(_thread);
  size_q++; // queue size increases
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

/*****************************************************************************************/
/*****************************************************************************************/

// Implementing the Round Robin Scheduler! 

/*****************************************************************************************/
/*****************************************************************************************/

RoundRobinScheduler::RoundRobinScheduler()
{
  // we define  a new Q for RR
  RRQ_size = 0;
  sec = 0; //ticks
  hz = 5; // frequencyt to be set
//  SimpleTimer * RRQtimer = new SimpleTimer(1000/EOQ);  // here i need 20Hz frequency for 
  InterruptHandler::register_handler(0, (InterruptHandler *)this); // registering the interrupt tyo the handler
  init_freq(hz); // initiate the timer with quantum selected
  Console::puts("RoundRobin Scehduler is created\n");
}

void RoundRobinScheduler::init_freq(int _hz) {
    // here we do dome low level stuf to initiate the frequency of the timer
    hz = _hz;                            
    int factor = 1193180 / _hz;         
    Machine::outportb(0x43, 0x34);                
    Machine::outportb(0x40, factor & 0xFF);      
    Machine::outportb(0x40, factor >> 8);        
}

void RoundRobinScheduler::handle_interrupt(REGS *_r) {
    sec++; // we increase the timer count
    if (sec >= hz )
    {
      sec = 0;
      Console::puts("Quantum passed- pre-empting required\n"); // quantum passing interrupt
      resume(Thread::CurrentThread());               
		  yield(); // get new thread and yioeld
    }
}

void RoundRobinScheduler::yield() 
{
  Machine::outportb(0x20, 0x20); // informing interrupt handler that its being handled
  if (Machine::interrupts_enabled())
  Machine::disable_interrupts();
  Thread * next_thread = RoundRobinQ.dequeue();
  RRQ_size--;
  if (!Machine::interrupts_enabled())
    Machine::enable_interrupts();
  Thread::dispatch_to(next_thread);
}

//other functions remain the same
void RoundRobinScheduler::resume(Thread * _thread) {
  if (Machine::interrupts_enabled())
    Machine::disable_interrupts();
  RoundRobinScheduler::add(_thread);
  if (!Machine::interrupts_enabled())
      Machine::enable_interrupts();
}

void RoundRobinScheduler::add(Thread * _thread) {
  if (Machine::interrupts_enabled())
    Machine::disable_interrupts();
  RoundRobinQ.enqueue(_thread);
  RRQ_size++;
  if (!Machine::interrupts_enabled())
      Machine::enable_interrupts();
}

void RoundRobinScheduler::terminate(Thread * _thread) {
// find the thread in the lsit
// if the thread is present then dequeue it and remove it
// if not found then enqueue it back
// we had to dequeue becasue there is no other way to traverse
 if (Machine::interrupts_enabled())
    Machine::disable_interrupts();
  int found_th = 0;
  for (int i = 0; i < RRQ_size; i++) {
    Thread * th_this = RoundRobinQ.dequeue();
    if(th_this->ThreadId() == _thread->ThreadId())
    {
      found_th = 1;
      break;
    }
    else{
      RoundRobinQ.enqueue(th_this);
    }
  }
  if(found_th == 1)
  {
    RRQ_size--;
    Console::puts("Thread terminated!\n");
  }  
   if (!Machine::interrupts_enabled())
      Machine::enable_interrupts();
}