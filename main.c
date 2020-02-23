#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "filesystem.h"

#define MAX_LINE_LENGTH 50


ssize_t readline(char **lineptr, FILE *stream)
{
	size_t len = 0;  // Size of the buffer, ignored.

	ssize_t chars = getline(lineptr, &len, stream);

	if ((*lineptr)[chars - 1] == '\n')
	{
		(*lineptr)[chars - 1] = '\0';
		--chars;
	}

	return chars;
}

int main() {
	char c;
	char l[MAX_LINE_LENGTH] = {'\0'};
	char *line = l;
	size_t maxLength = MAX_LINE_LENGTH;

	int passed=0;
	while (!passed) {
	printf("FILESYSTEM!\n");
	printf("Do you want to: \n"
		   "c: create new virtual disc\n"
		   "o: open virtual disc\n"
		   "q: quit\n");
	c = getchar();
	getline(&line, &maxLength, stdin);
	int opened = 0;

	switch (c) {
		case 'c':
			printf("Type in size: \n");
			long size;
			readline(&line, stdin);
			size = atol(line);
			printf("Type in name: \n");
			readline(&line, stdin);
			createFilesystem(size, line);
			passed=1;
			break;

		case 'o':
			while (!opened) {
				printf("Type in filesystem path: \n");
				readline(&line, stdin);
				char path[MAX_LINE_LENGTH] = {'\0'};
				strcpy(path, line);
				if (openFilesystem(path)) {
					opened = 1;
				}

			}
			passed =1;
			break;

		case 'q':
			return (0);
		default:
			printf("Try again!\n");
			break;

	}
}
	while (1){

		printf("\nc: copy file form disc to filesystem\n"
			   "o: copy file form filesystem to disc\n"
			   "a: add new direcotry\n"
			   "d: delete a file\n"
	  "p: print filesystem data\n"
   "l: make link to file\n"
   "b: add bytes to file\n"
   "r: remove bytes from file\n"
			   "q: quit\n");
		char c = getchar();
		getline(&line, &maxLength, stdin);
		switch (c){
			case 'q':
				closeFilesystem();
				return(0);
			case 'p':
				printFilesystem();
				break;
			case 'c':
				printf("Type in filename: \n");
				readline(&line, stdin);
				char filename[MAX_LINE_LENGTH] = {'\0'};
				strcpy(filename, line);
				printf("Type in directoryPath: \n");
				readline(&line, stdin);
				char directoryPath[MAX_LINE_LENGTH] = {'\0'};
				strcpy(directoryPath, line);

				copyToFilesystem(filename, directoryPath);
				break;
			case 'o':
				printf("Type in source path: \n");
				readline(&line, stdin);
				char pathFrom[MAX_LINE_LENGTH]= {'\0'};
				strcpy(pathFrom, line);
				printf("Type in destination path: \n");
				readline(&line, stdin);
				char pathTo[MAX_LINE_LENGTH]= {'\0'};
				strcpy(pathTo, line);

				copyFromFilesystem(pathFrom, pathTo);
				break;
			case 'a':
				printf("Type in new directory path: \n");
				readline(&line, stdin);
				char path[MAX_LINE_LENGTH]= {'\0'};
				strcpy(path, line);
				createDirectory(path);
				break;
			case 'd':
				printf("Type in file to delete path: \n");
				readline(&line, stdin);
				char filePath[MAX_LINE_LENGTH]= {'\0'};
				strcpy(filePath, line);
				deleteFileFromFilesystem(filePath);
				break;
			case 'l':
				printf("Type in path of file to create link in: \n");
				readline(&line, stdin);
				char pathFromLink[MAX_LINE_LENGTH]= {'\0'};
				strcpy(pathFromLink, line);
				printf("Type in path of file to create link to: \n");
				readline(&line, stdin);
				char pathToLink[MAX_LINE_LENGTH]= {'\0'};
				strcpy(pathToLink, line);
				makeLink(pathFromLink, pathToLink);
				break;
			case 'b':
				printf("Type in path of file: \n");
				readline(&line, stdin);
				char pathBytes[MAX_LINE_LENGTH]= {'\0'};
				strcpy(pathBytes, line);
				printf("Type in number of bytes to add: \n");
				readline(&line, stdin);
				char numberOfBytesLine[MAX_LINE_LENGTH]= {'\0'};
				strcpy(numberOfBytesLine, line);
				long numberOfBytes = atol(numberOfBytesLine);
				addBytes(pathBytes, numberOfBytes);
				break;
			case 'r':
				printf("Type in path of file: \n");
				readline(&line, stdin);
				char pathBytesRemove[MAX_LINE_LENGTH]= {'\0'};
				strcpy(pathBytesRemove, line);
				printf("Type in number of bytes to remove: \n");
				readline(&line, stdin);
				char numberOfBytesLineRemove[MAX_LINE_LENGTH]= {'\0'};
				strcpy(numberOfBytesLineRemove, line);
				long numberOfBytesRemove = atol(numberOfBytesLineRemove);
				removeBytes(pathBytesRemove, numberOfBytesRemove);
				break;
			default:
				printf("Wrong input!\n");
				break;
		}
	}

	return 0;
}
