/*
     File        : blocking_disk.c

     Author      : 
     Modified    : 

     Description : 

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include"blocking_disk.H"
#include "assert.H"
#include "utils.H"
#include "console.H"
extern Scheduler* SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */ 
/*--------------------------------------------------------------------------*/

BlockingDisk::BlockingDisk(DISK_ID _disk_id, unsigned int _size) 
  : SimpleDisk(_disk_id, _size) {
    this->blockedThreadsQ = new Queue();
    size_blk_q = 0;
    lock = 0;
}

/*--------------------------------------------------------------------------*/
/* SIMPLE_DISK FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool BlockingDisk::is_ready()
{
  return SimpleDisk::is_ready();
}

void BlockingDisk::read(unsigned long _block_no, unsigned char * _buf) {
  // -- REPLACE THIS!!!
  // SimpleDisk::read(_block_no, _buf);

  //wait_until_ready(); // we need to fix this
  // we need to leave the cpu here and do polling such that we periodically check if it is ready 

  if(lock == 0)
    {
      lock = 1;
     SimpleDisk::issue_operation(DISK_OPERATION::READ, _block_no);
  // so we add the blocked thread to a queue that maintains all the blocked threads
  // further we yiled the CPU
  if(!is_ready())
  {
    Console::puts("ready checking starts\n");
    Thread * curr_th = Thread::CurrentThread();

    blockedThreadsQ->enqueue(curr_th); // enqueue the thread?

    Console::puts("enqueue done\n");
    size_blk_q++;
    Console::puti(size_blk_q);
    Console::puts("\n");
    Console::puts("yielded the cpu instead of busy waiting!\n");
    SYSTEM_SCHEDULER->yield();
  }
  // now we have to - at some point dequeue the thread and check if its ready and if its ready we start implementing it
  // once ready get the data
  /* read data from port */
      int i;
       unsigned short tmpw;
      for (i = 0; i < 256; i++) {
        tmpw = Machine::inportw(0x1F0);
        _buf[i*2]   = (unsigned char)tmpw;
        _buf[i*2+1] = (unsigned char)(tmpw >> 8);

      }
      Console::puts("read!! enjoy!\n");
      lock = 0;
     // Console::puti(i);
  }
    else
{

  Console::puts("BUSY DISKKK\n");
    
}
}

void BlockingDisk::write(unsigned long _block_no, unsigned char * _buf) {

if(lock == 0)
  {
    lock = 1;
  SimpleDisk::issue_operation(DISK_OPERATION::WRITE, _block_no);
  if(!is_ready())
  {
    Thread * curr_th = Thread::CurrentThread();
    blockedThreadsQ->enqueue(curr_th); // enqueue the thread?
    size_blk_q++;
    SYSTEM_SCHEDULER->yield();
  }

    int i; 
    unsigned short tmpw;
    for (i = 0; i < 256; i++) 
    {
      tmpw = _buf[2*i] | (_buf[2*i+1] << 8);
      Machine::outportw(0x1F0, tmpw);
      //Console::puti(i);
    }
    Console::puts("written!! enjoy!\n");
    lock = 0;
  }
   
  else
{  
   Console::puts("DISK IS BUSY\n");
  
}
}

void BlockingDisk::handle_interrupt(REGS *_r)
{
  if(this->size_blk_q >0)
    {
      Thread * thisThr = this->blockedThreadsQ->dequeue();
      size_blk_q--;
      SYSTEM_SCHEDULER->resume(thisThr);
    }
}
