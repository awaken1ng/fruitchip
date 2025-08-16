#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <fletcher16.h>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s [file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE* fd = fopen(argv[1], "r");
    if (fd == NULL)
        exit(EXIT_FAILURE);

    if (fseek(fd, 0, SEEK_END) != 0)
        exit(EXIT_FAILURE);

    unsigned long file_size = ftell(fd);

    uint8_t *file_buffer = malloc(file_size);
    if (file_buffer == NULL)
        exit(EXIT_FAILURE);

    if (fseek(fd, 0, SEEK_SET) != 0)
        exit(EXIT_FAILURE);

    unsigned int read = fread(file_buffer, sizeof(uint8_t), file_size, fd);
    if (read != file_size)
        exit(EXIT_FAILURE);

    fclose(fd);

    uint16_t checksum = fletcher16(file_buffer, file_size);
    printf("0x%X\n", checksum);
}
