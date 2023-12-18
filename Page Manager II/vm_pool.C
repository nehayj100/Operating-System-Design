/*
 File: vm_pool.C
 
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

#include "vm_pool.H"
#include "console.H"
#include "utils.H"
#include "assert.H"
#include "simple_keyboard.H"
#include "page_table.H"
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
/* METHODS FOR CLASS   V M P o o l */
/*--------------------------------------------------------------------------*/

VMPool::VMPool(unsigned long  _base_address,
               unsigned long  _size,
               ContFramePool *_frame_pool,
               PageTable     *_page_table) {
    //assert(false);
    base_address = _base_address;
    size = _size;
    frame_pool = _frame_pool;
    page_table = _page_table;
    next_pool_ptr = NULL;
    pool_number= 0;
    page_table->register_pool(this);

    registered_pool_info *this_pool = (registered_pool_info *) base_address;
    this_pool[0].size_vm_pool = PageTable::PAGE_SIZE;
    this_pool[0].base_address = base_address;
    pool_size_available -= PageTable::PAGE_SIZE;
    vm_pools = this_pool;
    pool_number += 1;

    Console::puts("Constructed VMPool object.\n");
}

unsigned long VMPool::allocate(unsigned long _size) {
    //assert(false);
    if(_size > pool_size_available)
    {
        Console::puts("vm pool does not have enough memeory");
        assert(false);
    }
    int rem;
    // find the number of pages required (take the next rounded off # pages and allocate from proces memory pool)
    if(_size % Machine::PAGE_SIZE == 0)
        rem = 0;
    else
        rem = 1;
    unsigned long num_pages = _size/Machine::PAGE_SIZE + rem;
    vm_pools[pool_number].base_address = vm_pools[pool_number - 1].base_address + vm_pools[pool_number - 1].size_vm_pool;
    vm_pools[pool_number].size_vm_pool = num_pages * PageTable::PAGE_SIZE;
    pool_number += 1;
    pool_size_available -= num_pages * PageTable::PAGE_SIZE;
    //unsigned long *allocated_pool_start = (unsigned long *) (PageTable::process_mem_pool->get_frames(1)* Machine::PAGE_SIZE); 
    Console::puts("Allocated region of memory.\n");
    return vm_pools[pool_number-1].base_address;
}

void VMPool::release(unsigned long _start_address) {
    //assert(false);
    int this_region;
    for(int i=0; i<pool_number; i++)
    {
        if(vm_pools[i].base_address == _start_address)
            this_region = i;
    }
    unsigned long cnt_pages_tobe_freed = vm_pools[this_region].size_vm_pool / PageTable::PAGE_SIZE;
    while (cnt_pages_tobe_freed > 0)
    {
        page_table->free_page(_start_address);
        cnt_pages_tobe_freed -= 1;
        _start_address += PageTable::PAGE_SIZE;
    }
    // now update the region array
    for(int i=this_region; i<pool_number; i++ )
    {
        vm_pools[i] = vm_pools[i+1];
        pool_number -= 1;
        pool_size_available = vm_pools[this_region].size_vm_pool;
    }
    Console::puts("Released region of memory.\n");
}

bool VMPool::is_legitimate(unsigned long _address) {
    // assert(false);
    Console::puts("Checked whether address is part of an allocated region.\n");
    
    if(_address < base_address || _address > base_address + size)
        return false;
    else 
        return true;
}

