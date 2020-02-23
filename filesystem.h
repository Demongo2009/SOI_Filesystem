//
// Created by demongo on 02.01.2020.
//

#ifndef SOI_LAB6_FILESYSTEM_H
#define SOI_LAB6_FILESYSTEM_H

#include <bits/types/FILE.h>

#define BLOCK_SIZE 4096
#define INODE_AMOUNT 160
#define DATA_BLOCK_AMOUNT 56
#define INODE_BLOCK_AMOUNT 5
#define FILENAME_LENGTH 50
#define MAX_FILES_IN_DIRECTORY 7
#define MAX_PATH_LENGTH 50
#define INDEX_OF_FIRST_INODE_IN_DIR 2 //because 0 - current, 1 - previous

#define UNUSED 0
#define USED 1
#define ROOT 0

#define UNUSED_INDEX -1
#define NOT_AN_INDEX -2

typedef unsigned int uint;
typedef unsigned long ulong;

typedef struct SuperBlock{
	uint dataBlockAmount;
	uint freeBlockAmount;
	uint inodeAmount;
	uint freeInodeAmount;

	char name[FILENAME_LENGTH];
	ulong size;

	uint inodeTableBlockOffset;
	uint bitVectorOffset;
	uint blockOffset;
}SuperBlock;

typedef struct Block{
	char data[BLOCK_SIZE- sizeof(int)];
	int nextBlock;
}Block;

typedef struct Inode{
	long size;
	int firstBlock;
	char isDirectory;
	char isLink;
	char filename[FILENAME_LENGTH];
}Inode;

//typedef struct IndexListElem{
//	int index;
//	struct IndexListElem* nextElem;
//}IndexListElem;

typedef struct Filesystem{
	SuperBlock superBlock;
	FILE* file;
}Filesystem;

typedef struct DirectoryBlock{
	int inodesInsideIndexTable[MAX_FILES_IN_DIRECTORY];
	int filesAmount;
}DirectoryBlock;

int createFilesystem(long size, char *name);
void initializeRoot();
void initializeInode(Inode *inode);
ulong getFileSize(FILE* file);
int existsOnFilesystem(char *filename);
int getFirstFreeBlockIndex();
int getFirstFreeInodeIndex();
void setBlockState(int index, int state);
void initializeDirectoryBlock(DirectoryBlock* directoryBlock, char* path, int inode);
int findDirectoryInode(char* directoryPath);
int addToDirectory(char* directoryPath, char* filename, long size, int inodeIndex, int addToTable);
uint lastCharIndex(char* path, char c);
long getFileSizeOnFilesystem(char* path);
void copyFromFilesystem(char* pathForm, char* pathTo);
int getFirstBlockIndexOfFile(char* path);
int findInode(char* path);
void deleteDirectoryFromFilesystem(char* path, int inodeIndex,int allDelete);
void deleteFileFromFilesystem(char* path);
void deleteInode(char* path);
void deleteFromDirectory(char* path, long size);
int findIndexInDirectory(char* path, DirectoryBlock directoryBlock);
void writeDirectoryBlock(DirectoryBlock* d, int i);
void writeBitVector(int* j, int i);
void writeSuperBlock();
void writeInode(Inode* inode, int inodeIndex);
void writeBlock(Block* block, int blockIndex);
void readBlock(Block* block, int blockIndex);
void initializeSuperBlock(SuperBlock* superBlock, long size);
void readInode(Inode* inode, int index);
void readDirectoryBlock(DirectoryBlock* directoryBlock, int index);
void copyToFilesystem(char *filename, char* directoryPath);
int createDirectory(char* path);
int openFilesystem(char* path);
void printFilesystem();
void closeFilesystem();
void addBytes(char* filePath, long numberOfBytes);
void removeBytes(char* filePath, long numberOfBytes);
void makeLink(char* pathFrom, char* pathTo);
void removeLink(char* pathFrom, char* pathTo);

/////////////////// zamienic sizeof Block na BLOCK_SIZE
#endif //SOI_LAB6_FILESYSTEM_H
