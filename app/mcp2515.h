#ifndef MCP2515_H
#define MCP2515_H

#include <stdlib.h>
#include <math.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

#define MCP2515_STATUS 0x0e
#define MCP2515_CNF1 0x2a
#define MCP2515_CNF2 0x29
#define MCP2515_CNF3 0x28
#define MCP2515_CANINTE 0x2b
#define MCP2515_CANCTRL 0x0f
#define MCP2515_TXB0CTRL 0x30
#define MCP2515_CANINTF 0x2c
#define MCP2515_RXB0CTRL 0x60

int MCP2515_Open(void);
void MCP2515_Read(int fd, char *data);
void MCP2515_Write(int fd, char *data);

#endif