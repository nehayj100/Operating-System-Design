/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"
#include "simple_disk.H"
/*--------------------------------------------------------------------------*/
/* CLASS Inode */
/*--------------------------------------------------------------------------*/

/* You may need to add a few functions, for example to help read and store 
   inodes from and to disk. */

/*--------------------------------------------------------------------------*/
/* CLASS FileSystem */
/*--------------------------------------------------------------------------*/

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

Inode::Inode() {
    id = this->id;
    thisFile= NULL;
    fs = this->fs;
    next = NULL;
}
    
unsigned char* FileSystem::freeblk = nullptr; // Initialize appropriately based on your implementation

FileSystem::FileSystem() {
 //   Inode *thisINode = new Inode();

    head = NULL;
    tail = NULL;
    freeblk = new unsigned char[SimpleDisk::BLOCK_SIZE];
    static unsigned int num_files = 0;  
    Console::puts("In file system constructor.\n");
    //assert(false);
}

FileSystem::~FileSystem() {
    Console::puts("unmounting file system\n");
    this->disk = NULL;
    /* Make sure that the inode list and the free list are saved. */
    //assert(false);
}


/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/


bool FileSystem::Mount(SimpleDisk * _disk) {
    if(this->disk == NULL)
    {
        this->disk = _disk;
        Console::puts("mounting file system from disk\n");
    }   
    else
    {
        return false;
        Console::puts("Disk mount error\n");
    }

    /* Here you read the inode list and the free list into memory */
    
    //assert(false);
    return true;
}

void set_bit(unsigned char *array, int num)
{
    unsigned int byte_num = num / 8;
    unsigned int bit_num = num % 8;
    array[byte_num] = (0x80 << bit_num) | array[byte_num];
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) { // static!
    Console::puts("formatting disk\n");
//    FileSystem::disk = _disk;

    unsigned int num_blocks = _size/SimpleDisk::BLOCK_SIZE;
   // unsigned char *superblk = new unsigned char[BLOCK_SIZE];
    
    
    set_bit(freeblk, 0);
    set_bit(freeblk, 1);

    for(int i=1; i<num_blocks; i++)
        freeblk[i] = 0;
    
    _disk->write(1,freeblk);

    /* Here you populate the disk with an initialized (probably empty) inode list
       and a free list. Make sure that blocks used for the inodes and for the free list
       are marked as used, otherwise they may get overwritten. */
    //assert(false);
    return true;
}

Inode * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file with id = "); Console::puti(_file_id); Console::puts("\n");
    /* Here you go through the inode list to find the file. */
    if(head == NULL)
        return NULL;
    Inode * ptr = head;
    Console::puts("came to head\n");
    int j=0;
    while(ptr != NULL)
    {
        if(ptr->id == _file_id)
            break;
        else
        {
            ptr = ptr->next;
        Console::puti(j);
        j++;
        Console::puts("\n");
        }
        
    }
        
    Console::puts("got ptr\n");
    return ptr;
    //assert(false);
}

bool FileSystem::CreateFile(int file_id) {
    Console::puts("creating file with id:"); 
    Console::puti(file_id); Console::puts("\n");
    Inode *ptr = head;

    if(overall_current_block >= 10*1024*1024/SimpleDisk::BLOCK_SIZE)
        overall_current_block = 2;

    if(this->LookupFile(file_id))
        return false;
    if(head == NULL)
    {   
        head = tail = new Inode();
        head->id = file_id;
        head->next = NULL;  
    }
    else
    {
        tail->next = new Inode();
        tail = tail->next;
        tail->id = file_id;
    }
    tail->file_size_in_blks = 1; //setting default file size- will change in write
    tail->curr_blk_file = overall_current_block += 128;

    /* Here you check if the file exists already. If so, throw an error.
       Then get yourself a free inode and initialize all the data needed for the
       new file. After this function there will be a new file on disk. */
   // assert(false);
    return true;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file with id:"); Console::puti(_file_id); Console::puts("\n");
    /* First, check if the file exists. If not, throw an error. 
       Then free all blocks that belong to the file and delete/invalidate 
       (depending on your implementation of the inode list) the inode. */
    Inode *curr = head;
    Inode *prev = NULL;


    if(!(this->LookupFile(_file_id)))
        return false;
    
    while(curr->id != _file_id && curr != NULL)
    {
        prev =curr;
        curr = curr->next;
    }
    if(prev == NULL)
        head = curr->next;
    prev->next = curr->next;

    return true;
}
