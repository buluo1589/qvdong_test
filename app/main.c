#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include "mpu6050.h"
#include "mcp2515.h"
#include "ds18b20.h"

float roll_mcp2151, pitch_mcp2515, yaw_mcp2515, temp_mpu6050_mcp2515, temp_ds18b20_mcp2515;

int kbhit(void)
{
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);
    if (ch != EOF)
    {
        ungetc(ch, stdin);
        return 1;
    }
    return 0;
}

void float2char(char *data, float *floatdata)
{
    char farray[4];
    *(float *)farray = *floatdata;
    data[0] = farray[0];
    data[1] = farray[1];
    data[2] = farray[2];
    data[3] = farray[3];
}

float char2float(char *data)
{
    float f = 0;
    memcpy(&f, data, 4);
    return f;
}

int main()
{
    char key;
    char read_data[13] = {0};
    char write_data[8] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08};
    char data[4] = {0};
    int mpu6050_fd;
    int ds18b20_fd;
    int mcp2515_fd;

    mpu6050_fd = MPU6050_Open();
    if (mpu6050_fd < 0)
    {
        printf("open error\n");
        return -1;
    }

    ds18b20_fd = DS18B20_Open();
    if (ds18b20_fd < 0)
    {
        printf("open error\n");
        return -1;
    }

    mcp2515_fd = MCP2515_Open();
    if (mcp2515_fd < 0)
    {
        printf("open error\n");
        return -1;
    }

    MPU6050_Init(mpu6050_fd);
    while (1)
    {
        if (kbhit())
        {
            key = getchar();
            if (key == 'q' || key == 'Q')
            {
                break;
            }
        }

        MPU6050_Get(mpu6050_fd);
        DS18B20_Get(ds18b20_fd);

        printf("mpu6050:   %f   %f    %f   %f\n", Roll, Pitch, Yaw, Temp_mpu6050);

        float2char(data, &Roll);
        write_data[0] = data[0];
        write_data[1] = data[1];
        write_data[2] = data[2];
        write_data[3] = data[3];
        float2char(data, &Pitch);
        write_data[4] = data[0];
        write_data[5] = data[1];
        write_data[6] = data[2];
        write_data[7] = data[3];
        MCP2515_Write(mcp2515_fd, write_data);

        MCP2515_Read(mcp2515_fd, read_data);
        data[0] = read_data[5];
        data[1] = read_data[6];
        data[2] = read_data[7];
        data[3] = read_data[8];
        roll_mcp2151 = char2float(data);
        data[0] = read_data[9];
        data[1] = read_data[10];
        data[2] = read_data[11];
        data[3] = read_data[12];
        pitch_mcp2515 = char2float(data);

        float2char(data, &Yaw);
        write_data[0] = data[0];
        write_data[1] = data[1];
        write_data[2] = data[2];
        write_data[3] = data[3];
        float2char(data, &Temp_mpu6050);
        write_data[4] = data[0];
        write_data[5] = data[1];
        write_data[6] = data[2];
        write_data[7] = data[3];
        MCP2515_Write(mcp2515_fd, write_data);

        MCP2515_Read(mcp2515_fd, read_data);
        data[0] = read_data[5];
        data[1] = read_data[6];
        data[2] = read_data[7];
        data[3] = read_data[8];
        yaw_mcp2515 = char2float(data);
        data[0] = read_data[9];
        data[1] = read_data[10];
        data[2] = read_data[11];
        data[3] = read_data[12];
        temp_mpu6050_mcp2515 = char2float(data);

        float2char(data, &Temp_ds18b20);
        write_data[0] = data[0];
        write_data[1] = data[1];
        write_data[2] = data[2];
        write_data[3] = data[3];
        float2char(data, &Temp_mpu6050);
        write_data[4] = data[0];
        write_data[5] = data[1];
        write_data[6] = data[2];
        write_data[7] = data[3];
        MCP2515_Write(mcp2515_fd, write_data);

        MCP2515_Read(mcp2515_fd, read_data);
        data[0] = read_data[5];
        data[1] = read_data[6];
        data[2] = read_data[7];
        data[3] = read_data[8];
        temp_ds18b20_mcp2515 = char2float(data);
        data[0] = read_data[9];
        data[1] = read_data[10];
        data[2] = read_data[11];
        data[3] = read_data[12];
        temp_mpu6050_mcp2515 = char2float(data);

        printf("mcp2515:   %f   %f    %f   %f\n", roll_mcp2151, pitch_mcp2515, yaw_mcp2515, temp_mpu6050_mcp2515);
        printf("ds18b20: %f    mpu6050: %f\n\n", temp_ds18b20_mcp2515, temp_mpu6050_mcp2515);
    }

    close(mpu6050_fd);
    close(ds18b20_fd);
    close(mcp2515_fd);
    return 0;
}
// g++ main.c mpu6050.c ds18b20.c mcp2515.c -o main
