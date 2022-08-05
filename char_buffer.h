#ifndef CHAR_BUFFER_H
#define CHAR_BUFFER_H

#define CHAR_BUFFER_STARTING_SIZE 32

/// A struct representing an expanding buffer
struct char_buffer {
	char* data;
	int length;
	int max_length;
};

typedef struct char_buffer char_buffer;

/// Initializes a char_buffer and returns its location. Initial values are as follows:
///		data: a buffer with size of CHAR_BUFFER_STARTING_SIZE
///		length: the size of the data currently stored in the buffer
///		max_length: the size of the actual buffer; updated when the buffer is resized
/// @return the newly initialized char_buffer
char_buffer* make_char_buffer ();

/// Extends the given char_buffer by doubling its length. This is done by copying the old data over to a new, larger internal buffer.
/// @param the char_buffer to extend
void extend_buffer (char_buffer* buffer);

/// Adds a char to the given char_buffer. Calls extend_buffer if the buffer is too small.
/// @param buffer the char_buffer to add to
/// @param data char to add to the buffer
void char_buffer_add (char_buffer* buffer, char data);

/// Adds a given chunk of data to the given char_buffer. Calls extend_buffer if the buffer is too small, potentially multiple times.
/// @param buffer the char_buffer to add to
/// @param item the data to add
/// @param size the size of the data to add, in bytes
void char_buffer_add_item (char_buffer* buffer, void* item, int size);

/// Adds the given string to the given char_buffer. Calls extend_buffer if the buffer is too small, potentially multiple times.
/// @param buffer the char_buffer to add to
/// @param str the string to add
void char_buffer_add_str (char_buffer* buffer, char* str);

/// Makes a char_buffer and fills it with the contents of the file at the given filepath. The resulting data is null-terminated.
/// @param filename the name of the file to read from
/// @return the resulting char_buffer
char_buffer* char_buffer_from_file (char* filename);

/// Makes a char_buffer and fills it with the contents of stdin. The resulting data is null-terminated.
/// @return the resulting buffer
char_buffer* char_buffer_from_stdin ();

/// Prints the contents of the given char_buffer to stdout
/// @param the buffer to print
void print_buffer (char_buffer* buffer);

#endif