#include "ds18b20.h"

float Temp_ds18b20;

int DS18B20_Open(void)
{
    int fd;
    fd = open("/dev/ds18b20", O_RDWR);
    return fd;
}

int DS18B20_Init(int fd, int resolution)
{
    ioctl(fd, SET_RESOLUTION, resolution);
}

// int DS18B20_Get_Resolution(int fd)
// {
//     int data;
//     ioctl(fd, READ_RESOLUTION, &data);
//     return data;
// }

void DS18B20_Get(int fd)
{
    int data;
    read(fd, &data, sizeof(data));
    if ((data >> 11) & 0x01)
    {
        data = -data + 1;
        data &= ~(0xf8 << 8);
        Temp_ds18b20 = -data * 0.0625;
    }
    else
    {
        Temp_ds18b20 = data * 0.0625;
    }
}
