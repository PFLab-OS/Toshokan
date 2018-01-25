#pragma once

#include <stdio.h>
#include "elfhead.h"

typedef struct {
    const uint8_t* header;
    const Elf64_Ehdr* elf64_header;
    Elf64_Xword mem_size;
    Elf64_Xword align;
} Elf64Exec;

int elf64_parse(const void* elf_ptr, Elf64Exec* elf64_exec);
int elf64_deploy(const Elf64Exec* elf64_exec, FILE* img_file);
