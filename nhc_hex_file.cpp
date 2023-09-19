#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "nhc_hex_file.h"

static uint32_t is_hex(char c);

void nhc_hex_file::set_mem_size(uint32_t mem_size)
{

    this->mem_size = mem_size;
}

void nhc_hex_file::set_seed(uint8_t seed)
{

    this->seed = seed;
}

void nhc_hex_file::set_base_address(uint32_t base_address)
{

    this->base_address = base_address;
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
    FILE *f;

    f = fopen(p_file_name, "rt");
    
    if (!f)
    {
        return 0;
    }
    
    char *c;
    char szLine[200];
    
    pos = 0;
    tmp_address = 0;

    while (1)
    {
        c = fgets(szLine, 150, f);
        if (!c)
        {
            break;
        }
        
        if (!strncmp(szLine, ":00000001FF", 11))
        {
            break;
        }

        if (!process_line(szLine))
        {
            fclose(f);
            return 0;
        }
    }
    fclose(f);
    
    return 1;
}

uint8_t nhc_hex_file::str2num(char *p_str)
{
    uint8_t ret = 0;
    uint8_t t;

    if (p_str[0] > '9')
    {
        t = p_str[0] - 'A' + 10;
    }
    else
    {
        t = p_str[0] - '0';
    }

    ret = t << 4;

    if (p_str[1] > '9')
    {
        t = p_str[1] - 'A' + 10;
    }
    else
    {
        t = p_str[1] - '0';
    }

    ret |= t;

    return ret;
}

uint32_t is_hex(char c)
{
    if (c < '0')
    {
        return 0;
    }

    if (c > 'F')
    {
        return 0;
    }

    if ((c > '9') && (c < 'A'))
    {
        return 0;
    }

    return 1;
}

uint32_t nhc_hex_file::process_line(char *p_line)
{
    uint32_t n;
    uint32_t i;

    n = strlen(p_line);

    if (p_line[0] != ':')
    {
        return 0;
    }

    for (i = 1; i < n; ++i)
    {
        if (is_hex(p_line[i]) == 0)
        {
            n = i;
            break;
        }
    }

    uint32_t dwLo, dwHi;

    uint8_t rec_type;

    rec_type = str2num(&p_line[7]);

    if (rec_type)
    {
        if (rec_type == 0x04)
        {
            dwHi = str2num(&p_line[9]);
            dwLo = str2num(&p_line[11]);
            tmp_address = (dwHi * 0x100 + dwLo) * 0x10000;
        }
        return 1;
    }

    n = str2num(&p_line[1]);

    uint32_t dwLowAdd;
    uint32_t dwHiAdd;

    dwLowAdd = str2num(&p_line[5]);
    dwHiAdd = str2num(&p_line[3]);
    pos = tmp_address + dwHiAdd * 0x100;
    pos |= dwLowAdd;

    if (pos >= base_address)
    {
        pos -= base_address;
    }
    
    if (pos >= mem_size)
    {
        return 0;
    }

    int iPos;
    iPos = 9;

    for (i = 0; i < n; ++i)
    {
        p_buff[pos] = str2num(&p_line[iPos]);
        iPos += 2;
        ++pos;
        if (pos > mem_size)
        {
            return 0;
        }
    }

    return 1;
}
