/*
 File: ContFramePool.C
 
 Author: Neha Joshi
 Date  : 17/09/2023
 
 */

/*--------------------------------------------------------------------------*/
/* 
 POSSIBLE IMPLEMENTATION
 -----------------------

 The class SimpleFramePool in file "simple_frame_pool.H/C" describes an
 incomplete vanilla implementation of a frame pool that allocates 
 *single* frames at a time. Because it does allocate one frame at a time, 
 it does not guarantee that a sequence of frames is allocated contiguously.
 This can cause problems.
 
 The class ContFramePool has the ability to allocate either single frames,
 or sequences of contiguous frames. This affects how we manage the
 free frames. In SimpleFramePool it is sufficient to maintain the free 
 frames.
 In ContFramePool we need to maintain free *sequences* of frames.
 
 This can be done in many ways, ranging from extensions to bitmaps to 
 free-lists of frames etc.
 
 IMPLEMENTATION:
 
 One simple way to manage sequences of free frames is to add a minor
 extension to the bitmap idea of SimpleFramePool: Instead of maintaining
 whether a frame is FREE or ALLOCATED, which requires one bit per frame, 
 we maintain whether the frame is FREE, or ALLOCATED, or HEAD-OF-SEQUENCE.
 The meaning of FREE is the same as in SimpleFramePool. 
 If a frame is marked as HEAD-OF-SEQUENCE, this means that it is allocated
 and that it is the first such frame in a sequence of frames. Allocated
 frames that are not first in a sequence are marked as ALLOCATED.
 
 NOTE: If we use this scheme to allocate only single frames, then all 
 frames are marked as either FREE or HEAD-OF-SEQUENCE.
 
 NOTE: In SimpleFramePool we needed only one bit to store the state of 
 each frame. Now we need two bits. In a first implementation you can choose
 to use one char per frame. This will allow you to check for a given status
 without having to do bit manipulations. Once you get this to work, 
 revisit the implementation and change it to using two bits. You will get 
 an efficiency penalty if you use one char (i.e., 8 bits) per frame when
 two bits do the trick.
 
 DETAILED IMPLEMENTATION:
 
 How can we use the HEAD-OF-SEQUENCE state to implement a contiguous
 allocator? Let's look a the individual functions:
 
 Constructor: Initialize all frames to FREE, except for any frames that you 
 need for the management of the frame pool, if any.
 
 get_frames(_n_frames): Traverse the "bitmap" of states and look for a 
 sequence of at least _n_frames entries that are FREE. If you find one, 
 mark the first one as HEAD-OF-SEQUENCE and the remaining _n_frames-1 as
 ALLOCATED.

 release_frames(_first_frame_no): Check whether the first frame is marked as
 HEAD-OF-SEQUENCE. If not, something went wrong. If it is, mark it as FREE.
 Traverse the subsequent frames until you reach one that is FREE or 
 HEAD-OF-SEQUENCE. Until then, mark the frames that you traverse as FREE.
 
 mark_inaccessible(_base_frame_no, _n_frames): This is no different than
 get_frames, without having to search for the free sequence. You tell the
 allocator exactly which frame to mark as HEAD-OF-SEQUENCE and how many
 frames after that to mark as ALLOCATED.
 
 needed_info_frames(_n_frames): This depends on how many bits you need 
 to store the state of each frame. If you use a char to represent the state
 of a frame, then you need one info frame for each FRAME_SIZE frames.

 A WORD ABOUT RELEASE_FRAMES():
 
 When we release a frame, we only know its frame number. At the time
 of a frame's release, we don't know necessarily which pool it came
 from. Therefore, the function "release_frame" is static, i.e., 
 not associated with a particular frame pool.
 
 This problem is related to the lack of a so-called "placement delete" in
 C++. For a discussion of this see Stroustrup's FAQ:
 http://www.stroustrup.com/bs_faq2.html#placement-delete
 
 */
/*--------------------------------------------------------------------------*/


/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "cont_frame_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"

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
/* METHODS FOR CLASS   C o n t F r a m e P o o l */
/*--------------------------------------------------------------------------*/

// creating a linked list variable to store frame head
ContFramePool * ContFramePool::head;
ContFramePool * ContFramePool::tail;

// functions to get and set state of the frames (consecutive)

ContFramePool::FrameState ContFramePool::get_state(unsigned long _frame_no) {
    unsigned int bitmap_uf_index = _frame_no / 8; // this returns entire byte of 8 frame statuses
    unsigned char maskIndex = 0x1 << (_frame_no % 8); // this will shift a mask till desired frame bit in that byte
    // & will see if the frame & 1 = 1-> means bit is free (1 is free) else its used !!
    unsigned int bitmap_head_index = _frame_no / 8;
    unsigned char maskHead = 0x1 << (_frame_no % 8);

    // states can be for used, free, usedHead (HoS) other possibilities are not likely here
    // we conbine 2 bitmaps to get the final state
    if (((bitmapUfInfor[bitmap_uf_index] & maskIndex) == 0) && ((bitmapHeadInfo[bitmap_head_index] & maskHead) == 0))
    {

        return FrameState::HoS;
    }  
    else if (((bitmapUfInfor[bitmap_uf_index] & maskIndex) == 0) && ((bitmapHeadInfo[bitmap_head_index] & maskHead) != 0))
    {

        return FrameState::Used;
    }
    else
    {

        return FrameState::Free;
    }
}

void ContFramePool::set_state(unsigned long _frame_no, FrameState _state) {
    // getting respective byte and mask
    unsigned int bitmap_uf_index = _frame_no / 8;
    unsigned char maskIndex = 0x1 << (_frame_no % 8);

    unsigned int bitmap_head_index = _frame_no / 8;
    unsigned char maskHead = 0x1 << (_frame_no % 8);

    // here we check what state is to be set and accordingly use mask to set bitmap bit
    // we also update HoS in each case to indicate if used bit is head of a frame pool
    switch(_state) {
      case FrameState::Used:
      bitmapUfInfor[bitmap_uf_index] ^= maskIndex;
      bitmapHeadInfo[bitmap_head_index] |= maskHead; //set not head

      break;

    case FrameState::Free:
      bitmapUfInfor[bitmap_uf_index] |= maskIndex;
      bitmapHeadInfo[bitmap_head_index] |= maskHead;

      break;
    case FrameState::HoS:
    bitmapUfInfor[bitmap_uf_index] ^= maskIndex;
      bitmapHeadInfo[bitmap_head_index] ^= maskHead;

    }
    
}


ContFramePool::ContFramePool(unsigned long _base_frame_no,
                             unsigned long _n_frames,
                             unsigned long _info_frame_no)
{
    base_frame_no = _base_frame_no;
    nframes = _n_frames;
    nFreeFrames = _n_frames;
    info_frame_no = _info_frame_no;
    // If _info_frame_no is zero then we keep management info in the first and 2nd 
    //frame, else we use the provided frame and next frame to it to keep management info
    if(info_frame_no == 0) 
    {
        bitmapUfInfor = (unsigned char *) (base_frame_no * FRAME_SIZE);
        bitmapHeadInfo = (unsigned char *) ((base_frame_no+1) * FRAME_SIZE);
    } 
    else 
    {
        bitmapUfInfor = (unsigned char *) (info_frame_no * FRAME_SIZE);
        bitmapHeadInfo = (unsigned char *) ((info_frame_no+1) * FRAME_SIZE);
    }

    // Everything ok. Proceed to mark all frame as free.
    for(int fno = 0; fno < _n_frames; fno++) {
        set_state(fno, FrameState::Free);
    }
    
    // Mark the first and second frame as being used if they are being used
    if(_info_frame_no == 0) {
        set_state(0, FrameState::Used);
        set_state(1, FrameState::Used);
        nFreeFrames -= 2;
    }
    else if(_info_frame_no != 0) // mark the management frame as used if its in other place
    {
        set_state(info_frame_no, FrameState::Used);
        set_state(info_frame_no+1, FrameState::Used);
        nFreeFrames -= 2;
    }
    // adding pool to linked list
    if(ContFramePool::head == NULL) {
        // if this is the first frame then create is as the new LL 
        ContFramePool::head=this;
        ContFramePool::tail=this;
    }
    else{
        /// if this is the second pool then mark accordingly as a next node to the exiosting lsit
        ContFramePool::tail->next=this;
        ContFramePool::tail=this;
    }
    next=NULL; // make next of new node as null
    Console::puts("Frame Pool initialized\n");
}

unsigned long ContFramePool::get_frames(unsigned int _n_frames)
{
    // n frames left to allocate?
    assert(nFreeFrames >= _n_frames);
    int no_frame_flag = 0;
    int available_cont_frames;
    // Find a set of n frame that are not being used and return the frame index of head
    // Mark those frames as being used in the bitmap.
    unsigned int frame_no = 0; //used to iterate
    unsigned int total_frames = size_of_memory/FRAME_SIZE ; // 32MB / frame size
    //Console::puts("Total Frames\n");
    //Console::puti(total_frames);
    //Console::puts("\n");
    //Console::puts("Allotment started/n"); //
    //Console::puts("\n");
    //Console::puts("Free before\n");
    ///Console::puti(nFreeFrames);
    //Console::puts("\n");
    while(frame_no < total_frames) {
    	//Console::puts("1 ");
        available_cont_frames=0; // initialized at each frame index to cehck consecurtive frames from thsi point
        if (frame_no >= total_frames) // if frame limit exceeded
        {
            no_frame_flag = 1;
            break;
        }
        // now run a second loop to echk if n frames are available starting from current frame index
        for(int z=0; z<_n_frames; z++) 
        {
            //Console::puts("2 ");
            if (frame_no+z >= total_frames)
            {
                no_frame_flag = 1;
                break;
            }
            if(get_state(frame_no+z) == FrameState::Free)
                available_cont_frames ++; // checking free frames only
            else
                break; // if there are less than n frames from current frame idx
            if (available_cont_frames == _n_frames) // got frame idx with n frames free from it
                break;  
        }
        if (available_cont_frames == _n_frames)
        {
//          Console::puts("3 broke??");
            break; //got n free
        }
        frame_no++;   // this index does not provide n free frames so goto next frame idx
    }
    assert(no_frame_flag == 0); // we make sure we have not exceeded the limit
    //Console::puts("no memory space left!!/n");
    assert(available_cont_frames >= _n_frames);
    // now since we got free frames- make 1st frame of set as head
    set_state(frame_no, FrameState::HoS);
    for(int p = 1; p<_n_frames; p++)
    {
     set_state(frame_no+p, FrameState::Used);   // makr other frames as used
    }
    nFreeFrames -= _n_frames; // decr the # free frames
   // Console::puts("Free after\n");
    //Console::puti(nFreeFrames);
    //Console::puts("\n");
    return (frame_no + base_frame_no); // return the idx of frames allotted
}

void ContFramePool::mark_inaccessible(unsigned long _base_frame_no,
                                      unsigned long _n_frames)
{
    set_state(_base_frame_no, FrameState::HoS); // set the base frame as head 
    for(int p = 1; p<_n_frames; p++)
    {
     set_state(_base_frame_no+p, FrameState::Used); // now set the frames used since allocated
    }
}

void ContFramePool::release_the_frame(unsigned long _first_frame_no)
{
    if(get_state(_first_frame_no-base_frame_no)!=FrameState::HoS){
        return; // return as release is invalid
    }
    else{
        unsigned long firstFr = _first_frame_no;
        //Releasing HoS
        set_state(firstFr - base_frame_no,FrameState::Free);
        nFreeFrames ++;
        firstFr ++;
        //Releasing the rest of the contiguous frames in the set of frames
        while(firstFr-base_frame_no<nframes && get_state(firstFr - base_frame_no)== FrameState::Used)
        {
            set_state(firstFr - base_frame_no,FrameState::Free);     //release a frame
            nFreeFrames++;                  // free frame count incr
            firstFr++;
        }
    }
}

void ContFramePool::release_frames(unsigned long _first_frame_no)
{
    // check pool where the frame belongs
    ContFramePool* thisPool = ContFramePool::head;
    while(thisPool != NULL)
    {
        if((_first_frame_no >= thisPool->base_frame_no) && (_first_frame_no < thisPool->base_frame_no + thisPool->nframes))
        {
            // we found the required frame pool so proceed
            break;
        }
        thisPool = thisPool->next; // otheriwse fgo to the next pool!
    }
    
    // The frames not found
    if(thisPool==NULL){
        Console::puts("Frames are invalid / not found \n");
        return;  // cant release so return
    }
    //Release the frames if found
    thisPool->release_the_frame(_first_frame_no);
    
}

unsigned long ContFramePool::needed_info_frames(unsigned long _n_frames)
{
    return _n_frames / size_of_memory + (_n_frames % size_of_memory > 0 ? 1 : 0);
    // in this case we simply return # frames that are needed
}

