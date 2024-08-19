#include "mcp2515.h"

int MCP2515_Open(void)
{
    int fd;
    fd = open("/dev/mcp2515", O_RDWR);
    return fd;
}

void MCP2515_Read(int fd, char *data)
{
    read(fd, data, 13);
}

void MCP2515_Write(int fd, char *data)
{
    char buf[13] = {0x66, 0x08, 0x22, 0x33, 0x08, 0};
    int i;
    for (i = 0; i < 8; i++)
    {
        buf[i + 5] = data[i];
    }
    write(fd, buf, sizeof(buf));
}
