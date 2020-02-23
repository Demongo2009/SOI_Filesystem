//
// Created by demongo on 02.01.2020.
//

#include "filesystem.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

Filesystem filesystem;

void initializeSuperBlock(SuperBlock* superBlock, long size){
	superBlock->dataBlockAmount =0;
	superBlock->freeBlockAmount =(double)size/BLOCK_SIZE * (double)((double)56/120);
	superBlock->inodeAmount = (double)size/BLOCK_SIZE * (double)((double)56/120);
	superBlock->size=size;
	superBlock->inodeTableBlockOffset = sizeof(SuperBlock)+1;
	superBlock->bitVectorOffset = sizeof(SuperBlock)+ sizeof(Inode)*superBlock->inodeAmount+ 1;
	superBlock->blockOffset = sizeof(SuperBlock)+ sizeof(Inode)*superBlock->inodeAmount+ sizeof(int)*superBlock->freeBlockAmount +1;

}

void initializeInode(Inode* inode){
	inode->size = UNUSED;
	inode->firstBlock=0;
	inode->isDirectory=0;
	inode->isLink=0;
}

int openFilesystem(char* path){
	filesystem.file = fopen(path, "rb+");
	if(!filesystem.file) return 0;

	fread(&filesystem.superBlock, sizeof(SuperBlock), 1, filesystem.file);

	return 1;
}

void closeFilesystem(){
	if(filesystem.file){
		fclose(filesystem.file);
	}
}

int createFilesystem(long size, char *name){

	SuperBlock superBlock;
	initializeSuperBlock(&superBlock, size);
	filesystem.superBlock = superBlock;

	strcpy(filesystem.superBlock.name, name);

	filesystem.file = fopen(filesystem.superBlock.name, "wb+");
	if(!filesystem.file) return 0;
	ulong adress =0;

	fwrite(&filesystem.superBlock, sizeof(SuperBlock), 1, filesystem.file );

	for(int i=0; i < filesystem.superBlock.inodeAmount; i++){
		Inode inode;
		initializeInode(&inode);
		fwrite(&inode, sizeof(Inode), 1, filesystem.file);
	}

	for(int i=0; i < filesystem.superBlock.dataBlockAmount; i++){
		int j = UNUSED;
		fwrite(&j, sizeof(int), 1, filesystem.file);
	}

	initializeRoot();

	return 1;
}

void initializeRoot(){
	fseek(filesystem.file, filesystem.superBlock.inodeTableBlockOffset,0);
	Inode inode;
	initializeInode(&inode);
	char name[FILENAME_MAX] = {'\0'};
	name[0] = 'r';
	name[1] = 'o';
	name[2] = 'o';
	name[3] = 't';
//	name[4] = '/';
	strcpy(inode.filename, name);
	inode.isDirectory =1;
	inode.size = 1;
	DirectoryBlock directoryBlock;
	directoryBlock.filesAmount =0;
//	IndexListElem elem;
//	IndexListElem elem1;
//	elem.index = 0;
//	elem.nextElem = &elem1;
//	elem1.index = -1;
//	elem1.nextElem = NULL;
//	directoryBlock.inodesInsideIndexList = &elem;

	directoryBlock.inodesInsideIndexTable[0] = ROOT;
	directoryBlock.inodesInsideIndexTable[1] = NOT_AN_INDEX;

	for(int i=2 ; i<MAX_FILES_IN_DIRECTORY; i++){
		directoryBlock.inodesInsideIndexTable[i] = UNUSED_INDEX;
	}


	inode.firstBlock = 0;

	fwrite(&inode, sizeof(Inode), 1, filesystem.file);
	int j = USED;
	writeBitVector(&j,0);
	writeDirectoryBlock(&directoryBlock, 0);

	filesystem.superBlock.freeInodeAmount--;
	filesystem.superBlock.freeBlockAmount--;
	writeSuperBlock();
}

void writeSuperBlock(){
	fseek(filesystem.file,0,0);
	fwrite(&filesystem.superBlock, sizeof(SuperBlock), 1, filesystem.file);
}

void writeBitVector(int* j, int index){
	fseek(filesystem.file, filesystem.superBlock.bitVectorOffset + (index * sizeof(int)), 0);
	fwrite(j, sizeof(int),1, filesystem.file);
}

void writeDirectoryBlock(DirectoryBlock* directoryBlock, int index){
	fseek(filesystem.file, filesystem.superBlock.blockOffset + (index* BLOCK_SIZE), 0);
	fwrite(directoryBlock, sizeof(DirectoryBlock),1 , filesystem.file);
}

void writeInode(Inode* inode, int inodeIndex){
	fseek(filesystem.file, filesystem.superBlock.inodeTableBlockOffset + (inodeIndex * sizeof(Inode)), 0);
	fwrite(inode, sizeof(Inode), 1, filesystem.file);
}

void writeBlock(Block* block, int blockIndex){
	fseek(filesystem.file, filesystem.superBlock.blockOffset + (blockIndex* BLOCK_SIZE), 0);
	fwrite(block, sizeof(Block), 1, filesystem.file);
}

void printDirectory(int indexOfDirectoryInode, int spacesAmount){
	Inode currentInode;
	DirectoryBlock currentDirectoryBlock;

	readInode(&currentInode, indexOfDirectoryInode);

	readDirectoryBlock(&currentDirectoryBlock, currentInode.firstBlock);

	for(int i=2; i<MAX_FILES_IN_DIRECTORY; i++){
//			if(i== MAX_FILES_IN_DIRECTORY){
//				noFilesLeft = 1;
//				break;
//			}
		if(currentDirectoryBlock.inodesInsideIndexTable[i] == UNUSED_INDEX) continue;

		readInode(&currentInode, currentDirectoryBlock.inodesInsideIndexTable[i]);

		if(currentInode.size == UNUSED) continue;

		for(int j=0; j<spacesAmount; j++){
			printf(" ");
		}

		printf("%li %s\n",currentInode.size, currentInode.filename);

		if(currentInode.isDirectory){
			printDirectory(currentDirectoryBlock.inodesInsideIndexTable[i], spacesAmount+1);
		}


	}
}

void printFilesystem(){
//	FILE *copyFile = fopen(filesystemPath, "rb");
//	if(!copyFile){
//		printf("Cannot open file! %s\n", filesystemPath);
//		return;
//	}

	printf("Filesystem size: %li \n", filesystem.superBlock.size);

	Inode currentInode;

	readInode(&currentInode, 0);

	DirectoryBlock currentDirectoryBlock;

	readDirectoryBlock(&currentDirectoryBlock, 0);

	printf("Size used: %li  File name: %s\n", currentInode.size, currentInode.filename);

//	int noFilesLeft =0;
//	while(!noFilesLeft){
	int spacesAmount =1;
		for(int i=2; i<MAX_FILES_IN_DIRECTORY; i++){
//			if(i== MAX_FILES_IN_DIRECTORY){
//				noFilesLeft = 1;
//				break;
//			}
			if(currentDirectoryBlock.inodesInsideIndexTable[i] == UNUSED_INDEX) continue;
			readInode(&currentInode, currentDirectoryBlock.inodesInsideIndexTable[i]);

			if(currentInode.size == UNUSED) continue;

			for(int j=0; j<spacesAmount; j++){
				printf(" ");
			}

			printf("%li %s\n",currentInode.size, currentInode.filename);

			if(currentInode.isDirectory){
				printDirectory(currentDirectoryBlock.inodesInsideIndexTable[i], spacesAmount+1);
			}


		}
//	}

}

void copyToFilesystem(char *filename, char* directoryPath){
	FILE *copyFile = fopen(filename, "rb");
	if(!copyFile){
		printf("Cannot open file! %s\n", filename);
		return;
	}

	ulong copyFileSize = getFileSize(copyFile);
	if(copyFileSize > filesystem.superBlock.freeBlockAmount * BLOCK_SIZE){
		printf("Not enough space on filesystem to copy file %s\n", filename);
		return;
	}

	if(filesystem.superBlock.freeInodeAmount <= 0){
		printf("Max files number reached!");
		return;
	}

	if(existsOnFilesystem(directoryPath)){
		printf("File exists on filesystem %s\n", directoryPath);
		return;
	}

	int blockIndex = getFirstFreeBlockIndex();

	int inodeIndex = getFirstFreeInodeIndex();
	Inode inode;
	inode.size = copyFileSize;
	inode.firstBlock = blockIndex;
	strcpy(inode.filename, filename);
	inode.filename[strlen(filename)] = '\0';


	int copyFileSizeInBlocks = copyFileSize/BLOCK_SIZE;
	if(copyFileSize%BLOCK_SIZE !=0) copyFileSizeInBlocks+=1;


	switch (addToDirectory(directoryPath, filename, copyFileSize, inodeIndex,1)){
		case 0 :
			printf("Directory incorrect %s\n", directoryPath);
			return;
			break;
		case -1:
			printf("Too many files in directory");
			return;
			break;
	}

	for(int i=0; i< copyFileSizeInBlocks; ++i){
		Block block;
		fread(block.data, sizeof(Block), 1, copyFile);
		setBlockState(blockIndex, USED);
		int nextBlockIndex = getFirstFreeBlockIndex();
		if( (i+1) == copyFileSizeInBlocks) nextBlockIndex = UNUSED_INDEX;
		block.nextBlock = nextBlockIndex;
		writeBlock(&block, blockIndex);
		blockIndex = nextBlockIndex;
	}

	writeInode(&inode, inodeIndex);

	filesystem.superBlock.freeInodeAmount--;
	filesystem.superBlock.freeBlockAmount -= copyFileSizeInBlocks;
	writeSuperBlock();

	fclose(copyFile);

	printf("Successfully copied file %s! ", filename);
}

void copyFromFilesystem(char* pathFrom, char* pathTo){
	char* filename = pathFrom + lastCharIndex(pathFrom,'/')+1;

	if( !existsOnFilesystem(pathFrom) ){
		printf("No such file %s on disc\n", filename);
		return;
	}

	FILE* destinationFile = fopen(pathTo, "wb+");
	if(!destinationFile){
		printf("Cannot create a file %s\n", pathTo);
		return;
	}

	long size = getFileSizeOnFilesystem(pathFrom);
	if(size == -1){
		printf("Not enough space");
		return;
	}

	int inodeIndex = findInode(pathFrom);
	int blockIndex;
	blockIndex = getFirstBlockIndexOfFile(pathFrom);

	Inode inode;
	readInode(&inode, inodeIndex);

	Inode linkedInode;
	if(inode.isLink){
		readInode(&linkedInode, inode.firstBlock);

		blockIndex = linkedInode.firstBlock;
		size = linkedInode.size;

	}

	Block block;

	do{
		readBlock(&block, blockIndex);

		size -= BLOCK_SIZE;

		if(size < 0 ) {
			fwrite(block.data, BLOCK_SIZE + size, 1, destinationFile);
			break;
		}
		else fwrite(block.data, BLOCK_SIZE, 1, destinationFile);

		blockIndex = block.nextBlock;

	}while (blockIndex != UNUSED_INDEX);

	fclose(destinationFile);
	printf("successfull copy from %s to %s\n", pathFrom, pathTo);
}

void readBlock(Block* block, int blockIndex){
	fseek(filesystem.file, filesystem.superBlock.blockOffset+ (blockIndex* BLOCK_SIZE),0);
	fread(block, sizeof(Block), 1, filesystem.file);
}

void addBytes(char* filePath, long numberOfBytes){
	if(!existsOnFilesystem(filePath)){
		printf("No such file %s\n", filePath);
		return;
	}

	Inode inode;
	int inodeIndex = findInode(filePath);

	Inode directoryInode;
	char directoryPath[MAX_PATH_LENGTH] = {'\0'};
	int lastSlashIndex = lastCharIndex(filePath, '/');
	strncpy(directoryPath, filePath, lastSlashIndex);

	int directoryIndex = findDirectoryInode(directoryPath);

	readInode(&inode, inodeIndex);

	readInode(&directoryInode, directoryIndex);

	inode.size+= numberOfBytes;
	long currentSize = numberOfBytes;
	int fileSizeInBlocks = numberOfBytes/BLOCK_SIZE;

	writeInode(&inode, inodeIndex);


	addToDirectory(directoryPath, filePath+lastSlashIndex+1, numberOfBytes, inodeIndex, 0);

}

void removeBytes(char* filePath, long numberOfBytes){
	if(!existsOnFilesystem(filePath)){
		printf("No such file %s\n", filePath);
		return;
	}

	Inode inode;
	int inodeIndex = findInode(filePath);

	Inode directoryInode;
	char directoryPath[MAX_PATH_LENGTH] = {'\0'};
	int lastSlashIndex = lastCharIndex(filePath, '/');
	strncpy(directoryPath, filePath, lastSlashIndex);

	int directoryIndex = findDirectoryInode(directoryPath);

	readInode(&inode, inodeIndex);

	readInode(&directoryInode, directoryIndex);

	inode.size-= numberOfBytes;
	long currentSize = numberOfBytes;
	int fileSizeInBlocks = numberOfBytes/BLOCK_SIZE;

	writeInode(&inode, inodeIndex);


	addToDirectory(directoryPath, filePath+lastSlashIndex+1, -numberOfBytes, inodeIndex, 0);

}

void makeLink(char* pathFrom, char* pathTo){

	if(!existsOnFilesystem(pathTo)){
		printf("No such file %s\n", pathTo);
		return;
	}

	if(existsOnFilesystem(pathFrom)){
		printf("Such file exists %s\n", pathTo);
		return;
	}

	char filename[FILENAME_MAX]= {'\0'};
	int lastSlash = lastCharIndex(pathFrom,'/');
	strcpy(filename, pathFrom+lastSlash+1);

	Inode sourceInode;
	int sourceInodeIndex = findInode(pathTo);

	if(sourceInodeIndex == -1){
		printf("No such file %s", pathTo);
		return;
	}

	readInode(&sourceInode, sourceInodeIndex);

	Inode inode;
	int inodeIndex = getFirstFreeInodeIndex();

	inode.size = 1;
	inode.firstBlock = sourceInodeIndex;
	inode.isDirectory = 0;
	inode.isLink = 1; //TODO links in initializations
	strcpy(inode.filename, filename);

	writeInode(&inode, inodeIndex);

	addToDirectory(pathFrom, filename, 0, inodeIndex,1);

}


void removeLink(char* pathFrom, char* pathTo){
	if(!existsOnFilesystem(pathTo)){
		printf("No such file %s\n", pathTo);
		return;
	}

	if(existsOnFilesystem(pathFrom)){
		printf("Such file exists %s\n", pathTo);
		return;
	}

	char filename[FILENAME_MAX]= {'\0'};
	int lastSlash = lastCharIndex(pathFrom,'/');
	strcpy(filename, pathFrom+lastSlash+1);

	Inode sourceInode;
	int sourceInodeIndex = findInode(pathTo);

	if(sourceInodeIndex == -1){
		printf("No such file %s", pathTo);
		return;
	}

	readInode(&sourceInode, sourceInodeIndex);

	Inode inode;
	int inodeIndex = getFirstFreeInodeIndex();

	inode.size = 1;
	inode.firstBlock = sourceInodeIndex;
	inode.isDirectory = sourceInode.isDirectory;
	strcpy(inode.filename, filename);

	writeInode(&inode, inodeIndex);

	addToDirectory(pathFrom, filename, 0, inodeIndex,1);
}



void deleteFileFromFilesystem(char* path){
	if(!existsOnFilesystem(path)){
		printf("No such file %s\n", path);
		return;
	}

	int blockIndex = getFirstBlockIndexOfFile(path);
	int sizeInBlocks = 0;
	long size=0;

	Inode inode;
	int inodeIndex = findInode(path);
	if(inodeIndex == -1){
		inodeIndex = findDirectoryInode(path);
		if(inodeIndex == -1){
			printf("Error");
			return;
		}else{
			deleteDirectoryFromFilesystem(path, inodeIndex,0); // TODO
			return;
		}
	}


	do{
		Block block;
		readBlock(&block, blockIndex);



		setBlockState(blockIndex, UNUSED);

		blockIndex = block.nextBlock;
		++sizeInBlocks;

	}while (blockIndex != UNUSED_INDEX);

	deleteInode(path);


	filesystem.superBlock.freeInodeAmount ++;
	filesystem.superBlock.freeBlockAmount += sizeInBlocks;
	writeSuperBlock();

	printf("successful deletion %s\n", path);
}

void deleteDirectoryFromFilesystem(char* path, int directoryIndex, int allDelete){
	char* filename = path + lastCharIndex(path,'/')+1;

	Inode inode;
	readInode(&inode, directoryIndex);

	int blockIndex = inode.firstBlock;

	DirectoryBlock directoryBlock;
	readDirectoryBlock(&directoryBlock, blockIndex);

//	IndexListElem* curr =directoryBlock.inodesInsideIndexList;
	int i=2;
	while (i<MAX_FILES_IN_DIRECTORY){
//		int fileInodeIndex = curr->index;

		if(directoryBlock.inodesInsideIndexTable[i] != UNUSED_INDEX){
			Inode fileInode;
			readInode(&fileInode, directoryBlock.inodesInsideIndexTable[i]);

			if(strcmp(fileInode.filename, filename) != 0 && allDelete !=1){
				continue;
			}
			char directoryName[MAX_PATH_LENGTH]={'\0'};
			strcpy(directoryName, fileInode.filename);
			char directoryNameWithSlash[MAX_PATH_LENGTH]={'\0'};
			directoryNameWithSlash[0] = '/';
			strcat(directoryNameWithSlash, directoryName);
//			directoryNameAndInsides[strlen(fileInode.filename)]= '/';

			if(fileInode.isDirectory){

				deleteDirectoryFromFilesystem(directoryNameWithSlash, directoryBlock.inodesInsideIndexTable[i], 1);
			}else{
				deleteFileFromFilesystem(directoryNameWithSlash);
			}
		}
		i++;
//		curr = curr->nextElem;
	}

	setBlockState(blockIndex,UNUSED);
	deleteInode(path);

}

void readInode(Inode* inode, int index){
	fseek(filesystem.file, filesystem.superBlock.inodeTableBlockOffset+(index* sizeof(Inode)),0);
	fread(inode, sizeof(Inode),1, filesystem.file);
}

void readDirectoryBlock(DirectoryBlock* directoryBlock, int index){
	fseek(filesystem.file, filesystem.superBlock.blockOffset+(index* BLOCK_SIZE), 0);
	fread(directoryBlock, sizeof(DirectoryBlock),1,filesystem.file);
}

void deleteInode(char* path){
	ulong address = filesystem.superBlock.inodeTableBlockOffset;
	char* filename = path + lastCharIndex(path,'/')+1;

	for(int i=0; i < filesystem.superBlock.inodeAmount; ++i){
		Inode inode;
		fseek(filesystem.file, address, 0);
		fread(&inode, sizeof(Inode), 1, filesystem.file);

		if(inode.size != UNUSED && (strcmp(filename, inode.filename) == 0)){
			deleteFromDirectory(path, inode.size);
			inode.size = UNUSED;
			fseek(filesystem.file, address, 0);
			fwrite(&inode, sizeof(Inode), 1, filesystem.file);

			return;
		}
		address += sizeof(Inode);
	}
}

void deleteFromDirectory(char* path, long size){
	uint lastSlashIndex = lastCharIndex(path, '/');
	char prevDirectoryPath[MAX_PATH_LENGTH]= {'\0'};
	strncpy(prevDirectoryPath, path, lastSlashIndex);
	prevDirectoryPath[lastSlashIndex] = '/';
	char* filename = path + lastCharIndex(path,'/')+1;;

	int directoryInodeIndex = findDirectoryInode(prevDirectoryPath);

	Inode directoryInode;
	readInode(&directoryInode, directoryInodeIndex);

	directoryInode.size -= size;

	writeInode(&directoryInode, directoryInodeIndex);

	DirectoryBlock directoryBlock;
	ulong directoryBlockAddress = directoryInode.firstBlock;
	readDirectoryBlock(&directoryBlock, directoryBlockAddress);

	int directoryIndex = findIndexInDirectory(path, directoryBlock);
//	IndexListElem* curr=directoryBlock.inodesInsideIndexList;
	if(directoryIndex == NOT_AN_INDEX){
		printf("Wrong directory!");
		return;
	}else{
		directoryBlock.inodesInsideIndexTable[directoryIndex] = UNUSED_INDEX;

	}
	directoryBlock.filesAmount--;

	writeDirectoryBlock(&directoryBlock, directoryBlockAddress);


}

int findIndexInDirectory(char* path, DirectoryBlock directoryBlock){
	char* filename = path + lastCharIndex(path,'/')+1;;

//	IndexListElem* curr = directoryBlock.inodesInsideIndexList;
	int i=2;
	while (i<MAX_FILES_IN_DIRECTORY){
//		int inodeIndex = curr->index;

//		curr= curr->nextElem;
		Inode inode;
		readInode(&inode, directoryBlock.inodesInsideIndexTable[i]);

		if(inode.size != UNUSED && (strcmp(filename, inode.filename) == 0)){
			return i;
		}
		i++;
	}

	return NOT_AN_INDEX;
}

int findInode(char* path){
	ulong address = filesystem.superBlock.inodeTableBlockOffset;
	char* filename = path + lastCharIndex(path,'/')+1;;

	for(int i=0; i < filesystem.superBlock.inodeAmount; i++){
		Inode inode;
		fseek(filesystem.file, address, 0);
		fread(&inode, sizeof(Inode), 1, filesystem.file);

		if(inode.size != UNUSED && inode.isDirectory == 0 && (strcmp(filename, inode.filename) == 0)){
			return i;
		}

		address += sizeof(Inode);
	}

	return -1;
}

int getFirstBlockIndexOfFile(char* path){
	ulong address = filesystem.superBlock.inodeTableBlockOffset;
	char* filename = path + lastCharIndex(path,'/')+1;;


	for(int i=0; i < filesystem.superBlock.inodeAmount; i++){
		Inode inode;
		fseek(filesystem.file, address, 0);
		fread(&inode, sizeof(Inode), 1, filesystem.file);

		if(inode.size != UNUSED && (strcmp(filename, inode.filename) == 0) ){
			return inode.firstBlock;
		}

		address += sizeof(Inode);
	}
}

long getFileSizeOnFilesystem(char* path){
	ulong address = filesystem.superBlock.inodeTableBlockOffset;

	char* filename = path + lastCharIndex(path,'/')+1;;

	for(int i=0; i < filesystem.superBlock.inodeAmount; i++){
		Inode inode;
		fseek(filesystem.file, address, 0);
		fread(&inode, sizeof(Inode), 1, filesystem.file);

		if(inode.size != UNUSED && (strcmp(filename, inode.filename) == 0 ) ){
			return inode.size;
		}
		address+= sizeof(Inode);
	}
	return -1;
}

int addToDirectory(char* directoryPath, char* filename, long sizeAdded, int fileInodeIndex, int addToTable){
	DirectoryBlock directoryBlock;
	Inode inode;

	int inodeIndex = findDirectoryInode(directoryPath);
	if(inodeIndex == -1 ){
		return 0;
	}
	if( inodeIndex == ROOT){
		// root
		readInode(&inode, 0);

		if(!inode.isDirectory){
			return 0;
		}

		inode.size += sizeAdded;

		writeInode(&inode, 0);

		if(addToTable){

			readDirectoryBlock(&directoryBlock, 0);
// TODO next elem in list is not null


//		int newFileIndex = INDEX_OF_FIRST_INODE_IN_DIR;
//		IndexListElem* curr= directoryBlock.inodesInsideIndexList;

			int i=INDEX_OF_FIRST_INODE_IN_DIR;
			while (i<=MAX_FILES_IN_DIRECTORY && directoryBlock.inodesInsideIndexTable[i] != UNUSED_INDEX) {
				i++;
			}
			if(i == MAX_FILES_IN_DIRECTORY){
				printf("To many files in directory");
				return -1;
			}

//		IndexListElem elem;
//		elem.index = fileInodeIndex;
//		elem.nextElem = NULL;
//		curr->nextElem = &elem;
			directoryBlock.inodesInsideIndexTable[i] = fileInodeIndex;
			directoryBlock.filesAmount++;

			writeDirectoryBlock(&directoryBlock, 0);
		}

//		printf("f");
		return 1;

	}else{
		readInode(&inode, inodeIndex);

		if(!inode.isDirectory){
			return 0;
		}
		inode.size += sizeAdded;
		int blockIndex = inode.firstBlock;

		writeInode(&inode, inodeIndex);

		if(addToTable){

			readDirectoryBlock(&directoryBlock, blockIndex);

//		int newFileIndex = INDEX_OF_FIRST_INODE_IN_DIR;
//		IndexListElem* curr= directoryBlock.inodesInsideIndexList;
//		int i=0;
//		while (curr->nextElem && i<directoryBlock.filesAmount) {
//			curr= curr->nextElem;
//			i++;
//		}

			int i=INDEX_OF_FIRST_INODE_IN_DIR;
			while (i<=MAX_FILES_IN_DIRECTORY && directoryBlock.inodesInsideIndexTable[i] != UNUSED_INDEX) {
				i++;
			}
			if(i == MAX_FILES_IN_DIRECTORY){
				printf("To many files in directory");
				return -1;
			}

//		IndexListElem elem;
//		elem.index = fileInodeIndex;
//		elem.nextElem = NULL;
//		curr->nextElem = &elem;
			directoryBlock.inodesInsideIndexTable[i] = fileInodeIndex;
			directoryBlock.filesAmount++;

			writeDirectoryBlock(&directoryBlock, blockIndex);
		}


		uint lastSlashIndex = lastCharIndex(directoryPath, '/');
		char prevDirectoryPath[MAX_PATH_LENGTH];
		strncpy(prevDirectoryPath, directoryPath, strlen(directoryPath)-lastSlashIndex);
		addToDirectory(prevDirectoryPath, inode.filename, sizeAdded, inodeIndex, 0);
	}
	return 1;
}

int createDirectory(char* path){

	DirectoryBlock directoryBlock;
	Inode inode;
	initializeInode(&inode);
	inode.size= 1;
	inode.isDirectory= 1;
	char* lastSlash = path + lastCharIndex(path,'/')+1;;
	strcpy(inode.filename, lastSlash);
	inode.filename[strlen(lastSlash)] = '\0';

	int blockIndex = getFirstFreeBlockIndex();
	int inodeIndex = getFirstFreeInodeIndex();

	inode.firstBlock = blockIndex;
	initializeDirectoryBlock(&directoryBlock, path, inodeIndex);

	addToDirectory(path, inode.filename, inode.size, inodeIndex, 1);

	writeInode(&inode, inodeIndex);

	setBlockState(blockIndex, USED);

	writeDirectoryBlock(&directoryBlock, blockIndex);

	filesystem.superBlock.freeInodeAmount--;
	filesystem.superBlock.freeBlockAmount--;
	writeSuperBlock();

	printf("Successfully created directory");

}

uint lastCharIndex(char *str, char ch){
	char *chptr = strrchr(str, ch);
	if (chptr == NULL) return -1;
	return chptr - str;
}

uint firstCharIndex(char *str, char ch){
	char *chptr = strchr(str, ch);
	if (chptr == NULL) return -1;
	return chptr - str;
}

void initializeDirectoryBlock(DirectoryBlock*  directoryBlock, char* path, int currentInodeIndex){
	directoryBlock->filesAmount=0;
//	IndexListElem elem;
//	elem.index = currentInodeIndex;
//	elem.nextElem = NULL;
//	directoryBlock->inodesInsideIndexList = &elem;
	directoryBlock->inodesInsideIndexTable[0] = currentInodeIndex;

	uint lastSlashIndex = lastCharIndex(path, '/');
	char prevDirectoryPath[MAX_PATH_LENGTH]={'\0'};
	strncpy(prevDirectoryPath, path, lastSlashIndex+1);
	prevDirectoryPath[lastSlashIndex+1] = '\0';
	int prevDirectoryInode = findDirectoryInode(prevDirectoryPath);
//	IndexListElem elem1;
//	elem.nextElem = &elem1;

	if(prevDirectoryInode == -1){
//		elem1.index = ROOT;
		directoryBlock->inodesInsideIndexTable[1] = ROOT;
	}else{
//		elem1.index = prevDirectoryInode;
		directoryBlock->inodesInsideIndexTable[1] = prevDirectoryInode;
	}

	for(int i=2; i <MAX_FILES_IN_DIRECTORY;i++){
		directoryBlock->inodesInsideIndexTable[i] = UNUSED_INDEX;
	}
}

int findDirectoryInode(char* directoryPath){
	ulong address = filesystem.superBlock.inodeTableBlockOffset;
	char toLastSlash[MAX_PATH_LENGTH]={'\0'};
	char directoryName[FILENAME_MAX]={'\0'};

	if(lastCharIndex(directoryPath, '/') == -1){
//		printf("Must start with root/\n");
		return 0;
	}else{
		int indexLastChar = lastCharIndex(directoryPath,'/');
		strncpy(toLastSlash, directoryPath, indexLastChar);

		indexLastChar = lastCharIndex(toLastSlash,'/');

		if(indexLastChar == -1){
			strcpy(directoryName, toLastSlash);
			directoryName[strlen(toLastSlash)] = '\0';

		}else{
			strcpy(directoryName, toLastSlash+indexLastChar+1);
		}

	}

	for(int i=0; i < filesystem.superBlock.inodeAmount; i++){
		Inode inode;
		fseek(filesystem.file, address, 0);
		fread(&inode, sizeof(Inode), 1, filesystem.file);
		if(inode.size != UNUSED && inode.isDirectory==1 && (strcmp(directoryName, inode.filename))==0) return i;
		address += sizeof(Inode);
	}
	return -1;
}

void setBlockState(int index, int state){
	ulong adress = filesystem.superBlock.bitVectorOffset + (index* sizeof(int));
	int bit =state;
	fseek(filesystem.file, adress,0);
	fwrite(&bit, sizeof(int), 1, filesystem.file);
}

int getFirstFreeInodeIndex(){
	ulong address =filesystem.superBlock.inodeTableBlockOffset;
	Inode inode;
	initializeInode(&inode);
	int i=0;

	do{
		fseek(filesystem.file,address,0);
		fread(&inode, sizeof(Inode), 1 ,filesystem.file);
		address += sizeof(Inode);
		++i;
	}while (inode.size != UNUSED && address < filesystem.superBlock.bitVectorOffset);

	return i-1;
}

int getFirstFreeBlockIndex(){
	ulong adress = filesystem.superBlock.bitVectorOffset;
	int bit;
	int i;
	i=0;

	do{
		fseek(filesystem.file, adress, 0);
		fread(&bit, sizeof(int), 1, filesystem.file);
		adress += sizeof(int);
		++i;
	}while (bit == USED);

	return i-1;
}

ulong getFileSize(FILE* file){
	fseek(file, 0L, SEEK_END);
	ulong size = ftell(file);
	fseek(file, 0, 0);
	return size;
}

int existsOnFilesystem(char *filename){
	ulong address = filesystem.superBlock.inodeTableBlockOffset;
	int firstIndex = firstCharIndex(filename, '/');
	if(firstIndex == -1){
		printf("must begin with root/\n");
		return -1;
	}

	char currentName[FILENAME_MAX]={'\0'};
	char currentPath[FILENAME_MAX]={'\0'};
	strncpy(currentName, filename, firstIndex);
	strcpy(currentPath, filename);

	int passedTest=0;
//	while(currentName[0]!=0){
	Inode inode;
	address = filesystem.superBlock.inodeTableBlockOffset;
	passedTest=0;
	for(int i=0; i < filesystem.superBlock.inodeAmount; i++){
			fseek(filesystem.file, address, 0);
			fread(&inode, sizeof(Inode), 1, filesystem.file);
			if(inode.size != UNUSED && (strcmp(currentName, inode.filename) == 0) ){
				passedTest=1;
				break;
			}
			address += sizeof(Inode);
		}

		if(!passedTest) return 0;

	while(passedTest!=0){
		if(firstIndex == -1){
			break;
		}
//		char pathToDirectory[MAX_PATH_LENGTH] = {'\0'};
//		int indexOfSlashOfCurrDirectory = firstIndex + firstCharIndex(filename+firstIndex+1, '/');
//		strncpy(pathToDirectory, filename, indexOfSlashOfCurrDirectory);
		int indexDirectoryBlock = inode.firstBlock;
		strcpy(currentPath, currentPath+firstIndex+1);

		firstIndex = firstCharIndex(currentPath, '/');
		if(firstIndex == -1){
			strcpy(currentName, currentPath);

		}else{

			strncpy(currentName, currentPath, firstIndex);
			currentName[firstIndex] = '\0';
		}

		if(currentName[0]==0){
			break;
		}

		DirectoryBlock directoryBlock;

		if( indexDirectoryBlock == -1){
			return 0;
		}

		readDirectoryBlock(&directoryBlock, indexDirectoryBlock);

		passedTest =0;
		for(int i=INDEX_OF_FIRST_INODE_IN_DIR; i<=MAX_FILES_IN_DIRECTORY; i++){
			if(i == MAX_FILES_IN_DIRECTORY){
				return 0;
			}
			int indexOfFileInode = directoryBlock.inodesInsideIndexTable[i];
			if(indexOfFileInode == UNUSED_INDEX) continue;

			readInode(&inode, indexOfFileInode);

			if(strcmp(inode.filename, currentName)==0){
				passedTest = 1;
				break;
			}
		}
	}
//	}

	if(!passedTest) return 0;

	return 1;
}


