#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include <fletcher16.h>
#include <string.h>

#define CHECK_SECTION_SIZE 4

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        printf("Usage: %s [file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    FILE* fd = fopen(argv[1], "r+");
    if (fd == NULL)
        exit(EXIT_FAILURE);

    if (fseek(fd, 0, SEEK_END) != 0)
        exit(EXIT_FAILURE);

    unsigned long file_size = ftell(fd);
    uint16_t file_size_u16 = (uint16_t)file_size - CHECK_SECTION_SIZE;
    if (file_size > 0x62C)
    {
        printf("Error: stage 1 payload is too big");
        exit(EXIT_FAILURE);
    }

    uint8_t *file_buffer = malloc(file_size);
    if (file_buffer == NULL)
        exit(EXIT_FAILURE);

    if (fseek(fd, 0, SEEK_SET) != 0)
        exit(EXIT_FAILURE);

    unsigned int read = fread(file_buffer, sizeof(uint8_t), file_size_u16, fd);
    if (read != file_size_u16)
        exit(EXIT_FAILURE);

    // embed file size and checksum at the end, sections `.size` and `.checksum` in linker file
    uint16_t checksum = fletcher16(file_buffer, file_size_u16);

    if (fwrite(&file_size_u16, sizeof(file_size_u16), 1, fd) != 1)
        exit(EXIT_FAILURE);

    if (fwrite(&checksum, sizeof(checksum), 1, fd) != 1)
        exit(EXIT_FAILURE);

    fclose(fd);
}