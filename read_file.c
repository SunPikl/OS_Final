/**
 * Reads in a file from QFS and outputs it into an image file
 * 
 * Authors: Morgan Montz & Matthew Jones
 * 
 **/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "qfs.h"

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <disk image file> <file to read> <output file>\n", argv[0]);
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

	// Store input info
	char * extractFile = argv[2];
	FILE * storeFile = fopen(argv[3], "wb+");
	
    // Read superblock
    superblock_t sb;
    fread(&sb, sizeof(superblock_t), 1, fp);
    
    // Go to directories
	uint32_t offset = sizeof(superblock_t); // Offset
	int usedDir = sb.total_direntries - sb.available_direntries; // Total entries
	fseek(fp, offset, SEEK_SET); // Set dir location
	
	// Find file
	int start = -1;
	struct direntry entry;
	for(int dir = 0; dir < usedDir; dir ++){
		// Store file entry
		fread(&entry, sizeof(entry), 1, fp);
        
        // If names are the same
        if(!strcmp(entry.filename, extractFile)){
			start = entry.starting_block;
			break;
		} 
	}
	
	// Check if the file found
	if(start == -1){
		printf("ERROR: File doesn't exist");
	} else {
		// Find offset and start location
		uint32_t diroffset = sizeof(superblock_t) + sizeof(struct direntry) * sb.total_direntries + sb.bytes_per_block * start; //offset
		fseek(fp, diroffset, SEEK_SET); // Set dir location
		
		struct fileblock data;
		data.data = malloc(sb.bytes_per_block - 3);
		int currSize = entry.file_size;
		
		do {			
			// Read in file info 
			fread(&data.is_busy, 1, 1, fp);

			// Read the data bytes
			fread(data.data, sb.bytes_per_block - 3, 1, fp);

			// Read next_block
			fread(&data.next_block, sizeof(uint16_t), 1, fp);
			
			// Write to file
			fwrite(data.data, sb.bytes_per_block - 3, 1, storeFile);
			currSize -= (sb.bytes_per_block - 3);
            fseek(fp, sizeof(superblock_t) + sizeof(struct direntry) * sb.total_direntries + sb.bytes_per_block * data.next_block, SEEK_SET);
		} while (currSize >= 0);
	}

    fclose(fp);
    fclose(storeFile);
    return 0;
}
