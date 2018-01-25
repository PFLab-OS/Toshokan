#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include "elf_loader.h"

static const void* get_mmap_ptr(const char* file_name);

int main(int argc, char** argv)
{
    if (argc != 2 && argc != 3) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        return 1;
    }

    const void* elf_ptr = get_mmap_ptr(argv[1]);
    if (elf_ptr == NULL) {
        fputs("Failed to get mmap'ed file pointer\n", stderr);
        return 1;
    }

    const char* img_filename = argc == 3 ? argv[2] : "img.bin";
    FILE* img_file = fopen(img_filename, "w");

    Elf64Exec elf64_exec;
    if (elf64_parse(elf_ptr, &elf64_exec) == -1) {
        fputs("Failed to parse file: invalid ELF header.\n", stderr);
        return 1;
    }

    if (elf64_deploy(&elf64_exec, img_file) == -1) {
        fputs("Failed to deploy ELF to an image.\n", stderr);
        return 1;
    }

    return 0;
}

static const void* get_mmap_ptr(const char* file_name)
{
    int fd = open(file_name, O_RDONLY);
    if (fd == -1) {
        perror("open");
        return NULL;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("fstat");
        return NULL;
    }

    size_t file_size = (size_t)sb.st_size;

    const void* mmap_ptr = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mmap_ptr == MAP_FAILED) {
        perror("mmap");
        return NULL;
    }

    return mmap_ptr;
}
