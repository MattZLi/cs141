#ifndef SET_ASSOCIATIVE_H
#define SET_ASSOCIATIVE_H

#include "main_memory.h"
#include "cache_stats.h"

#define SET_ASSOCIATIVE_NUM_SETS 8
#define SET_ASSOCIATIVE_NUM_SETS_LN 3
#define SET_ASSOCIATIVE_NUM_WAYS 2
#define SET_ASSOCIATIVE_NUM_WAYS_LN 1

typedef struct set_associative_cache
{
    main_memory* mm;
    cache_stats cs;

    // number of blocks = number of sets * number of ways
    memory_block* blks[SET_ASSOCIATIVE_NUM_SETS][SET_ASSOCIATIVE_NUM_WAYS];

    // if valid is 1, block holds meaningful data
    int valid[SET_ASSOCIATIVE_NUM_SETS][SET_ASSOCIATIVE_NUM_WAYS];

    // if dirty is 1, cache block doesnt match main memory block
    int dirty[SET_ASSOCIATIVE_NUM_SETS][SET_ASSOCIATIVE_NUM_WAYS];

    int use[SET_ASSOCIATIVE_NUM_SETS][SET_ASSOCIATIVE_NUM_WAYS];

} set_associative_cache;

// Do not edit below this line

set_associative_cache* sac_init(main_memory* mm);

void sac_store_word(set_associative_cache* sac, void* addr, unsigned int val);

unsigned int sac_load_word(set_associative_cache* sac, void* addr);

void sac_free(set_associative_cache* sac);

#endif