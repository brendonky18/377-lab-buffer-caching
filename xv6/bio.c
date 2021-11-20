// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.
//
// The implementation uses two state flags internally:
// * B_VALID: the buffer data has been read from the disk.
// * B_DIRTY: the buffer data has been modified
//     and needs to be written to disk.

#include "types.h"
#include "defs.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "fs.h"
#include "buf.h"

struct
{
    struct spinlock lock; // Lock to ensure that multiple processes can't access/modify the same buffer simultaneously
    struct buf buf[NBUF];

    // Linked list of all buffers, through prev/next.
    // head.next is most recently used.
    struct buf head;
} bcache;

uint hitcount;
uint misscount;

/**
 * inits the buffer cache linked list
 */
void binit(void)
{
    struct buf *b;

    initlock(&bcache.lock, "bcache");

    //PAGEBREAK!
    // Create double-linked list of buffers
    bcache.head.prev = &bcache.head;
    bcache.head.next = &bcache.head;
    for (b = bcache.buf; b < bcache.buf + NBUF; b++)
    {
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        initsleeplock(&b->lock, "buffer");
        bcache.head.next->prev = b;
        bcache.head.next = b;
    }

    // tracks the hit rate of the eviction policy
    hitcount = 0;
    misscount = 0;
}

/**
 * @brief Looks for the passed buffer cache
 * Look through buffer cache for block on device dev.       
 * If not found, allocate a buffer.       
 * In either case, return locked buffer.
 * @param dev the device number of the block being searched for
 * @param blockno the block/sector number being searched for
 * @return struct buf* 
 * A pointer to the buffer corresponding to the sector requested 
 */
static struct buf *
bget(uint dev, uint blockno)
{
    struct buf *b;

    acquire(&bcache.lock);

    // Is the block already cached?
    for (b = bcache.head.next; b != &bcache.head; b = b->next)
    {
        if (b->dev == dev && b->blockno == blockno)
        {
            hitcount++;

            b->refcnt++;
            release(&bcache.lock);
            acquiresleep(&b->lock);
            return b;
        }
    }

    misscount++;
    /**
     * TODO:
     * Modify the eviction policy:
     *     Get the tail of the buffer cache's linked list
     *     If the buffer is in use, sleep until it is free
     *     Update the buffer's metadata, mark as invalid
     * 
     * NOTE:
     * This may result in a deadlock, how could you resolve this?
     */
    // Not cached; recycle an unused buffer.
    // Even if refcnt==0, B_DIRTY indicates a buffer is in use
    // because log.c has modified it but not yet committed it.
    for (b = bcache.head.prev; b != &bcache.head; b = b->prev)
    {
        if (b->refcnt == 0 && (b->flags & B_DIRTY) == 0)
        {
            b->dev = dev;
            b->blockno = blockno;
            b->flags = 0;
            b->refcnt = 1;
            
            /**
             * TODO:
             * Modify insertion to the linked list to be sorted in FIFO order: 
             *     Remove the evicted buffer cache from it's current position
             *     Update the surrounding node's pointers
             *     Move to the head of the linked list
             */

            release(&bcache.lock);
            acquiresleep(&b->lock);
            return b;
        }
    }
    panic("bget: no buffers");
}

// Return a locked buf with the contents of the indicated block.
struct buf *
bread(uint dev, uint blockno)
{
    struct buf *b;

    b = bget(dev, blockno);
    if ((b->flags & B_VALID) == 0)
    {
        iderw(b);
    }
    return b;
}

// Write b's contents to disk.  Must be locked.
void bwrite(struct buf *b)
{
    // mark the block as dirty so that iderw knows to write it out to disk
    if (!holdingsleep(&b->lock))
        panic("bwrite");
    b->flags |= B_DIRTY;
    iderw(b);
}

// Release a locked buffer.
// Move to the head of the MRU list.
void brelse(struct buf *b)
{
    if (!holdingsleep(&b->lock))
        panic("brelse");

    releasesleep(&b->lock);

    acquire(&bcache.lock);
    b->refcnt--;
    /**
     * TODO:
     * Remove, so that the order of the buffer cache's linked list is fixed
     */
    if (b->refcnt == 0)
    {
        // no one is waiting for it.
        b->next->prev = b->prev;
        b->prev->next = b->next;
        b->next = bcache.head.next;
        b->prev = &bcache.head;
        bcache.head.next->prev = b;
        bcache.head.next = b;
    }

    release(&bcache.lock);
}

/**
 * @brief Calculates the hitrate of the buffer cache
 * @return float: The hit rate
 */
float bhitrate(void) 
{
    uint count = hitcount + misscount;
    if(count == 0) {
        return 1.0; // return 1 to avoid divide by 0 error
    } else {
        return (float) hitcount / count;
    }
}

/**
 * @brief Resets the hit rate, sets both counters to 0
 * 
 * @return int 0
 */
int bresethitrate(void) {
    hitcount = 0;
    misscount = 0;
    return 0;
}
//PAGEBREAK!
// Blank page.
