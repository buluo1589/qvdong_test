#ifndef MPU6050_H
#define MPU6050_H

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MPU6050_Addr 0xD0
#define WHO 0x75
#define PWR_M1 0x6B
#define PWR_M2 0x6C
#define SMPL 0x19
#define CONFIG 0x1A
#define G_CONFIG 0x1B
#define A_CONFIG 0x1C
#define AX_H 0x3B
#define AX_L 0x3C
#define AY_H 0x3D
#define AY_L 0x3E
#define AZ_H 0x3F
#define AZ_L 0x40
#define TEMP_H 0x41
#define TEMP_L 0x42
#define GX_H 0x43
#define GX_L 0x44
#define GY_H 0x45
#define GY_L 0x46
#define GZ_H 0x47
#define GZ_L 0x48
#define LSB_G 16.4f
#define LSB_A 1

extern float Roll, Pitch, Yaw, G_Roll, G_Pitch, G_Yaw, Temp_mpu6050;

void MPU6050_Get(int fd);
void MPU6050_Init(int fd);
int MPU6050_Open();

#endif
