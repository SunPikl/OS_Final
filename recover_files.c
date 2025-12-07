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

    // open super
    superblock_t sb;
    fread(&sb, sizeof(superblock_t), 1, fp);
    
    // offset
    uint32_t diroffset = sizeof(superblock_t); //offset
    uint32_t dataoffset = sizeof(superblock_t) + sizeof(struct direntry) * sb.total_direntries ; //offset
    fseek(fp, dataoffset, SEEK_SET); // set dir location
    
    // loop through blocks
	struct fileblock data;
	data.data = malloc(sb.bytes_per_block - 3);
	int currFile = 1;
	FILE * newFile;
	for(int block = 0; block <  sb.total_blocks; block ++){	
		// get block data 
		fread(&data.is_busy, 1, 1, fp); // is block busy		
		fread(data.data, sb.bytes_per_block - 3, 1, fp); // read the data bytes
		fread(&data.next_block, sizeof(uint16_t), 1, fp); // read next_block		
		
		//if data not free aka has data
		if(data.is_busy){
			//if start of file
			if(data.data[0] == 0xFF && data.data[1] == 0xD8){
				
				char str[28];
				sprintf(str, "recovered_file_%d.jpg", currFile);
				newFile = fopen(str, "wb+");
						
				printf("hi");
				fflush(stdout);
				
				while (1==1) {
					fseek(fp, sizeof(superblock_t) + sizeof(struct direntry) * sb.total_direntries + sb.bytes_per_block * data.next_block, SEEK_SET);
					
					// get block data 
					fread(&data.is_busy, 1, 1, fp); // is block busy		
					fread(data.data, sb.bytes_per_block - 3, 1, fp); // read the data bytes
					fread(&data.next_block, sizeof(uint16_t), 1, fp); // read next_block
					
					if(data.data[0] != 0xFF && data.data[1] != 0xD9){
						break;
					}
				}
				
				printf("life is good");
				fflush(stdout);
				
				fclose(newFile);
			}
			
			// printf("bye");
			// fflush(stdout);
			
		}
		
	}
    // if start 0xFF 0xD8
	//  set dir info
    //  loop through blocks until no next OR end in 0xFF 0xD9
	// 		store data to uncovered file
    //      if end, count num bytes, set dir size

    fclose(fp);
    return 0;
}
