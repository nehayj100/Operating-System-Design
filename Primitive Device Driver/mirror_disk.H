/*
     File        : blocking_disk.H

     Author      : 

     Date        : 
     Description : 

*/

#ifndef _MIRROR_DISK_H_
#define _MIRROR_DISK_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/
#include "simple_disk.H"
# include "thread.H"
#include "scheduler.H"
#include "interrupts.H"
#include "blocking_disk.H"
/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* B l o c k i n g D i s k  */
/*--------------------------------------------------------------------------*/
// class Scheduler;
// class Queue;

class MirrorDisk : public BlockingDisk {
private:
    BlockingDisk * MASTER_DISK;
    BlockingDisk * SLAVE_DISK;

public:
//    Queue *blockedThreadsMirrorQ;
//    int size_blk_q_m;
//    //void get_q_element();
//    bool is_ready();
//    //int locked=0;
   MirrorDisk(DISK_ID _disk_id, unsigned int _size); 
   /* Creates a BlockingDisk device with the given size connected to the 
      MASTER or SLAVE slot of the primary ATA controller.
      NOTE: We are passing the _size argument out of laziness. 
      In a real system, we would infer this information from the 
      disk controller. */
   /* DISK OPERATIONS */



   virtual void read(unsigned long _block_no, unsigned char * _buf);
   /* Reads 512 Bytes from the given block of the disk and copies them 
      to the given buffer. No error check! */

   virtual void write(unsigned long _block_no, unsigned char * _buf);
   /* Writes 512 Bytes from the buffer to the given block on the disk. */

   //void handle_interrupt(REGS *_r);
};

#endif

