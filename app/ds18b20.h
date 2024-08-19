#ifndef DS18B20_H
#define DS18B20_H

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define SET_RESOLUTION _IOW('A', 0, int)
#define READ_RESOLUTION _IOR('A', 1, int)

extern float Temp_ds18b20;

int DS18B20_Init(int fd, int resolution);
// int DS18B20_Get_Resolution(int fd);
int DS18B20_Open(void);
void DS18B20_Get(int fd);

#endif
