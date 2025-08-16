//
//  brook_files.c
//  http-c-broker
//
//  Created by David Xavier on 26/07/2025.
//

#include "brook_files.h"
#include "brook_core.h"

FILE*
brook_open_file(char* file_name, char* action) {
    FILE* fp;
    fp = fopen(file_name, action);
    return fp;
}

FILE*
brook_open_file_str(brook_str_t file_name, char* action) {
	
	char* tmp = calloc(1, file_name.len + 1);
	memcpy(tmp, file_name.data, file_name.len);
	
	FILE* fp;
	fp = fopen(tmp, action);
	free(tmp);
	
	return fp;
}

char*
brook_read_file(FILE* file) {

    fseek(file, 0, SEEK_END); // make file pointer to end of file
    size_t len = ftell(file); // get lenght of file
    fseek(file, 0, SEEK_SET); // back pointer to start of file

	char* content = calloc(1, len + 1); // store in heap my file content
    fread(content, 1, len, file); // read file to buffer

    return content;
}

void
brook_close_file (FILE* file) {
    fclose(file);
}

int
brook_get_files_from_dir (const char* dir) {
	
	DIR* FD;
	struct dirent* in_file;
	FILE* entry_file;
	FILE* file;
	
	char* data = NULL;
	
	if (NULL == (FD = opendir(dir))) {
		fprintf(stderr, "Error : Failed to open input directory (%s) - %s\n", dir, strerror(errno));
	}
	
	while ((in_file = readdir(FD))) {
		
		if (!strcmp (in_file->d_name, "."))
			continue;
		if (!strcmp (in_file->d_name, ".."))
			continue;
		
		file = brook_open_file(in_file->d_name, "r");
		char* file_content = brook_read_file(file);
		brook_close_file(file);

	}
	
	return BROOK_OK;
}
