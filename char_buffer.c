#include <stdio.h>
#include <stdlib.h>

#include "char_buffer.h"

// This file is from one of my old projects, so the naming conventions are different. All visible function names have been updated though.

char_buffer* make_char_buffer () {
	char_buffer* buffer;
    buffer = malloc (sizeof (char_buffer));
	buffer->data = malloc (sizeof(char) * CHAR_BUFFER_STARTING_SIZE);
	buffer->length = 0;
	buffer->max_length = sizeof(char) * CHAR_BUFFER_STARTING_SIZE;
	return buffer;
}

void extend_buffer (char_buffer* buffer) {
	
	//Allocate the new buffer
	char* oldData = buffer->data;
	buffer->data = malloc (buffer->max_length * 2);
	if (!buffer->data) {
		printf ("Error: memory allocation failure while extending buffer");
		exit (1);
	}
	
	//Copy the data over to the new buffer
	int pos;
	for (pos = 0; pos < buffer->length; pos ++) {
		buffer->data[pos] = oldData[pos];
	}
	buffer->max_length = buffer->max_length * 2;
	
	//Free the old buffer
	free (oldData);
}

void char_buffer_add (char_buffer* buffer, char data) {
	if (buffer->length > buffer->max_length - 1) {
		extend_buffer (buffer);
	}
	buffer->data[buffer->length] = data;
	buffer->length += sizeof(char);
}

void char_buffer_add_item (char_buffer* buffer, void* item, int size) {
	int i;
	for (i = 0; i < size; i++) {
		char_buffer_add (buffer, ((char*)(item))[i]);
	}
}

void char_buffer_add_str (char_buffer* buffer, char* str) {
	int i = 0;
	while (str[i]) {
		char_buffer_add (buffer, str[i]);
		i++;
	}
}

char_buffer* char_buffer_from_file (char* filename) {
	
	//Open the file and create the buffer
	char_buffer* filedata = make_char_buffer ();
	FILE* infile = fopen (filename, "r");
	int c;
	
	//Read the contents of the file to the buffer
	while (((c = fgetc (infile)) != EOF)) {
		char_buffer_add (filedata, c);
	}
	char_buffer_add (filedata, 0);
	return filedata;
}

char_buffer* char_buffer_from_stdin () {
	char_buffer* data = make_char_buffer ();
	int c;
	while (((c = fgetc (stdin)) != EOF)) {
		char_buffer_add (data, c);
	}
	char_buffer_add (data, 0);
	return data;
}

void print_buffer (char_buffer* buffer) {
	int i;
	for (i = 0; i < buffer->length; i ++) {
		printf ("%c", buffer->data[i]);
	}
	printf ("\n");
}
