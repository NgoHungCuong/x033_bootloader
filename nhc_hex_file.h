#ifndef NHC_HEX_FILE_H_
#define NHC_HEX_FILE_H_

#include <stdint.h>

class nhc_hex_file
{
private:
    uint32_t mem_size;
    uint8_t seed;
    uint32_t base_address;
    uint8_t *p_buff;
    uint32_t tmp_address;
    uint32_t pos;
    uint32_t process_line(char *p_line);
    uint8_t str2num(char *p_str);
    
public:
    void set_mem_size(uint32_t mem_size);
    void set_seed(uint8_t seed);
    void set_base_address(uint32_t base_address);
    uint32_t init_buffer(void);
    void free_buffer(void);
    uint8_t *get_buffer(void);
    uint32_t read_file(char *p_file_name);
};

#endif
