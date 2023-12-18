/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2021/11/28

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
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
#include "file.H"
#include "file_system.H"
extern FileSystem *FILE_SYSTEM;
/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR/DESTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File(FileSystem *_fs, int _id)
{

    Console::puts("the overall curr clk is:");
    Console::puti(overall_current_block);
    Console::puts("\n");
    file_id = _id;
    size = 0;
    Console::puts("the blocked assinged is: ");
    Console::puti(current_block);
    Console::puts("\n");
    Console::puts("for file with id: ");
    Console::puts("\n");
    Console::puti(_id);

    // }

    // craeting the inode for the file
    position = 0;

    Inode *final_node = _fs->LookupFile(_id);
    current_block = final_node->curr_blk_file;

    Console::puts("Opening file.\n");
    // assert(false);
}

File::~File()
{
    position = 0;
    Console::puts("Closing file.\n");
    /* Make sure that you write any cached data to disk. */
    /* Also make sure that the inode in the inode list is updated. */
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char *_buf)
{
    // get # blocks ot be read
    Inode *node = FILE_SYSTEM->LookupFile(this->file_id);
    unsigned int blocks_to_read = node->file_size_in_blks;

    Console::puts("printing number of blocks: ");
    Console::puti(blocks_to_read);
    int bytes_left = _n;
    int block = 0;
    unsigned int itr_curr_blk = current_block;
    while (blocks_to_read > 0)
    {
        // unsigned char buf[SimpleDisk::BLOCK_SIZE] = {0};
        if (itr_curr_blk <= 1)
            assert(false);

        memset(buf, 0, SimpleDisk::BLOCK_SIZE);
        FILE_SYSTEM->disk->read(itr_curr_blk, buf);

        position = 0;
        // /byte_left > 0 &&
        while (position < SimpleDisk::BLOCK_SIZE && (bytes_left > 0)) // chars to read
        {
            // Console::puts("reading in blk:");
            // Console::puti(block);
            // Console::puts("char read is: \n");
            // char ch = (char)buf[position];
            // Console::putch(ch);
            // Console::puts("\n");

            _buf[block++] = (char)buf[position++];
            //_n--;
          //  Console::puts("THIS IS SPTRED IN BUF\n");
         //   int k = block - 1;
            // Console::putch(_buf[k]);
            // Console::puts("\n");
            bytes_left--;
            // decr chars to read
            // if (position == SimpleDisk::BLOCK_SIZE - 1 || bytes_left <= 0)
        }

        // bytes_left -= 512;
        itr_curr_blk++;
        blocks_to_read--;
    }
  //  Console::puts("I have read this in read block before returning: ");
  //  Console::puts(_buf);
  Console::puts("\n");
  Console::puts("Reading Done\n");
    return block;
}

// _buf will have all the bytes- its iteration would be continuous but blocks written 1 by 1 so reset block index at the end of each iteration
// at a time read/write of 1 block happens in simple sdisk so simply repeat this operation #block times so as to read or write big files!
// pass the num of blocke to be written and have an overall outer loop there- this will also increase overall_block_num every time a block is written
// to get number of blocks we need to [ass the file size and then calculate the number of blocks in the file before we read and write
// also now as we read or wrrite more blocks we need to udpat ethe inode with number of blocks information
// this can be done by looking up the inode for the file from inode list and udpating the number of blocks of that file in the inode
// this is needed as when we write and then read we need to know how much to read and write
// also update the freeblock bitmap
// might also need to change reset() - just make position 0 bcoz while reading we read from strating block till end (using #blocks)
// change EOF- block size* num_blocks - 1
// Inode *final_node = _fs->LookupFile(_id);
// do the following outside loop
// Inode *node = FILE_SYSTEM->LookupFile(this->file_id);
// node->num_blks = calculated # blcoks;

int File::Write(unsigned int _n, const char *_buf)
{
    // getting and setting the number of CONTIGUOUS BLOCKS allotted to the file
    Inode *node = FILE_SYSTEM->LookupFile(this->file_id);
    node->file_size_in_blks = (_n + SimpleDisk::BLOCK_SIZE - 1) / SimpleDisk::BLOCK_SIZE;
    unsigned int iterator_blks = (_n + SimpleDisk::BLOCK_SIZE - 1) / SimpleDisk::BLOCK_SIZE;

    int block = 0;
    int byte_left = _n;
    unsigned long it_curr_blk = current_block;

    while (iterator_blks > 0) // outer loop that loops each block
    {
        unsigned char buf[SimpleDisk::BLOCK_SIZE] = {0}; // Allocate buf

        if (it_curr_blk <= 1)
            assert(false);

        unsigned int bytenm = it_curr_blk / 8;
        unsigned int bitnm = it_curr_blk % 8;
        FILE_SYSTEM->freeblk[bytenm] = (0x80 >> bitnm) | FILE_SYSTEM->freeblk[bytenm];

        memset(buf, 0, SimpleDisk::BLOCK_SIZE);
        position = 0; // reset position as we start to read from new blk now

        while (byte_left > 0 && position < SimpleDisk::BLOCK_SIZE)
        {
            // Console::puts("char read is: \n");
            // char ch = _buf[block];
            // Console::putch(ch);
            // Console::puts("\n");

            buf[position++] = _buf[block++];
            // block_cache[block++] = (char) _buf[position++];
            // Console::puts("write in blk:");
            // Console::puti(block);
            // Console::puts("\n");

            if (position == SimpleDisk::BLOCK_SIZE || byte_left <= 0) /////////////////// 01 rem
                break;
            byte_left--;
        }
        // Console::puts("CHECK CHECK CHECK !!!!!!!!!:    ");
        // Console::puti(it_curr_blk);
        // Console::puts("\n");

        FILE_SYSTEM->disk->write(it_curr_blk, buf);
        overall_current_block++;

        //Console::puts("writing to file\n");

        it_curr_blk++;
        iterator_blks--;
    }
    // size += _n;
  //  Console::puts("exiting\n");
  Console::puts("Writing Done\n");
    return block;
}

void File::Reset()
{
    Console::puts("resetting file\n");
    this->position = 0;
    // assert(false);
}

bool File::EoF()
{
    Console::puts("checking for EoF\n");
    //   assert(false);
    return (position > SimpleDisk::BLOCK_SIZE - 1);
}
