/**
 * Lists the information of a QFS disk file
 * 
 * Authors: Morgan Montz & Matthew Jones
 * 
 **/


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
    printf("Block Size: %d\n", sb.bytes_per_block);
    printf("Total Blocks: %d\n", sb.total_blocks);
    printf("Amount of Free Blocks: %d\n", sb.available_blocks);
    printf("Total Directory Entries: %d\n", sb.total_direntries);
    printf("Amount of Free Directory Entries: %d\n", sb.available_direntries);
    printf("\n-----------------------------\n");
	
	//offset
	uint32_t offset = sizeof(superblock_t);
	
	//tot entries 
	int usedDir = sb.total_direntries - sb.available_direntries;
	
	//set location
	fseek(fp, offset, SEEK_SET);
	
	//all files in directory
	for(int dir = 0; dir < usedDir; dir ++){
		struct direntry entry;
		fread(&entry, sizeof(entry), 1, fp);
        
        //print info
        printf("File Name: %s\n", entry.filename);
        printf("File Size: %d\n", entry.file_size);
        printf("Start Block #: %d\n\n", entry.starting_block);
	}

    fclose(fp);
    return 0;
}
