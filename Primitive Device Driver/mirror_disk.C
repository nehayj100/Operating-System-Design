

#include "mirror_disk.H"
#include"blocking_disk.H"
#include "assert.H"
#include "utils.H"
#include "console.H"
extern Scheduler* SYSTEM_SCHEDULER;

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */ 
/*--------------------------------------------------------------------------*/

MirrorDisk::MirrorDisk(DISK_ID _disk_id, unsigned int _size) 
  : BlockingDisk(_disk_id, _size) {
MASTER_DISK = new BlockingDisk(DISK_ID::MASTER, _size);
SLAVE_DISK = new BlockingDisk(DISK_ID::DEPENDENT, _size);
}


void MirrorDisk::read(unsigned long _block_no, unsigned char * _buf)
{
  Console::puts("I am in read of mirror!!!\n");
  while(MASTER_DISK->is_ready() || SLAVE_DISK->is_ready())
  {
    Console::puts("IN READ WHILEEEEEEE!!!\n");
    if(MASTER_DISK->is_ready() && MASTER_DISK->lock == 0)
    {
        Console::puts("master got ready\n");
        MASTER_DISK->read(_block_no,_buf);
    }
    else if(SLAVE_DISK->is_ready() && SLAVE_DISK->lock == 0)
    {
      Console::puts("slave got ready\n");
      SLAVE_DISK->read(_block_no, _buf);
    }
    else{
      Console::puts("neither master nor slave is ready so yileded !!\n");
      Thread * thisThr = Thread::CurrentThread();
      MASTER_DISK->blockedThreadsQ->enqueue(thisThr); // enqueue the thread?
      SLAVE_DISK->blockedThreadsQ->enqueue(thisThr);
      MASTER_DISK->size_blk_q++;
      SLAVE_DISK->size_blk_q++;
      SYSTEM_SCHEDULER->yield();
    }
  }
  
}


void MirrorDisk::write(unsigned long _block_no, unsigned char * _buf) 
{
   MASTER_DISK->write (_block_no, _buf);
   SLAVE_DISK->write (_block_no, _buf);
}
