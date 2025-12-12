#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "qfs.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <disk image file> <file to remove>\n", argv[0]);
        return 1;
    }

    FILE *fp = fopen(argv[1], "rb+");
    if (!fp) {
        perror("fopen");
        return 2;
    }

#ifdef DEBUG
    printf("Opened disk image: %s\n", argv[1]);
#endif

	// Store input info
	char * removeFile = argv[2];
	
    // Read superblock
    superblock_t sb;
    fread(&sb, sizeof(superblock_t), 1, fp);
    
    // Go to directories
	uint32_t offset = sizeof(superblock_t); // Offset
	int usedDir = sb.total_direntries - sb.available_direntries; // Total entries
	fseek(fp, offset, SEEK_SET); // Set dir location
	
	// Find file
	int start = -1;
	int evilDir = -1;
	struct direntry entry;
	for(int dir = 0; dir < usedDir; dir ++){
		// Store file entry
		fread(&entry, sizeof(entry), 1, fp);
        
        // If names are the same
        if(!strcmp(entry.filename, removeFile)){
			start = entry.starting_block;
			
			// Set to entry location
			fseek(fp, sizeof(superblock_t) + sizeof(struct direntry) * dir, SEEK_SET);
			evilDir = dir;
			break;
		} 
	}
	
	printf("we deleted directory %d", evilDir);
	
	// Shuffle remaining directories over
	struct direntry currEntry;
	int numBlocks = 0;
	for(int write = 1; write < (usedDir - evilDir); write++){
		// Store current directory entry temporarily
		fseek(fp, sizeof(superblock_t) + sizeof(struct direntry) * (write + evilDir), SEEK_SET);
		fread(&currEntry, sizeof(currEntry), 1, fp);
		
		// Rewrite
		fseek(fp, sizeof(superblock_t) + sizeof(struct direntry) * (write + evilDir - 1), SEEK_SET);
		fwrite(&currEntry, sizeof(struct direntry), 1, fp);
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
			numBlocks ++;
			// Read in file info 
			//fread(&data.is_busy, 1, 1, fp);
			data.is_busy = 1;
			fwrite(&data.is_busy, sizeof(data.is_busy), 1, fp);

			// Read the data bytes
			fread(data.data, sb.bytes_per_block - 3, 1, fp);

			// Read next_block
			fread(&data.next_block, sizeof(uint16_t), 1, fp);
			
			// Write to file
			currSize -= (sb.bytes_per_block - 3);
            fseek(fp, sizeof(superblock_t) + sizeof(struct direntry) * sb.total_direntries + sb.bytes_per_block * data.next_block, SEEK_SET);
		} while (currSize >= 0);
	}
	
	// Fix superblock
    superblock_t tempsb;
    tempsb.available_blocks = sb.available_blocks + numBlocks;
    tempsb.available_direntries = sb.available_direntries + 1;
    tempsb.bytes_per_block = sb.bytes_per_block;
    tempsb.fs_type = sb.fs_type;
    memcpy(tempsb.label, sb.label, sizeof(tempsb.label));
    memcpy(tempsb.reserved, sb.reserved, sizeof(tempsb.reserved));
    tempsb.total_blocks = sb.total_blocks;
    tempsb.total_direntries = sb.total_direntries;
    
    fseek(fp, 0, SEEK_SET);
    fwrite(&tempsb, sizeof(superblock_t), 1, fp);
    
    fclose(fp);
    return 0;
}
