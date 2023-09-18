#ifndef NHC_USB_H_
#define NHC_USB_H_

#include <stdint.h>

class nhc_usb
{
public:
    uint32_t open(uint32_t vid, uint32_t pid);
    void close(void);
    uint32_t write(uint8_t *p_data, uint32_t len = 64, uint8_t ep = 1);
    uint32_t read(uint8_t *p_data, uint32_t len = 64, uint8_t ep = 1);
};

#endif
