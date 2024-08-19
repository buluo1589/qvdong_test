#include "mpu6050.h"

float dt = 0.005;
float Q_Acc = 0.95238;

float Acc_x, Acc_y, Acc_z, Groy_x, Groy_y, Groy_z;
char BUF[14];

float Roll_a, Pitch_a, Roll_g, Pitch_g, Yaw_g;

float Roll_temp = 0;
float Pitch_temp = 0;
float Yaw_temp = 0;

float Roll, Pitch, Yaw, G_Roll, G_Pitch, G_Yaw, Temp_mpu6050;

int MPU6050_Open()
{
    int fd;

    fd = open("/dev/mpu6050", O_RDWR);

    return fd;
}

void mpu6050_write(int fd, char reg, char value)
{
    char buf[2];
    buf[0] = reg;
    buf[1] = value;
    write(fd, buf, sizeof(buf));
}

void mpu6050_read(int fd)
{
    read(fd, BUF, sizeof(BUF));
}

void MPU6050_Init(int fd)
{ 
    mpu6050_write(fd, PWR_M1, 0x00);   // 关睡眠，改时钟
    mpu6050_write(fd, SMPL, 0x07);     // 输出频率设置
    mpu6050_write(fd, CONFIG, 0x04);   // 滤波
    mpu6050_write(fd, G_CONFIG, 0x18); // 角速度速率设置
    mpu6050_write(fd, A_CONFIG, 0x01); // 加速度设置
}

void Angle_Calcu(int fd)
{
    mpu6050_read(fd);

    Acc_x = (float)((signed short)((BUF[0] << 8) | BUF[1])) / LSB_A;
    Acc_y = (float)((signed short)((BUF[2] << 8) | BUF[3])) / LSB_A;
    Acc_z = (float)((signed short)((BUF[4] << 8) | BUF[5])) / LSB_A;

    Groy_x = (float)((signed short)((BUF[8] << 8) | BUF[9])) / LSB_G;
    Groy_y = (float)((signed short)((BUF[10] << 8) | BUF[11])) / LSB_G;
    Groy_z = (float)((signed short)((BUF[12] << 8) | BUF[13])) / LSB_G;
    // printf("A:%f  %f  %f G:%d  %d  %d  %d  %d  %d\n", Acc_x, Acc_y, Acc_z, BUF[9]<<8|BUF[8],BUF[9]<<8,BUF[10],BUF[11],BUF[12],BUF[13]);
    Roll_a = (float)atan2(Acc_x, Acc_z) * 180.0f / 3.14159f;
    Pitch_a = -(float)atan2(Acc_y, Acc_z) * 180.0f / 3.14159f;

    Roll_g = Roll_temp + Groy_x * dt;
    Pitch_g = Pitch_temp + Groy_y * dt;
    Yaw_g = Yaw_temp + Groy_z * dt;

    Roll_temp = Q_Acc * Roll_a + (1 - Q_Acc) * Roll_g;
    Pitch_temp = Q_Acc * Pitch_a + (1 - Q_Acc) * Pitch_g;
    Yaw_temp = Yaw_g;
}

void MPU6050_Get(int fd)
{
    Angle_Calcu(fd);
    Roll = Roll_temp;
    Pitch = Pitch_temp;
    Yaw = Yaw_temp;
    G_Roll = Roll_g;
    G_Pitch = Pitch;
    G_Yaw = Yaw_g;
    Temp_mpu6050 = 36.53 + ((float)((signed short)((BUF[6] << 8) | BUF[8]))) / 340.0;
}
