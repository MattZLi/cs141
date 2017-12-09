#include <stdint.h>

#include "memory_block.h"
#include "direct_mapped.h"

direct_mapped_cache* dmc_init(main_memory* mm)
{
    //allocate space
    direct_mapped_cache* result = malloc(sizeof(direct_mapped_cache));

    result->mm = mm;
    result->cs = cs_init();

    for (int i = 0; i < DIRECT_MAPPED_NUM_SETS; i++)
    {
        //making an array of size num sets
        result->valid[i] = 0;
        result->dirty[i] = 0;
    }

    return result;
}

// helper function to choose a set
static int addr_to_set(void* addr)
{
    return ((uintptr_t)addr >> MAIN_MEMORY_BLOCK_SIZE_LN) % (DIRECT_MAPPED_NUM_SETS);
}


void dmc_store_word(direct_mapped_cache* dmc, void* addr, unsigned int val)
{
    // Precompute start address of memory block
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;

    // select set to store in
    int setn = addr_to_set(addr);

    // if write miss AND dirty
    if ((dmc->valid[setn])
        && (mb_start_addr != dmc->sets[setn]->start_addr)
        && dmc->dirty[setn])
    {
        // copy dirty stuff to main memory
        mm_write(dmc->mm, dmc->sets[setn]->start_addr, dmc->sets[setn]);

        // remove dirty tag
        dmc->dirty[setn] = 0;
    }

    // if compulsory miss OR miss
    if ((!dmc->valid[setn]) || (mb_start_addr != dmc->sets[setn]->start_addr))
    {
        // load in from main memory
        dmc->sets[setn] = mm_read(dmc->mm, mb_start_addr);

        dmc->valid[setn] = 1;
        dmc->dirty[setn] = 0;

        // increment write misses
        ++dmc->cs.w_misses;
    }

    // Update relevant word in memory block
    unsigned int* mb_addr = dmc->sets[setn]->data + addr_offt;
    *mb_addr = val;

    // set as dirty
    dmc->dirty[setn] = 1;

    // increment write queries
    ++dmc->cs.w_queries;
}

unsigned int dmc_load_word(direct_mapped_cache* dmc, void* addr)
{
    // Precompute start address of memory block
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;

    // select set to store in
    int setn = addr_to_set(addr);

    // if read miss AND dirty
    if ((dmc->valid[setn])
        && (mb_start_addr != dmc->sets[setn]->start_addr)
        && (dmc->dirty[setn]))
    {
        // copy dirty stuff to main memory
        mm_write(dmc->mm, dmc->sets[setn]->start_addr, dmc->sets[setn]);

        // remove dirty tag
        dmc->dirty[setn] = 0;
    }

    // if compulsory miss OR miss
    if ((!dmc->valid[setn]) || (mb_start_addr != dmc->sets[setn]->start_addr))
    {
        // load in from main memory
        dmc->sets[setn] = mm_read(dmc->mm, mb_start_addr);

        dmc->valid[setn] = 1;
        dmc->dirty[setn] = 0;

        // increment read misses
        ++dmc->cs.r_misses;
    }

    // Extract the word we care about
    unsigned int* mb_addr = dmc->sets[setn]->data + addr_offt;
    unsigned int result = *mb_addr;

    // increment read queries
    ++dmc->cs.r_queries;

    // Return result
    return result;
}

void dmc_free(direct_mapped_cache* dmc)
{
    free(dmc);
}