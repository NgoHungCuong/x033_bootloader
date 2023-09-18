#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nhc_hex_file.h"

void nhc_hex_file::set_mem_size(uint32_t mem_size)
{

    this->mem_size = mem_size;
}

void nhc_hex_file::set_seed(uint8_t seed)
{

    this->seed = seed;
}

uint32_t nhc_hex_file::init_buffer(void)
{
    
    p_buff = (unsigned char *)malloc(mem_size);
    if (!p_buff)
    {
        return 0;
    }

    memset(p_buff, seed, mem_size);

    return 1;
}

void nhc_hex_file::free_buffer(void)
{

    free(p_buff);
}

uint8_t *nhc_hex_file::get_buffer(void)
{

    return p_buff;
}

uint32_t nhc_hex_file::read_file(char *p_file_name)
{

    return 0;
}
