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
    VMPool * PageTable::vm_pool_head = NULL;

    void PageTable::init_paging(ContFramePool * _kernel_mem_pool,
                                ContFramePool * _process_mem_pool,
                                const unsigned long _shared_size)
    {
        // assert(false);
        // here we simply initialize the variables to initialize paging
        kernel_mem_pool = _kernel_mem_pool;
        process_mem_pool = _process_mem_pool;
        shared_size = _shared_size;
        Console::puts("Initialized Paging System\n");
    }

    PageTable::PageTable()
    {
        // assert(false);
        // we construct the page table in 2 parts - PDE and PTE
        // keeping the pde in kernel 
        page_directory = (unsigned long *) (kernel_mem_pool->get_frames(1)*PAGE_SIZE); 
        // creating the page in process frame pool
        unsigned long *page_table = (unsigned long *) (process_mem_pool->get_frames(1)*PAGE_SIZE); 
        unsigned long address=0; // holds the physical address of where a page is
        unsigned int i;

    // map the first 4MB of memory
    for(i=0; i<1024; i++)
    {
        page_table[i] = address | 3; // attribute set to: supervisor level, read/write, present(011 in binary) 
        address = address + 4096; // 4096 = 4kb --> we got to next frame as extry in table is per frame 
    }   

    page_directory[0] = (unsigned long)(page_table); // attribute set to: supervisor level, read/write, present(011 in binary)
    page_directory[0] = page_directory[0] | 3;
    // now set up a recursive page directory so that the last entry willn refer to the first entry of PDE
    // this way we can use logial addresses to refer to the page dir
    page_directory[1023] = (unsigned long) page_directory; 
    // make the last entry valid as it refers to the first entry to create effectively the recursive page table
    page_directory[1023] = page_directory[1023] | 3;
    for(i=1; i<1023; i++)
    {
        page_directory[i] = 0 | 2; 
        // making other page directory entries invalid! ofcourse now entry 1 and 1023 are valid!
        // attribute set to: supervisor level, read/write, not present(010 in binary)
    } 
        Console::puts("Constructed Page Table object\n");
    }

    void PageTable::load()
    {
        //assert(false);
        current_page_table = this; // just make the created page table as the current page table 
        Console::puts("Loaded page table\n");
    }

    void PageTable::enable_paging()
    {
        write_cr3((unsigned long)(current_page_table->page_directory));
        write_cr0(read_cr0() | 0x80000000); // set the paging bit in CR0 to 1
        paging_enabled = 1;
        Console::puts("Enabled paging\n");
    }

    void PageTable::handle_fault(REGS * _r)
    {
        //assert(false);
        unsigned long page_fault_addr = read_cr2(); //get the virtual addr of page fault
        unsigned long pde_idx = page_fault_addr >> 22; // extract the 10 MSBs to get Page directory(pde) index
        unsigned long pte_idx = ((page_fault_addr >> 12) & 0x3FF); // extract bit 12 to 21 to get the page table inde
        unsigned long* page_dir_addr = current_page_table->page_directory;

        // now first we will check if the address is legit -  we scan through our linked list that consists of all the virtual memory pools allcoated 
        // if the address lies within any of those pools- we can be sure that its legit

        // check if legit-
        /* temperorily i am setting this var- delete affter creation of check_legit funtion */ 
        unsigned int is_legit = 0;
        VMPool *this_pool_ptr = PageTable::vm_pool_head;

        for(;this_pool_ptr != NULL; this_pool_ptr=this_pool_ptr->next_pool_ptr)
        {
            if(this_pool_ptr->is_legitimate(page_fault_addr) == true)
                {
                    is_legit = 1;
                    break;
                }
        }
        // end of checking

        if(is_legit == 0 and this_pool_ptr != NULL)
        {
            Console::puts("Entry is not legitimate\n");
            assert(false);
    }
    // now handle page fault
        if ((page_dir_addr[pde_idx] & 1) == 0)
        {
            // if pde is not valid then get a kernel frame pool's frame and assign to the entry of pde 
            unsigned long* new_pde_frame = (unsigned long *) (process_mem_pool->get_frames(1)*PAGE_SIZE); // getting the page from process memory pool
            // mark in pde
            unsigned long* recursive_adjustment = (unsigned long *) (0xFFFFF0000);
            recursive_adjustment[pde_idx] = (unsigned long) (new_pde_frame);
            recursive_adjustment[pde_idx] = recursive_adjustment[pde_idx] | 3; // set meta data to R/W, supervisor and present 
        }
        // managing the invalid PTEs
            // if pte is not valid then get a frame from process frame pool and assign it there
        unsigned long* new_pte_frame = (unsigned long *) (process_mem_pool->get_frames(1)*PAGE_SIZE); 
        // make entry in pt
        unsigned long *pte_adj = (unsigned long *)((0x3FF<< 22)| (pde_idx <<12));
        pte_adj[pte_idx] = (unsigned long ) (new_pte_frame) | 3; // set meta data to R/W,supervisor and present 
    
        Console::puts("handled page fault\n");
    }

    void PageTable::register_pool(VMPool * _vm_pool)
    {
        if(PageTable::vm_pool_head == NULL)
            PageTable::vm_pool_head = _vm_pool;
        else{
            VMPool *head_of_pool = PageTable::vm_pool_head;
            for(;head_of_pool != NULL; head_of_pool=head_of_pool->next_pool_ptr)
                head_of_pool->next_pool_ptr = _vm_pool;
        }
        Console::puts("registered VM pool\n");
    }

    void PageTable::free_page(unsigned long _page_no) {
        unsigned long pde_idx = (_page_no & 0xFFC)>>22;
        unsigned long pte_idx = (_page_no & 0x003FF ) >>12 ;

        unsigned long *pte_entry= (unsigned long *) ( (0x000003FF << 22) | (pde_idx << 12) );
        unsigned long frame_no = (pte_entry[pte_idx] & 0xFFFFF000)/ PAGE_SIZE;
        
        process_mem_pool->release_frames(frame_no);
        pte_entry[pte_idx] |= 2; // MARK INVALID


        // flush TLB
        load();
        Console::puts("freed page\n");
    }

