/* CS360 Lab 4: C */
#include <stdio.h>
#include <stdlib.h>

#define MIN_FILE_SIZE 1
#define MAX_FILE_SIZE 10485760
#define ONE_MEG 1048576

FILE* fp;

struct MP3Data 
{
	long fileSize;
	unsigned char* data;
};

int initialize(int argc, char* argv[])
{
	// Open the file given on the command line
	if (argc != 2)
	{
		printf("Usage: %s filename.mp3\n", argv[0]);
		return(EXIT_FAILURE);
	}

	fp = fopen(argv[1], "rb");
	if (fp == NULL)
	{
		printf("Can't open file %s\n", argv[1]);
		return(EXIT_FAILURE);
	}
	return (EXIT_SUCCESS);
}

int readFile(struct MP3Data* mp3Data)
{
	// How many bytes are there in the file?  If you know the OS you're
	// on you can use a system API call to find out.  Here we use ANSI standard
	// function calls.
	long size = 0;
	fseek(fp, 0, SEEK_END);		// go to 0 bytes from the end
	size = ftell(fp);				// how far from the beginning?
	rewind(fp);						// go back to the beginning

	if (size < MIN_FILE_SIZE || size > MAX_FILE_SIZE)
	{
		printf("File size is not within the allowed range\n");
		return(EXIT_FAILURE);
	}

	mp3Data->fileSize = size;

	printf("File size: %.2f MB\n", (float)size / ONE_MEG);
	// Allocate memory on the heap for a copy of the file
	mp3Data->data = (unsigned char*)malloc(size);
	// Read it into our block of memory
	size_t bytesRead = fread(mp3Data->data, sizeof(unsigned char), size, fp);
	if (bytesRead != size)
	{
		printf("Error reading file. Unexpected number of bytes read: %I64d\n", bytesRead);
		free(mp3Data->data);
		return(EXIT_FAILURE);
	}
	return(EXIT_SUCCESS);
}

void printbinchar(unsigned char character)
{
	char output[9];
	itoa(character, output, 16);
	printf("%s\n\n", output);
}

void searchForSyncBytes(struct MP3Data* mp3Data)
{
	for (long i = 0; i < mp3Data->fileSize; i++)
	{
		if (mp3Data->data[i] == (unsigned char)0xFF && (mp3Data->data[i + 1] & 0xF0)  == (unsigned char)0xF0)
		{
			printf("sync bytes at offset: ");
            printf("byte # : %ld\n", i);
            printf("found byte0\n");
            printbinchar(mp3Data->data[i]);
			printf("found byte1\n");
            printbinchar(mp3Data->data[i + 1]);
			printf("found byte2\n");
            printbinchar(mp3Data->data[i + 2]);
			printf("found byte3\n");
            printbinchar(mp3Data->data[i + 3]);
            break;
		}
	}
}

int main(int argc, char** argv)
{
	if (initialize(argc, argv) == EXIT_FAILURE)
	{
		fclose(fp);				// close and free the file
		exit(EXIT_FAILURE);		// return 0;
	}

	struct MP3Data mp3Data;
	mp3Data.data = NULL;
	if (readFile(&mp3Data) == EXIT_FAILURE)
	{
		fclose(fp);				// close and free the file
		exit(EXIT_FAILURE);		// return 0;
	}

	searchForSyncBytes(&mp3Data);

	exit(EXIT_SUCCESS);		// return 1;
}