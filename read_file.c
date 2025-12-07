#include <stdio.h>
#include <stdint.h>
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

	// store input info
	char * extractFile = argv[2];
	FILE * storeFile = fopen(argv[3], "rb+");
	

    // Read superblock
    superblock_t sb;
    fread(&sb, sizeof(superblock_t), 1, fp);
    
    // Go to directories
	uint32_t offset = sizeof(superblock_t); //offset
	int usedDir = sb.total_direntries - sb.available_direntries; //total entries
	fseek(fp, offset, SEEK_SET); // set dir location
	
	//find file
	int start = -1;
	struct direntry entry;
	for(int dir = 0; dir < usedDir; dir ++){
		//store file entry
		fread(&entry, sizeof(entry), 1, fp);
        
        //if names are the same
        if(!strcmp(entry.filename, extractFile)){
			start = entry.starting_block;
			break;
		} 
	}
	
	printf("DEBUG 1 %s %d", entry.filename, start);
	fflush(stdout);
	
	//check if file found
	if(start == -1){
		printf("ERROR: File doesn't exist");
	} else {
		//find offset and start location
		uint32_t diroffset = sizeof(superblock_t) + sizeof(struct direntry) * sb.total_direntries + sizeof(struct fileblock) * start; //offset
		fseek(fp, diroffset, SEEK_SET); // set dir location
		
		//loop
		struct fileblock data;
		do {
			//read in file info 
			fread(&data, sizeof(data), 1, fp);
			
			//write to file
			fwrite(data.data, sizeof(char), (entry.file_size), storeFile);
		} while (data.next_block == NULL);
	}
	

    fclose(fp);
    fclose(storeFile);
    return 0;
}
