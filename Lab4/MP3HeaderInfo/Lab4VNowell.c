/* CS360 Lab 4: C - Vince Nowell 11/3/2021 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MIN_FILE_SIZE 1
#define MAX_FILE_SIZE 10485760
#define ONE_MEG 1048576

const int bitRates[] = { 0,32,40,48,56,64,80,96,112,128,160,192,224,256,320 };
const int sampleRates[] = { 44100,48000,32000 };

FILE* fp;

struct MP3Data
{
	char* fileName;
	long fileSize;
	long headerOffset;
	int isMPEG1;
	int isLayer3;
	int bitRate;
	int sampleRate;
	int hasErrorProtection;
	int hasCopyright;
	int isOriginal;
	int alert;
	unsigned char* data;
};

int initialize(int argc, char* argv[], struct MP3Data* mp3Data)
{
	// Open the file given on the command line, 
	// if a filename was passed in, check if it is a .MP3 file, and if it can be opened
	
	if (argc != 2)
	{
		printf("Usage: %s filename.mp3\n", argv[0]);
		return(EXIT_FAILURE);
	}

	mp3Data->fileName = argv[1];

	char *lowerFileName;
	lowerFileName = strdup(argv[1]);

	char* mp3Found = strstr(strlwr(lowerFileName), ".mp3");
	if (mp3Found == NULL)
	{
		printf("File specified, %s, is not a .MP3 file\n\n", argv[1]);
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

long searchForSyncBits(struct MP3Data* mp3Data)
{
	mp3Data->headerOffset = -1; //assume offset won't be found
	for (long i = 0; i < (mp3Data->fileSize - 16); i++)
	{
		if (mp3Data->data[i] == (unsigned char)0xFF && (mp3Data->data[i + 1] & 0xF0) == (unsigned char)0xF0)
		{
			mp3Data->headerOffset = i;
			break; //found the offset to the start of the MP3 header data - stop searching
		}
	}
	return mp3Data->headerOffset; 
}

int mpegLayer3Check(struct MP3Data* mp3Data)
{
	mp3Data->isMPEG1 = ((mp3Data->data[mp3Data->headerOffset + 1] & 0x08) == 8);
	mp3Data->isLayer3 = ((mp3Data->data[mp3Data->headerOffset + 1] & 0x06) == 2);
	return (mp3Data->isMPEG1 && mp3Data->isLayer3);
}

int copyrightCheck(struct MP3Data* mp3Data)
{
	mp3Data->hasCopyright = ((mp3Data->data[mp3Data->headerOffset + 3] & 0x08) == 8);
	mp3Data->isOriginal = ((mp3Data->data[mp3Data->headerOffset + 3] & 0x04) == 4);
	mp3Data->alert = (mp3Data->hasCopyright && !mp3Data->isOriginal);
	return mp3Data->alert;
}

void getMP3Data(struct MP3Data* mp3Data)
{
	int bitrate = ((mp3Data->data[mp3Data->headerOffset + 2] & 0xF0))/16;
	int samplerate = ((mp3Data->data[mp3Data->headerOffset + 2] & 0x0C))/4;

	mp3Data->bitRate = bitRates[bitrate];
	mp3Data->sampleRate = sampleRates[samplerate];
	return;
}

void displayMP3Data(struct MP3Data* mp3Data)
{
	printf("********** MP3 Header Info **********");
	printf("\nMP3 file is MPEG version 1 Layer 3");
	printf("\n\nFile name: %s", mp3Data->fileName);
	printf("\nFile size: %.2f MB", (float)mp3Data->fileSize / ONE_MEG);

	printf("\n\nBit rate : %d kbps", mp3Data->bitRate);
	printf("\nFrequency: %.1f kHz\n\n", (float)mp3Data->sampleRate/1000);

	if (mp3Data->hasCopyright)
		printf("MP3 file is copyrighted");
	else
		printf("MP3 file is not copyrighted");
	
	if (mp3Data->isOriginal)
		printf("\nMP3 file is original file");
	else
		printf("\nMP3 file is a copy of the original");

	printf("\n\n");
}

int main(int argc, char** argv)
{
	system("cls");

	struct MP3Data mp3Data;
	mp3Data.data = NULL;

	if (initialize(argc, argv, &mp3Data) == EXIT_FAILURE)
	{
		if (fp != NULL)
			fclose(fp);			// close and free the file
		exit(EXIT_FAILURE);		// return 0;
	}

	if (readFile(&mp3Data) == EXIT_FAILURE)
	{
		fclose(fp);				// close and free the file
		exit(EXIT_FAILURE);		// return 0;
	}

	if (searchForSyncBits(&mp3Data) == -1)
	{
		printf("Could not find MP3 header info\n");
		exit(EXIT_FAILURE);
	}

	if(mpegLayer3Check(&mp3Data) == 0)
	{
		printf("MP3 file is not MPEG version 1 Layer3\n");
		exit(EXIT_FAILURE);
	}

	if (copyrightCheck(&mp3Data) == 1)
	{
		printf("MP3 is a copy of a copyrighted file!!");
		exit(EXIT_FAILURE);
	}

	getMP3Data(&mp3Data);

	displayMP3Data(&mp3Data);

	exit(EXIT_SUCCESS);	
}