SOURCES = main.c ds18b20.c mpu6050.c mcp2515.c

OBJECTS = $(SOURCES:.c=.o)

CC = g++

main: $(OBJECTS)
	$(CC) $(CFLAGS) $^ -o $@
	rm -f $(OBJECTS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) main