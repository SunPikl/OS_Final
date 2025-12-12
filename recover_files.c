/**
 * Recovers files from a QFS disk image
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

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <filesystem_image>\n", argv[0]);
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

    // Open super
    superblock_t sb;
    fread(&sb, sizeof(superblock_t), 1, fp);
    
    // Offset
    uint32_t diroffset = sizeof(superblock_t); // Offset
    uint32_t dataoffset = sizeof(superblock_t) + sizeof(struct direntry) * sb.total_direntries ; //offset
    fseek(fp, dataoffset, SEEK_SET); // Set dir location
    
    // Loop through blocks
	struct fileblock data;
	data.data = malloc(sb.bytes_per_block - 3);
	int currFile = 1;
	int x = 0;
	FILE * newFile;
	for(int block = 0; block <  sb.total_blocks; block ++){	
		fseek(fp, sizeof(superblock_t) + sizeof(struct direntry) * sb.total_direntries + sb.bytes_per_block * block, SEEK_SET);
		// Get block data 
		fread(&data.is_busy, 1, 1, fp); // Is block busy		
		fread(data.data, sb.bytes_per_block - 3, 1, fp); // Read the data bytes
		fread(&data.next_block, sizeof(uint16_t), 1, fp); // Read next_block		
		
			// If start of file
			if(data.data[0] == 0xFF && data.data[1] == 0xD8){
				
				char str[28];
				sprintf(str, "recovered_file_%d.jpg", currFile);
				newFile = fopen(str, "wb+");
						
				// Load in data
				fwrite(data.data, sb.bytes_per_block - 3, 1, newFile);
				
				while (x == 0) {
					fseek(fp, sizeof(superblock_t) + sizeof(struct direntry) * sb.total_direntries + sb.bytes_per_block * data.next_block, SEEK_SET);
					
					// Get block data 
					fread(&data.is_busy, 1, 1, fp); // Is block busy		
					fread(data.data, sb.bytes_per_block - 3, 1, fp); // Read the data bytes
					fread(&data.next_block, sizeof(uint16_t), 1, fp); // Read next_block
					
					// Load in data
					fwrite(data.data, sb.bytes_per_block - 3, 1, newFile);
					
					if(!data.next_block) {
						break;
					}

				}
				
				fclose(newFile);
				currFile++;
			}		
		
	}

    fclose(fp);
    return 0;
}
