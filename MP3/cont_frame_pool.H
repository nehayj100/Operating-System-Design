/*
 File: cont_frame_pool.H
 
 Author: R. Bettati
 Department of Computer Science
 Texas A&M University
 Date  : 17/02/04 
 
 Description: Management of the CONTIGUOUS Free-Frame Pool.
 
 As opposed to a non-contiguous free-frame pool, here we can allocate
 a sequence of CONTIGUOUS frames.
 
 */

#ifndef _CONT_FRAME_POOL_H_                   // include file only once
#define _CONT_FRAME_POOL_H_

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "machine.H"
#include "simple_frame_pool.H"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* C o n t F r a m e   P o o l  */
/*--------------------------------------------------------------------------*/

class ContFramePool : public SimpleFramePool {
    
public:

    ContFramePool(unsigned long _base_frame_no,
                  unsigned long _n_frames,
                  unsigned long _info_frame_no);
   
    
    unsigned long get_frames(unsigned int _n_frames);
    
    static void release_frames(unsigned long _first_frame_no); 
   
    static unsigned long needed_info_frames(unsigned long _n_frames) { return 1; }

};
#endif