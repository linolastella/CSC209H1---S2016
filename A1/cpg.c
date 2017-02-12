#include <stdio.h>
#include <stdlib.h>

//function declarations
int valid_chars_count (char *input, int size);
int valid_char (char c);
float expected_CG_pattern (int C, int G, int N);
int total_count (int C, int G);

int main (int argc, char *argv[]) {
	char * inputfile_name=argv[1]; 
	FILE * input_fp;
	char *input_buffer; //stores all bytes in the input file
	char *sequence; //char array holding the final cleaned DNA sequence
		
	int *valid_cpg_positions; //array of all positions in sequence
	int input_len, seq_len;
		
	int i, j;
	int r;

	//open the file
	if ( !(input_fp= fopen ( inputfile_name , "rb" )) ) {
		printf("Could not open file \"%s\" for reading input lines \n", inputfile_name);
		return (-1);
	}
		
	//compute total number of characters in the input file
	fseek (input_fp, 0, SEEK_END);
	input_len=ftell (input_fp);
	rewind (input_fp); 
	printf ("total bytes in file: %d\n", input_len);
		
	//allocate an array to hold these characters
	input_buffer = (char*) malloc (input_len+1);
		
	//read file content into the input buffer
	r = fread (input_buffer, 1, input_len,  input_fp);
	printf ("read characters %d\n", r);
	if (r != input_len) {
		printf("Reading error \n");
    	return (-1);
	}
	//add terminating character
	input_buffer [input_len] ='\0';

	//determine total number of valid DNA characters
	//and allocate array of chars to hold them
	seq_len = valid_chars_count (input_buffer, input_len);
	printf ("total characters %d total valid characters %d \n", input_len, seq_len);
	sequence = (char*) malloc (seq_len+1); 
		
	//transfer valid characters from raw buffer
	for (i=0, j=0; i < input_len; i++) {
		if (valid_char (input_buffer [i])) {
			sequence [j++] = input_buffer [i];
		}
	}
	sequence [seq_len] = '\0';
	
	//allocate int array for all the positions
	valid_cpg_positions = (int*) malloc (seq_len*sizeof(int));
	for (i=0; i<seq_len; i++) {
		valid_cpg_positions[i] = 0;
	}
	
	/* YOUR CpG ISLANDS DISCOVERY CODE HERE */
	
	
	return 0;
}

int valid_chars_count (char *input, int size) {
	int i, count = 0;
	for (i=0; i<size; i++) {
		if (valid_char (input [i]) )
			count++;
	}
	return count;
}

int valid_char (char c) {
	if (c == 'a' || c == 'c' || c == 'g' || c == 't' ) 
		return 1;
	return 0;
}

float expected_CG_pattern (int C, int G, int N) {
    return (C * G) / N;
}

int total_count (int C, int G) {
    return C * G;
}
