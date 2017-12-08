#include <stdint.h>

#include "memory_block.h"
#include "direct_mapped.h"

direct_mapped_cache* dmc_init(main_memory* mm)
{
    // TODO

    // same format as simple

    //allocate space
    direct_mapped_cache* result = malloc(sizeof(direct_mapped_cache));

    result->mm = mm;
    result->cs = cs_init();

    for (int i = 0; i < DIRECT_MAPPED_NUM_SETS; i++) {
        //making an array of size numsets
        result->set_array[i] = NULL;
        result->dirty[i] = 0;
    }

    return result;
}

// Optional

static int addr_to_set(void* addr)
{
	// function: given an address of 32 bytes, calculates index by looking at bytes x-y
	// returns this information as an int
	size_t addr_shift = (size_t) (addr - MAIN_MEMORY_START_ADDR) >> MAIN_MEMORY_BLOCK_SIZE_LN;

    return addr_shift;
}


void dmc_store_word(direct_mapped_cache* dmc, void* addr, unsigned int val)
{

    //dmc start
    // select set to store in
    int set_num = addr_to_set(addr);

    // Precompute start address of memory block
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;

    //


    // Load memory block from main memory
    memory_block* mb = mm_read(sc->mm, mb_start_addr);

    // Update relevant word in memory block
    unsigned int* mb_addr = dmc->set_array[set_num]-> + addr_offt;
    *mb_addr = val;

    // set as dirty
    dmc->dirty[set_num] = 1;

    // Store memory block back into main memory
    mm_write(sc->mm, mb_start_addr, mb);

    // Update statistics
    //++sc->cs.w_queries;
    //++sc->cs.w_misses;
    ++dmc->cs.w_queries;

    // Free memory block
    //mb_free(mb);

    // simple end
}

unsigned int dmc_load_word(direct_mapped_cache* dmc, void* addr)
{
    // TODO

    // simple start
    // Precompute start address of memory block
    size_t addr_offt = (size_t) (addr - MAIN_MEMORY_START_ADDR) % MAIN_MEMORY_BLOCK_SIZE;
    void* mb_start_addr = addr - addr_offt;

    // check if compulsory miss and must load in from main mem
    if dmc->set_array[set_num] == NULL {

        dmc->set_array[set_num] = mm_read(dmc->mm, mb_start_addr);

        // need to increment misses

    }

    // Load memory block from main memory
    memory_block* mb = mm_read(sc->mm, mb_start_addr);

    // Extract the word we care about
    unsigned int* mb_addr = mb->data + addr_offt;
    unsigned int result = *mb_addr;

    // Update statistics
    ++sc->cs.r_queries;
    ++sc->cs.r_misses;

    // Free memory block
    mb_free(mb);

    // Return result
    return result;

}

void dmc_free(direct_mapped_cache* dmc)
{
    // TODO

    free(dmc);


}