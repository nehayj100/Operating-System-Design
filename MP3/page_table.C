#include "assert.H"
#include "exceptions.H"
#include "console.H"
#include "paging_low.H"
#include "page_table.H"

PageTable * PageTable::current_page_table = NULL;
unsigned int PageTable::paging_enabled = 0;
ContFramePool * PageTable::kernel_mem_pool = NULL;
ContFramePool * PageTable::process_mem_pool = NULL;
unsigned long PageTable::shared_size = 0;


void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                            ContFramePool * _process_mem_pool,
                            const unsigned long _shared_size)
{
   // initialize the class variable passed arguments
   kernel_mem_pool = _kernel_mem_pool;
   process_mem_pool = _process_mem_pool;
   shared_size = _shared_size;
   Console::puts("Initialized Paging System\n");
}

PageTable::PageTable()
{
   // get frames from kernel mem pool to store the page table and page directory!
   page_directory = (unsigned long *) (kernel_mem_pool->get_frames(1)*PAGE_SIZE); 
   // creating a directory (in 1 frame of kernel memory space ie initial 4 MB) for the page table I'm trying to create for the currrent process 
   unsigned long *page_table = (unsigned long *) (kernel_mem_pool->get_frames(1)*PAGE_SIZE); 
   // creating the page table (in 1 frame of the kernel meory space ie initial 4 MB) 
   // every time I initlalize this page table- i want to add entries in the table that are directly mapped (first 4 <B of the memory space)
   unsigned long address=0; // holds the physical address of where a page is
   unsigned int i;

   // map the first 4MB of memory
   for(i=0; i<1024; i++)
   {
      // at each index i- I get the base index of each frame in physical memory- which i map to same addr ie frame no= page no
      // I imagine this as the memory which has eg. 0000 to ffff will goto 000 to ffff in the RAM (now this 0000 to ffff was given by cpu as logical addr)
      // now since I have 12 extra offset bits that are not a part of entry I use them as meta data- here were told what the bits represent so I am simple using them to set my frame info
      page_table[i] = address | 3; // attribute set to: supervisor level, read/write, present(011 in binary) 
      address = address + 4096; // 4096 = 4kb --> we got to next frame as extry in table is per frame 
   }   

   // now since we have made the page table- we will update the page table address i the directory 
   // I unsderstand that the page table directory will now have only 1 entry as we are dealing with only 1 process and only 1 page table of that process
   page_directory[0] = (unsigned long)(page_table); // attribute set to: supervisor level, read/write, present(011 in binary)
   page_directory[0] = page_directory[0] | 3;

   // make toehr entries invalid!
   for(i=1; i<1024; i++)
   {
      page_directory[i] = 0 | 2; 
      // attribute set to: supervisor level, read/write, not present(010 in binary)
   } 
   Console::puts("Constructed Page Table object\n");
}


void PageTable::load()
{
   current_page_table = this; // just make the created page table as the current page table 
   Console::puts("Loaded page table\n");
}

void PageTable::enable_paging()
{
   // write into CR3 - the adress of page table directory
   write_cr3((unsigned long)(current_page_table->page_directory));
   write_cr0(read_cr0() | 0x80000000); // set the paging bit in CR0 to 1
   paging_enabled = 1;
   Console::puts("Enabled paging\n");
}

void PageTable::handle_fault(REGS * _r)
{
   unsigned long page_fault_addr = read_cr2(); //get the virtual addr of page fault
   unsigned long pde_idx = page_fault_addr >> 22; // extract the 10 MSBs to get Page directory(pde) index
   unsigned long pte_idx = ((page_fault_addr >> 12) & 0x3FF); // extract bit 12 to 21 to get the page table index
   
   unsigned long* page_dir_addr = current_page_table->page_directory;
   // mask the meta data so you can actually get the pte address
   unsigned long *pde_entry_without_meta_data = (unsigned long *) (page_dir_addr[pde_idx] & ~0xFFF); 
   unsigned long *pte_entry = (unsigned long *) pde_entry_without_meta_data[pte_idx]; // get the pte address
  // check if pde index addr has valid bit =1- if not valid then handle pde fault
   if ((page_dir_addr[pde_idx] & 1) == 0)
   {
      // if pde is not valid then get a kernel frame pool's frame and assign to the entry of pde 
      unsigned long* new_pde_frame = (unsigned long *) (kernel_mem_pool->get_frames(1)*PAGE_SIZE); 
      // mark in pde
      page_dir_addr[pde_idx] = (unsigned long) (new_pde_frame);
      page_dir_addr[pde_idx] = page_dir_addr[pde_idx]| 3; // set meta data to R/W, supervisor and present 
      for(unsigned long i=1;i<ENTRIES_PER_PAGE;i++)
      {
         new_pde_frame[i] = 2; // make toehr entries invalid
      }
   }
   // go to pte and check if that entry is valid
    if ((pde_entry_without_meta_data[pte_idx] & 1) == 0)
   {
      // if pte is not valid then get a frame from process frame pool and assign it there
      unsigned long* new_pte_frame = (unsigned long *) (process_mem_pool->get_frames(1)*PAGE_SIZE); 
      // make entry in pt
      pde_entry_without_meta_data[pte_idx] = (unsigned long) (new_pte_frame);
      pde_entry_without_meta_data[pte_idx] = pde_entry_without_meta_data[pte_idx]| 3; // set meta data to R/W,supervisor and present 
   }
   Console::puts("handled page fault\n");
   // good to return! 
}
