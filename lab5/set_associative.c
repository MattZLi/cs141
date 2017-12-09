#include <stdint.h>

#include "memory_block.h"
#include "set_associative.h"

set_associative_cache* sac_init(main_memory* mm)
{
    // allocate space
    set_associative_cache* result = malloc(sizeof(set_associative_cache));

    result->mm = mm;
    result->cs = cs_init();

    for (int i = 0; i < SET_ASSOCIATIVE_NUM_SETS; i++)
    {
        for (int j = 0; j < SET_ASSOCIATIVE_NUM_WAYS; j++)
        {
            //making an array of size sets * ways
            result->valid[i][j] = 0;
            result->dirty[i][j] = 0;
            result->use[i][j] = j;
        }
    }

    return result;
}

// helper function to choose a set
static int addr_to_set(void* addr)
{
    return ((uintptr_t)addr >> MAIN_MEMORY_BLOCK_SIZE_LN) % (SET_ASSOCIATIVE_NUM_SETS);
}

// helper function to return least recently used
static void mark_as_used(set_associative_cache* sac, int set, int way)
{
    int temp = sac->use[set][way];
    for (int i = 0; i < SET_ASSOCIATIVE_NUM_WAYS; i++)
    {
        if (sac->use[set][i] > temp)
        {
            --sac->use[set][i];
        }
    }
    // highest use value indicates most recently used
    sac->use[set][way] = SET_ASSOCIATIVE_NUM_WAYS - 1;
}

// helper function to return least recently used
static int lru(set_associative_cache* sac, int set)
{
    int wayns;
    // search for least recently used
    for (int i = 0; i < SET_ASSOCIATIVE_NUM_WAYS; i++)
    {
        if (sac->use[set][i] == 0)
        {
            wayns = i;
        }
    }
    return wayns;
}

// global variables to track if address exists in cache
int founds = 0;
int wayns = -1;
// helper function to search for address in cache
void finds(set_associative_cache* sac, void* mb_start_addr, int set)
{
    for (int i = 0; i < SET_ASSOCIATIVE_NUM_WAYS; i++)
    {
        if ((sac->valid[set][i]) && (mb_start_addr == sac->blks[set][i]->start_addr))
        {
            founds = 1;
            wayns = i;
        }
    }
}


void sac_store_word(set_associative_cache* sac, void* addr, unsigned int val)
{
    // Precompute start address of memory block, taken from simple.c
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;

    // select set to store in
    int setn = addr_to_set(addr);

    // select way to store in

    // first, check if any way already has the address
    finds(sac, mb_start_addr, setn);

    // if we can't find it, we must evict lru
    if (!founds)
    {
        wayns = lru(sac, setn);

        if ((sac->valid[setn][wayns]) && (sac->dirty[setn][wayns]))
        {
            // copy dirty stuff to main memory
            mm_write(sac->mm, sac->blks[setn][wayns]->start_addr, sac->blks[setn][wayns]);

            // remove dirty tag
            sac->dirty[setn][wayns] = 0;
        }

        sac->blks[setn][wayns] = mm_read(sac->mm, mb_start_addr);

        sac->valid[setn][wayns] = 1;
        sac->dirty[setn][wayns] = 0;

        // increment write misses
        ++sac->cs.w_misses;
    }

    // Update relevant word in memory block
    unsigned int* mb_addr = sac->blks[setn][wayns]->data + addr_offt;
    *mb_addr = val;

    // update lru
    mark_as_used(sac, setn, wayns);

    // set as dirty
    sac->dirty[setn][wayns] = 1;

    // increment write queries
    ++sac->cs.w_queries;

    // reset found bit
    founds = 0;
    wayns = -1;
}


unsigned int sac_load_word(set_associative_cache* sac, void* addr)
{
    // Precompute start address of memory block
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;

    // select set to store in
    int setn = addr_to_set(addr);

    // select way to store in

    // first, check if any way already has the address
    finds(sac, mb_start_addr, setn);

    // if we can't find it, we must evict lru
    if (!founds)
    {
        wayns = lru(sac, setn);

        if ((sac->valid[setn][wayns]) && (sac->dirty[setn][wayns]))
        {
            // copy dirty stuff to main memory
            mm_write(sac->mm, sac->blks[setn][wayns]->start_addr, sac->blks[setn][wayns]);

            // remove dirty tag
            sac->dirty[setn][wayns] = 0;
        }

        sac->blks[setn][wayns] = mm_read(sac->mm, mb_start_addr);

        sac->valid[setn][wayns] = 1;
        sac->dirty[setn][wayns] = 0;

        // increment write misses
        ++sac->cs.r_misses;
    }

    // Extract the word we care about
    unsigned int* mb_addr = sac->blks[setn][wayns]->data + addr_offt;
    unsigned int result = *mb_addr;

    // update lru
    mark_as_used(sac, setn, wayns);

    // increment write queries
    ++sac->cs.r_queries;

    // reset found
    founds = 0;
    wayns = -1;

    // Return result
    return result;
}

void sac_free(set_associative_cache* sac)
{
    free(sac);
}