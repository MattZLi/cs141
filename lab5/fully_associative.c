#include "memory_block.h"
#include "fully_associative.h"

fully_associative_cache* fac_init(main_memory* mm)
{
    // allocate space
    fully_associative_cache* result = malloc(sizeof(fully_associative_cache));

    result->mm = mm;
    result->cs = cs_init();

    for (int i = 0; i < FULLY_ASSOCIATIVE_NUM_WAYS; i++)
    {
        //making an array of size num ways
        result->valid[i] = 0;
        result->dirty[i] = 0;
        result->use[i] = i;
    }

    return result;
}

// helper function to set most recently used
static void mark_as_used(fully_associative_cache* fac, int way)
{
    int temp = fac->use[way];
    for (int i = 0; i < FULLY_ASSOCIATIVE_NUM_WAYS; i++)
    {
        if (fac->use[i] > temp)
        {
            --fac->use[i];
        }
    }
    // highest use value indicates most recently used
    fac->use[way] = FULLY_ASSOCIATIVE_NUM_WAYS - 1;
}

// helper function to return least recently used
static int lru(fully_associative_cache* fac)
{
    int wayn;
    // search for least recently used
    for (int i = 0; i < FULLY_ASSOCIATIVE_NUM_WAYS; i++)
    {
        if (fac->use[i] == 0)
        {
            wayn = i;
        }
    }
    return wayn;
}

// global variables to track if address exists in cache
int found = 0;
int wayn = -1;
// helper function to search for address in cache
void find(fully_associative_cache* fac, void* mb_start_addr)
{
    for (int i = 0; i < FULLY_ASSOCIATIVE_NUM_WAYS; i++)
    {
        if ((fac->valid[i]) && (mb_start_addr == fac->ways[i]->start_addr))
        {
            found = 1;
            wayn = i;
        }
    }
}


void fac_store_word(fully_associative_cache* fac, void* addr, unsigned int val)
{
    // Precompute start address of memory block, taken from simple.c
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;

    // select way to store in

    // first, check if any way already has the address
    find(fac, mb_start_addr);

    // if we can't find it, we must evict lru
    if (!found)
    {
        wayn = lru(fac);

        if ((fac->valid[wayn]) && (fac->dirty[wayn]))
        {
            // copy dirty stuff to main memory
            mm_write(fac->mm, fac->ways[wayn]->start_addr, fac->ways[wayn]);

            // remove dirty tag
            fac->dirty[wayn] = 0;
        }

        fac->ways[wayn] = mm_read(fac->mm, mb_start_addr);

        fac->valid[wayn] = 1;
        fac->dirty[wayn] = 0;

        // increment write misses
        ++fac->cs.w_misses;
    }

    // Update relevant word in memory block
    unsigned int* mb_addr = fac->ways[wayn]->data + addr_offt;
    *mb_addr = val;

    // update lru
    mark_as_used(fac, wayn);

    // set as dirty
    fac->dirty[wayn] = 1;

    // increment write queries
    ++fac->cs.w_queries;

    // reset found bit
    found = 0;
    wayn = -1;
}


unsigned int fac_load_word(fully_associative_cache* fac, void* addr)
{
    // Precompute start address of memory block
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;

    // select way to store in

    // first, check if any way already has the address
    find(fac, mb_start_addr);

    // if we can't find it, we must evict lru
    if (!found)
    {
        wayn = lru(fac);

        // if dirty, write back to mm
        if ((fac->valid[wayn]) && (fac->dirty[wayn]))
        {
            // copy dirty stuff to main memory
            mm_write(fac->mm, fac->ways[wayn]->start_addr, fac->ways[wayn]);

            // remove dirty tag
            fac->dirty[wayn] = 0;
        }

        fac->ways[wayn] = mm_read(fac->mm, mb_start_addr);

        fac->valid[wayn] = 1;
        fac->dirty[wayn] = 0;

        // increment read misses
        ++fac->cs.r_misses;
    }

     // Extract the word we care about
    unsigned int* mb_addr = fac->ways[wayn]->data + addr_offt;
    unsigned int result = *mb_addr;

    // update lru
    mark_as_used(fac, wayn);

    // increment read queries
    ++fac->cs.r_queries;

    // reset found
    found = 0;
    wayn = -1;

    // Return result
    return result;
}

void fac_free(fully_associative_cache* fac)
{
    free(fac);
}