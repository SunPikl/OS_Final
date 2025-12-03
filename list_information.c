#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "qfs.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <disk image file>\n", argv[0]);
        return 1;
    }
    FILE *fp = fopen(argv[1], "rb");
    if (!fp) {
        perror("fopen");
        return 2;
    }

#ifdef DEBUG
    printf("Opened disk image: %s\n", argv[1]);
#endif

    //TODO
    
    //store superblock
    superblock_t sb;
    fread(&sb, sizeof(superblock_t), 1, fp);
    
    //list all info from block
    printf("Type: %d\n", sb.fs_type);
    printf("Total Blocks: %d\n", sb.total_blocks);
    printf("Available Blocks: %d\n", sb.available_blocks);
    printf("Bytes per Block: %d\n", sb.bytes_per_block);
    printf("Total Directory Entries: %d\n", sb.total_direntries);
    printf("Available Directory Entries: %d\n", sb.available_direntries);
    printf("Reserved: %hhn\n", sb.reserved);
    printf("Label: %s\n", sb.label);
    

    fclose(fp);
    return 0;
}
