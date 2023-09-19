#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nhc_usb.h"
#include "nhc_hex_file.h"

#define APP_VER "230919"

uint32_t u32Base = 0x08000000;
uint32_t u32BaseHex = 0x00000000;
uint32_t u32BootSize = 12 * 1024;
uint32_t u32AppSize = 64 * 1024;
uint32_t u32BuffSize = 8 * 1024;
uint32_t u32EraseUnit = 4 * 1024;
uint32_t u32ReadUnit = 1;
uint32_t u32WriteUnit = 1;
uint32_t u32KeySize = 0;
uint32_t u32ChipSize = 64 * 1024;

uint32_t u8tou32(uint8_t* p)
{
	uint32_t u32Tmp;

	u32Tmp = p[3];
	u32Tmp <<= 8;
	u32Tmp += p[2];
	u32Tmp <<= 8;
	u32Tmp += p[1];
	u32Tmp <<= 8;
	u32Tmp += p[0];

	return u32Tmp;
}

void u32tou8(uint32_t u32In, uint8_t* p)
{
	p[0] = u32In;
	u32In >>= 8;
	p[1] = u32In;
	u32In >>= 8;
	p[2] = u32In;
	u32In >>= 8;
	p[3] = u32In;
}

uint32_t IsNotBlank(uint8_t* p, uint32_t u32Len);

nhc_usb ktUsb;
nhc_hex_file ktHex;

int main(int argc, char** argv)
{
	printf("Arduino X033 bootloader version %s\n", APP_VER);
	if (argc != 2)
	{
		printf("Usage: arduino_x033 flash_file.hex\n");
		return -1;
	}

	uint32_t u32Start;
	uint32_t u32Finish;

	uint8_t* pu8NeedWrite;
	uint8_t* pu8NeedErase;
	uint8_t* pu8NeedVerify;
	uint32_t u32NumOfWrite = 0;
	uint32_t u32NumOfErase = 0;
	uint32_t u32NumOfVerify = 0;

	//u32Start = GetTickCount();

	if (!ktUsb.open(0x4348, 0x0012)) {
		printf("Found no Arduino X033 board\n");
		return -1;
	}

	if (1)
	{
		uint8_t u8Tmp[64];
		u8Tmp[0] = 0x00;
		if (!ktUsb.write(u8Tmp))
		{
			printf("USB IO: ERROR\n");
			ktUsb.close();
			return -1;
		}
		if (!ktUsb.read(u8Tmp))
		{
			printf("USB IO: ERROR\n");
			ktUsb.close();
			return -1;
		}

		u32Base = u8tou32(u8Tmp + 32);
		u32BaseHex = u8tou32(u8Tmp + 36);
		u32BootSize = u8tou32(u8Tmp + 40);
		u32AppSize = u8tou32(u8Tmp + 44);
		u32BuffSize = u8tou32(u8Tmp + 48);
		u32EraseUnit = u8tou32(u8Tmp + 52);
		u32ReadUnit = u8tou32(u8Tmp + 56);
		u32WriteUnit = u8tou32(u8Tmp + 60);

		if (!strstr((char*)u8Tmp, "Arduino X033"))
		{
			printf("Wrong Arduino X033 board\n");
			ktUsb.close();
			return -1;
		}
	}

	if (1)
	{
		ktHex.set_mem_size(u32BootSize + u32AppSize);
		ktHex.set_base_address(u32BaseHex);
		ktHex.set_seed(0xff);
	}

	ktHex.init_buffer();
	if (!ktHex.read_file(argv[1]))
	{
		printf("Load flash file: %s: FAILED\n", argv[1]);
		ktHex.free_buffer();
		ktUsb.close();
		return -1;
	}	

    uint8_t *p_buff;

    p_buff = ktHex.get_buffer();

	if (1)
	{
		uint32_t i;
		uint32_t n;

		n = u32AppSize / u32EraseUnit;
		u32NumOfErase = 0;
		pu8NeedErase = new uint8_t[n];

		for (i = 0; i < n; ++i)
		{
			if (IsNotBlank(&p_buff[u32BootSize + i * u32EraseUnit], u32EraseUnit))
			{
				++u32NumOfErase;
				pu8NeedErase[i] = 1;
			}
			else
			{
				pu8NeedErase[i] = 0;
			}
		}
	}

	if (1)
	{
		uint8_t u8Tmp[64];
		uint32_t i;
		uint32_t u32Address;
		uint32_t n;

		printf("Erase:");

		n = u32AppSize / u32EraseUnit;

		for (i = 0; i < n; ++i)
		{
			if (pu8NeedErase[i])
			{
				u8Tmp[0] = 0x01;
				u32Address = i * u32EraseUnit + u32BootSize + u32Base;
				u8Tmp[1] = u32Address;
				u8Tmp[2] = u32Address >> 8;
				u8Tmp[3] = u32Address >> 16;
				u8Tmp[4] = u32Address >> 24;

				if (!ktUsb.write(u8Tmp))
				{
					printf("\nUSB IO: ERROR\n");
					ktUsb.close();
					ktHex.free_buffer();
					delete[] pu8NeedErase;
					return -1;
				}
				if (!ktUsb.read(u8Tmp))
				{
					printf("\nUSB IO: ERROR\n");
					ktUsb.close();
					ktHex.free_buffer();
					delete[] pu8NeedErase;
					return -1;
				}

				if (u8Tmp[0] != 0x01)
				{
					printf("\nErase: FAILED\n");
					ktUsb.close();
					ktHex.free_buffer();
					delete[] pu8NeedErase;
					return -1;
				}

				printf(".");
				fflush(stdout);
			}
		}
		delete[] pu8NeedErase;
		printf("\n");
		fflush(stdout);
	}

	if (1)
	{
		uint32_t i;
		uint32_t n;

		n = u32AppSize / u32WriteUnit;
		u32NumOfWrite = 0;
		pu8NeedWrite = new uint8_t[n];

		for (i = 0; i < n; ++i)
		{
			if (IsNotBlank(&p_buff[u32BootSize + i * u32WriteUnit], u32WriteUnit))
			{
				++u32NumOfWrite;
				pu8NeedWrite[i] = 1;
			}
			else
			{
				pu8NeedWrite[i] = 0;
			}
		}
	}

	if (1)
	{
		uint8_t u8Tmp[64];
		uint32_t i;
		uint32_t u32NumOfPage;
		uint32_t u32FlashSize;
		uint32_t u32FlashPageSize;

		printf("Write:");
		u32FlashSize = u32AppSize;
		u32FlashPageSize = u32WriteUnit;
		u32NumOfPage = u32FlashSize / u32FlashPageSize;

		for (i = 0; i < u32NumOfPage; ++i)
		{
			uint32_t u32Addess;
			if (pu8NeedWrite[i])
			{
				u8Tmp[0] = 0x06;
				u8Tmp[1] = (u32FlashPageSize >> 0);
				u8Tmp[2] = (u32FlashPageSize >> 8);
				u8Tmp[3] = (u32FlashPageSize >> 16);
				u8Tmp[4] = (u32FlashPageSize >> 24);

				if (!ktUsb.write(u8Tmp))
				{
					printf("\nUSB IO: ERROR\n");
					ktUsb.close();
					ktHex.free_buffer();
					delete[] pu8NeedWrite;
					return -1;
				}

				if (!ktUsb.write(&p_buff[u32BootSize + i * u32FlashPageSize], u32FlashPageSize))
				{
					printf("\nUSB IO: ERROR\n");
					ktUsb.close();
					ktHex.free_buffer();
					delete[] pu8NeedWrite;
					return -1;
				}
				u32Addess = i * u32FlashPageSize + u32BootSize + u32Base;
				u8Tmp[0] = 0x02;
				u8Tmp[1] = (u32Addess >> 0);
				u8Tmp[2] = (u32Addess >> 8);
				u8Tmp[3] = (u32Addess >> 16);
				u8Tmp[4] = (u32Addess >> 24);
				u8Tmp[5] = (u32FlashPageSize >> 0);
				u8Tmp[6] = (u32FlashPageSize >> 8);
				u8Tmp[7] = (u32FlashPageSize >> 16);
				u8Tmp[8] = (u32FlashPageSize >> 24);
				if (!ktUsb.write(u8Tmp))
				{
					printf("\nUSB IO: ERROR\n");
					ktUsb.close();
					ktHex.free_buffer();
					delete[] pu8NeedWrite;
					return -1;
				}
				if (!ktUsb.read(u8Tmp))
				{
					printf("\nUSB IO: ERROR\n");
					ktUsb.close();
					ktHex.free_buffer();
					delete[] pu8NeedWrite;
					return -1;
				}
				if (!u8Tmp[0])
				{
					printf("\nWrite: FAILED\n");
					ktUsb.close();
					ktHex.free_buffer();
					delete[] pu8NeedWrite;
					return -1;
				}
				printf(".");
				fflush(stdout);
			}
		}

		delete[] pu8NeedWrite;
		printf("\n");
		fflush(stdout);
	}

	if (1)
	{
		uint32_t i;
		uint32_t n;

		n = u32AppSize / u32ReadUnit;
		u32NumOfVerify = 0;
		pu8NeedVerify = new uint8_t[n];

		for (i = 0; i < n; ++i)
		{
			if (IsNotBlank(&p_buff[u32BootSize + i * u32ReadUnit], u32ReadUnit))
			{
				++u32NumOfVerify;
				pu8NeedVerify[i] = 1;
			}
			else
			{
				pu8NeedVerify[i] = 0;
			}
		}
	}

	if (1)
	{
		uint8_t u8Tmp[64];
		uint32_t i;
		uint32_t u32NumOfPage;
		uint32_t u32FlashSize;
		uint32_t u32FlashPageSize;

		printf("Verify:");
		u32FlashSize = u32AppSize;
		u32FlashPageSize = u32ReadUnit;
		u32NumOfPage = u32FlashSize / u32FlashPageSize;

		for (i = 0; i < u32NumOfPage; ++i)
		{
			uint32_t u32Addess;
			if (pu8NeedVerify[i])
			{
				u8Tmp[0] = 0x06;
				u8Tmp[1] = (u32ReadUnit >> 0);
				u8Tmp[2] = (u32ReadUnit >> 8);
				u8Tmp[3] = (u32ReadUnit >> 16);
				u8Tmp[4] = (u32ReadUnit >> 24);
				if (!ktUsb.write(u8Tmp))
				{
					printf("\nUSB IO: ERROR\n");
					ktUsb.close();
					ktHex.free_buffer();
					delete[] pu8NeedVerify;
					return -1;
				}

				if (!ktUsb.write(&p_buff[u32BootSize + i * u32FlashPageSize], u32FlashPageSize))
				{
					printf("\nUSB IO: ERROR\n");
					ktUsb.close();
					ktHex.free_buffer();
					delete[] pu8NeedVerify;
					return -1;
				}

				u32Addess = i * u32FlashPageSize + u32BootSize + u32Base;
				u8Tmp[0] = 0x04;
				u8Tmp[1] = (u32Addess >> 0);
				u8Tmp[2] = (u32Addess >> 8);
				u8Tmp[3] = (u32Addess >> 16);
				u8Tmp[4] = (u32Addess >> 24);
				u8Tmp[5] = (u32FlashPageSize >> 0);
				u8Tmp[6] = (u32FlashPageSize >> 8);
				u8Tmp[7] = (u32FlashPageSize >> 16);
				u8Tmp[8] = (u32FlashPageSize >> 24);
				if (!ktUsb.write(u8Tmp))
				{
					printf("\nUSB IO: ERROR\n");
					ktUsb.close();
					ktHex.free_buffer();
					delete[] pu8NeedVerify;
					return -1;
				}

				if (!ktUsb.read(u8Tmp))
				{
					printf("\nUSB IO: ERROR\n");
					ktUsb.close();
					ktHex.free_buffer();
					delete[] pu8NeedVerify;
					return -1;
				}

				if (!u8Tmp[0])
				{
					printf("\nVerify: FAILED\n");
					ktUsb.close();
					ktHex.free_buffer();
					delete[] pu8NeedVerify;
					return -1;
				}
				printf(".");
				fflush(stdout);
			}
		}
		delete[] pu8NeedVerify;
		printf("\n");
		fflush(stdout);
	}

	if (1)
	{
		//reset to app
		uint8_t u8Tmp[64];

		u8Tmp[0] = 0x08;
		ktUsb.write(u8Tmp);
	}

	ktUsb.close();
	ktHex.free_buffer();

	//u32Finish = GetTickCount();
	//u32Finish -= u32Start;

	//printf("Success: %dms\n", u32Finish);
    printf("Success\n");

	return 0;
}

uint32_t IsNotBlank(uint8_t *p, uint32_t u32Len)
{
	uint32_t i;

	for (i = 0; i < u32Len; ++i)
	{
		if (p[i] != 0xff)
		{
			return 1;
		}
	}

	return 0;
}
