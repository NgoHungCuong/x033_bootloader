#include <stdint.h>
#include <stdio.h>
#include "nhc_usb.h"
#include "nhc_hex_file.h"

nhc_usb usb;

int main(int argc, char **argv)
{

    if (!usb.open(0x4348, 0x0012))
    {
        printf("Khong tim thay USB\n");
    }
    else
    {
        printf("Tim thay USB\n");
        uint8_t tmp[64];
        tmp[0] = 0x00;
        if (usb.write(tmp))
        {
            if (usb.read(tmp))
            {
                printf("%s\n", (char *)tmp);
            }
            else
            {
                printf("Read: FAIL\n");
            }
        }
        else
        {
            printf("Write: FAIL\n");
        }
    }

    return 0;
}
