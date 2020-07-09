#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#define BLOCK 1024
#define IBLOCKS 3

struct sBlock{
    int size;
    int files;
    int freeSpace;
    int iBlocks;
    int iAddr;
}info;

struct Inode{
    int d_addr[10];
    int ind_addr;
    int size;
    char name[10];
}inodes[BLOCK*IBLOCKS];


int ls( void ){
    FILE *disk;
    int i=0;
    int numOfInodes;
    char iMask[BLOCK*IBLOCKS];
    char dMask[BLOCK];

    disk = fopen("virtualDisk", "r");
    if(!disk){
        printf("Virtual disk doesn't exist\n");
        return -1;
    }

    if(!fread(&info, sizeof(info), 1, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }

    numOfInodes = info.iBlocks*BLOCK/sizeof(struct Inode)-1;

    if(fseek(disk, BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(iMask, sizeof(char), numOfInodes, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }
    if(fseek(disk, 2*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(dMask, sizeof(char), info.size-info.iBlocks-3, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }
    if(fseek(disk, 3*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(inodes, sizeof(struct Inode), numOfInodes, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }

    if(info.files == 0){
        printf("There are not any files on disk\n");
        return -1;
    }
    printf("Name\t\tSize\t\tAddress\n");

    for(i=0;i < numOfInodes;i++){
        if(iMask[i] == 1){
            printf("%s\t\t%d B\t\t%d\n", inodes[i].name, inodes[i].size, inodes[i].d_addr[0]);
        }
    }

    if(fclose(disk)){
        printf("Error: can't close the disk\n");
        return -1;
    }

    return 0;
}

int seeDiskMap( void ){
    FILE *disk;
    int i=0;
    int numOfInodes;
    char iMask[BLOCK*IBLOCKS];
    char dMask[BLOCK];
    int bInodes = 0;
    int bBlocks = 0;

    disk = fopen("virtualDisk", "r");
    if(!disk){
        printf("Virtual disk doesn't exist\n");
        return -1;
    }

    if(!fread(&info, sizeof(info), 1, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }
    printf("Disk's size: %d kB\n", info.size);
    printf("Size for data: %d kB\n", info.size - info.iBlocks - 3);
    printf("Amount of files on disk: %d\n", info.files);
    printf("Free space: %d kB\n", info.freeSpace);

    numOfInodes = info.iBlocks*BLOCK/sizeof(struct Inode)-1;

    if(fseek(disk, BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(iMask, sizeof(char), numOfInodes, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }
    if(fseek(disk, 2*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(dMask, sizeof(char), info.size-info.iBlocks-3, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }
    if(fseek(disk, 3*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(inodes, sizeof(struct Inode), numOfInodes, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }

    for(i=0;i < numOfInodes;i++){
        if(iMask[i] == 1)
            bInodes++;
    }

    for(i=0;i < info.size-info.iBlocks-3;++i){
        if(dMask[i] == 1)
            bBlocks++;
    }

    printf("Busy blocks: %d\t", bBlocks);
    printf("Free blocks: %d\n", info.size - 3 - info.iBlocks - bBlocks);
    printf("Busy inodes: %d\t", bInodes);
    printf("Free inodes: %d\n", numOfInodes - bInodes);

    if(fclose(disk)){
        printf("Error: can't close the disk\n");
        return -1;
    }

    return 0;
}

int removeFile(char *filename){
    FILE *disk;
    int i=0;
    int size;
    int ind_Addresses[BLOCK];
    int fileInode;
    int busyBlocks;
    int numOfInodes;
    int dBlocks;
    char iMask[BLOCK*IBLOCKS];
    char dMask[BLOCK];

    disk = fopen("virtualDisk", "r+");
    if(!disk){
        printf("Virtual disk doesn't exist\n");
        return -1;
    }

    printf("Removing %s...\n", filename);

    if(!fread(&info, sizeof(info), 1, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }
    numOfInodes = info.iBlocks*BLOCK/sizeof(struct Inode)-1;
    dBlocks = info.size - 3 - info.iBlocks;

    if(fseek(disk, BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(iMask, sizeof(char), numOfInodes, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }
    if(fseek(disk, 2*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(dMask, sizeof(char), info.size-info.iBlocks-3, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }
    if(fseek(disk, 3*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(inodes, sizeof(struct Inode), numOfInodes, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }

    for(i=0;i < numOfInodes;i++){
        if(iMask[i] == 1 && strcmp(filename, inodes[i].name) == 0)
            break;
    }
    if(i == numOfInodes){
        printf("There is not such a file on disk\n");
        return -1;
    }

    fileInode = i;

    if(inodes[fileInode].size%BLOCK == 0)
        busyBlocks = inodes[fileInode].size/BLOCK;
    else
        busyBlocks = inodes[fileInode].size/BLOCK + 1;

    if(inodes[fileInode].size > 10*BLOCK)
        busyBlocks++;

    iMask[fileInode] = 0;
    info.files--;
    info.freeSpace += busyBlocks;

    if(busyBlocks <= 10){
        for(i=0;i < busyBlocks;++i)
            dMask[inodes[fileInode].d_addr[i]/BLOCK-6] = 0;
    }
    else{
        for(i=0; i < 10;++i)
            dMask[inodes[fileInode].d_addr[i]/BLOCK - 6] = 0;
        dMask[inodes[fileInode].ind_addr/BLOCK - 6] = 0;
        busyBlocks -= 11;
        if(fseek(disk, inodes[fileInode].ind_addr, 0)){
            printf("Error: can't seek\n");
            return -1;
        }
        if(!fread(ind_Addresses, sizeof(int), busyBlocks, disk)){
            printf("Error: can't read from disk\n");
            return -1;
        }
        for(i=0;i < busyBlocks;++i){
            dMask[ind_Addresses[i]/BLOCK - 6] = 0;
        }
    }

    if(fseek(disk, 0, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fwrite(&info, sizeof(info), 1, disk)){
        printf("Error: can't write to disk\n");
        return -1;
    }
    if(fseek(disk, BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fwrite(iMask, sizeof(char), numOfInodes, disk)){
        printf("Error: can't write to disk\n");
        return -1;
    }
    if(fseek(disk, 2*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fwrite(dMask, sizeof(char), dBlocks, disk)){
        printf("Error: can't write to disk\n");
        return -1;
    }
    if(fseek(disk, 3*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fwrite(inodes, sizeof(inodes[0]), numOfInodes, disk)){
        printf("Error: can't write to disk\n");
        return -1;
    }

    printf("File removed\n");

    if(fclose(disk)){
        printf("Error: can't close the disk\n");
        return -1;
    }

    return 0;
}

int copyToDisk(char *filename){
    FILE *fp, *disk;
    int copiedSize, size;
    char buff[BLOCK];
    int i=0, j=0;
    int neededBlocks;
    int freeInode;
    int numOfInodes;
    int dBlocks;
    int freeIndAddr[BLOCK];
    char iMask[BLOCK*IBLOCKS];
    char dMask[BLOCK];
    int freeBlocks[BLOCK];

    printf("Adding %s...\n", filename);

    fp = fopen(filename, "r");
    if(!fp){
        printf("File with this name doesn't exist\n");
        return -1;
    }

    disk = fopen("virtualDisk", "r+");

    if(!disk){
        printf("Virtual disk doesn't exist\n");
        return -1;
    }
    if(!fread(&info, sizeof(info), 1, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }

    numOfInodes = info.iBlocks*BLOCK/sizeof(struct Inode)-1;
    dBlocks = info.size - 3 - info.iBlocks;
    freeBlocks[dBlocks];

    if(info.files == numOfInodes){
        printf("There is not space for another file\n");
        return -1;
    }

    if(fseek(disk, BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(iMask, sizeof(char), numOfInodes, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }
    if(fseek(disk, 2*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(dMask, sizeof(char), dBlocks, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }
    if(fseek(disk, 3*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(inodes, sizeof(struct Inode), numOfInodes, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }

    for(i=0;i < numOfInodes;++i){
        if(iMask[i] == 1 && strcmp(inodes[i].name, filename) == 0){
            printf("There is already such a file\n");
            return -1;
        }
    }

    for(i=0;i < numOfInodes;++i)
        if(iMask[i] == 0)
            break;

    freeInode = i;

    for(i=0;i < dBlocks;++i){
        if(dMask[i] == 0){
            freeBlocks[j] = i;
            j++;
        }
    }

    if(fseek(fp, 0, 2)){
        printf("Error: can't seek\n");
        return -1;
    }

    copiedSize = ftell(fp);

    if(copiedSize > 272384){
        printf("File is too big (266kB max)\n");
        return -1;
    }
    if(copiedSize%BLOCK == 0){
        neededBlocks = copiedSize/BLOCK;
        info.freeSpace -= copiedSize/BLOCK;
    }
    else{
        neededBlocks = copiedSize/BLOCK+1;
        info.freeSpace -= copiedSize/BLOCK+1;
    }
    if(copiedSize > 10 *BLOCK){
        neededBlocks++;
        info.freeSpace--;
    }

    if(j < neededBlocks){
        printf("There is not enough space for '%s'\n", filename);
        return -1;
    }

    info.files++;
    i=0;
    inodes[freeInode].size = copiedSize;
    strcpy(inodes[freeInode].name, filename);

    iMask[freeInode] = 1;

    for(i=0;i < neededBlocks; i++)
        dMask[freeBlocks[i]] = 1;

    if(fseek(fp, 0, 0)){
        printf("Error: can't seek\n");
        return -1;
    }

    if(copiedSize <= 10*BLOCK){
        for(i=0;i < neededBlocks;++i)
            inodes[freeInode].d_addr[i] = freeBlocks[i]*BLOCK + 6*BLOCK;
        inodes[freeInode].ind_addr = 0;
        i=0;
         do{
            if(!fread(buff, sizeof(char), BLOCK, fp)){
                printf("Error: can't read from disk\n");
                return -1;
            }
            if(fseek(disk, inodes[freeInode].d_addr[i], 0)){
                printf("Error: can't seek\n");
                return -1;
            }
            if(!fwrite(buff, sizeof(char), BLOCK, disk)){
                printf("Error: can't write to disk\n");
                return -1;
            }
            ++i;
            neededBlocks--;
        }while(neededBlocks>0);
    }
    else{
        for(i=0;i < 10;++i){
            inodes[freeInode].d_addr[i] = freeBlocks[i]*BLOCK + 6*BLOCK;
        }
        inodes[freeInode].ind_addr = freeBlocks[i]*BLOCK + 6*BLOCK;
        if(fseek(disk, inodes[freeInode].ind_addr, 0)){
            printf("Error: can't seek\n");
            return -1;
        }
        ++i;
        freeIndAddr[dBlocks];
        for(j=0;j < neededBlocks-i;j++){
            freeIndAddr[j] = freeBlocks[i+j]*BLOCK + 6*BLOCK;
        }
        if(!fwrite(freeIndAddr, sizeof(freeIndAddr[0]), neededBlocks-i, disk)){
            printf("Error: can't write to disk\n");
            return -1;
        }

        for(i=0;i < 10;i++){
            if(!fread(buff, sizeof(char), BLOCK, fp)){
                printf("Error: can't read from disk\n");
                return -1;
            }
            if(fseek(disk, inodes[freeInode].d_addr[i], 0)){
                printf("Error: can't seek\n");
                return -1;
            }
            if(!fwrite(buff, sizeof(char), BLOCK, disk)){
                printf("Error: can't write to disk\n");
                return -1;
            }
            neededBlocks--;
        }
        neededBlocks--;
        if(fseek(disk, inodes[freeInode].ind_addr, 0)){
            printf("Error: can't seek\n");
            return -1;
        }
        i=0;
        for(neededBlocks;neededBlocks > 0;neededBlocks--){
            if(!fread(buff, sizeof(char), BLOCK, fp)){
                printf("Error: can't read from disk\n");
                return -1;
            }
            if(fseek(disk, freeIndAddr[i], 0)){
                printf("Error: can't seek\n");
                return -1;
            }
            if(!fwrite(buff, sizeof(char), BLOCK, disk)){
                printf("Error: can't write to disk\n");
                return -1;
            }
            ++i;
        }
    }

    if(fseek(disk, 0, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fwrite(&info, sizeof(info), 1, disk)){
        printf("Error: can't write to disk\n");
        return -1;
    }
    if(fseek(disk, BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fwrite(iMask, sizeof(char), numOfInodes, disk)){
        printf("Error: can't write to disk\n");
        return -1;
    }
    if(fseek(disk, 2*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fwrite(dMask, sizeof(char), dBlocks, disk)){
        printf("Error: can't write to disk\n");
        return -1;
    }
    if(fseek(disk, 3*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fwrite(inodes, sizeof(inodes[0]), numOfInodes, disk)){
        printf("Error: can't write to disk\n");
        return -1;
    }

    if(fclose(disk)){
        printf("Error: can't close the disk\n");
        return -1;
    }

    if(fclose(fp)){
        printf("Error: can't close the file\n");
        return -1;
    }

    printf("File added to virtual disk\n");

    return 0;
}

int copyFromDisk(char* filename){
    FILE *fp, *disk;
    int i=0;
    int fileInode;
    char buff[BLOCK];
    int ind_Addresses[BLOCK];
    int busyBlocks;
    int numOfInodes;
    int bToRead;
    int dBlocks;
    char iMask[BLOCK*IBLOCKS];
    char dMask[BLOCK];

    printf("Copying %s from disk...\n", filename);

    fp = fopen(filename, "r");
    if(fp){
        printf("File with this name already exists\n");
        return -1;
    }

    disk = fopen("virtualDisk", "r+");

    if(!disk){
        printf("Virtual disk doesn't exist\n");
        return -1;
    }

    if(!fread(&info, sizeof(info), 1, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }

    numOfInodes = info.iBlocks*BLOCK/sizeof(struct Inode)-1;
    dBlocks = info.size - 3 - info.iBlocks;

    if(fseek(disk, BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(iMask, sizeof(char), numOfInodes, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }
    if(fseek(disk, 2*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(dMask, sizeof(char), dBlocks, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }
    if(fseek(disk, 3*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fread(inodes, sizeof(struct Inode), numOfInodes, disk)){
        printf("Error: can't read from disk\n");
        return -1;
    }

    for(i=0;i < numOfInodes;i++){
        if(iMask[i] == 1 && strcmp(filename, inodes[i].name) == 0)
            break;
    }

       if(i == numOfInodes){
        printf("There is no such a file on disk\n");
        return -1;
    }

    fp = fopen(filename, "w");
    if(!fp){
        printf("Error: can't open a file\n");
        return -1;
    }

    fileInode = i;

    if(inodes[fileInode].size%BLOCK == 0)
        busyBlocks = inodes[fileInode].size/BLOCK;
    else
        busyBlocks = inodes[fileInode].size/BLOCK + 1;
    if(inodes[fileInode].size > 10*BLOCK)
        busyBlocks++;

    if(busyBlocks <= 10){
        for(i=0;i < busyBlocks-1;i++){
            if(fseek(disk, inodes[fileInode].d_addr[i], 0)){
                printf("Error: can't seek\n");
                return -1;
            }
            if(!fread(buff, sizeof(char), BLOCK, disk)){
                printf("Error: can't read from disk\n");
                return -1;
            }
            if(!fwrite(buff, sizeof(char), BLOCK, fp)){
                printf("Error: can't write to disk\n");
                return -1;
            }
        }
        if(fseek(disk, inodes[fileInode].d_addr[i], 0)){
            printf("Error: can't seek\n");
            return -1;
        }
        if(!fread(buff, sizeof(char), inodes[fileInode].size-(busyBlocks-1)*1024, disk)){
            printf("Error: can't read from disk\n");
            return -1;
        }
        if(!fwrite(buff, sizeof(char), inodes[fileInode].size-(busyBlocks-1)*1024, fp)){
            printf("Error: can't write to disk\n");
            return -1;
        }
    }
    else{
        for(i=0;i < 10;i++){
            if(fseek(disk, inodes[fileInode].d_addr[i], 0)){
                printf("Error: can't seek\n");
                return -1;
            }
            if(!fread(buff, sizeof(char), BLOCK, disk)){
                printf("Error: can't read from disk\n");
                return -1;
            }
            if(!fwrite(buff, sizeof(char), BLOCK, fp)){
                printf("Error: can't write to disk\n");
                return -1;
            }
        }
        bToRead = busyBlocks - 11;
        if(fseek(disk, inodes[fileInode].ind_addr, 0)){
            printf("Error: can't seek\n");
            return -1;
        }
        if(!fread(ind_Addresses, sizeof(int), bToRead, disk)){
            printf("Error: can't read from disk\n");
            return -1;
        }
        for(i=0;i < bToRead-1;i++){
            if(fseek(disk, ind_Addresses[i], 0)){
                printf("Error: can't seek\n");
                return -1;
            }
            if(!fread(buff, sizeof(char), BLOCK, disk)){
                printf("Error: can't read from disk\n");
                return -1;
            }
            if(!fwrite(buff, sizeof(char), BLOCK, fp)){
                printf("Error: can't write to disk\n");
                return -1;
            }
        }
        if(fseek(disk, ind_Addresses[i], 0)){
            printf("Error: can't seek\n");
            return -1;
        }
        if(!fread(buff, sizeof(char), inodes[fileInode].size-(busyBlocks-2)*1024, disk)){
            printf("Error: can't read from disk\n");
            return -1;
        }
        if(!fwrite(buff, sizeof(char), inodes[fileInode].size-(busyBlocks-2)*1024, fp)){
            printf("Error: can't write to disk\n");
            return -1;
        }
    }

    if(fclose(disk)){
        printf("Error: can't close the disk\n");
        return -1;
    }

    if(fclose(fp)){
        printf("Error: can't close the file\n");
        return -1;
    }

    printf("File copied from virtual disk\n");

    return 0;
}

int deleteDisk( void ){

    if(!unlink("virtualDisk"))
        printf("Disk deleted\n");
    else
        printf("Virtual disk doesn't exist\n");

    return 0;
}

int createDisk(int size) {
    FILE *disk;
    int i;
    int dBlocks = size*1024/BLOCK-IBLOCKS-3;
    int numOfInodes = IBLOCKS*BLOCK/sizeof(struct Inode)-1;
    char iMask[BLOCK*IBLOCKS];
    char dMask[BLOCK];
    char sign;

    if(size < 7){
        printf("Too small disk size\n");
        return -1;
    }
    if(size > 1030){
        printf("Too big disk size\n");
        return -1;
    }

    disk = fopen("virtualDisk", "wb+");
    if(!disk){
        printf("Error: can't create disk\n");
        return -1;
    }

    info.size = size;
    info.files = 0;
    info.freeSpace = dBlocks;
    info.iAddr = 3*BLOCK;
    info.iBlocks = IBLOCKS;

    for(i=0;i < numOfInodes;++i)
        iMask[i] = 0;
    for(i=0;i < dBlocks;++i)
        dMask[i] = 0;

    if(!fwrite(&info, sizeof(info), 1, disk)){
        printf("Error: can't write to disk\n");
        return -1;
    }
    if(fseek(disk, BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fwrite(iMask, sizeof(char), numOfInodes, disk)){
        printf("Error: can't write to disk\n");
        return -1;
    }
    if(fseek(disk, 2*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fwrite(dMask, sizeof(char), dBlocks, disk)){
        printf("Error: can't write to disk\n");
        return -1;
    }
    if(fseek(disk, 3*BLOCK, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    if(!fwrite(inodes, sizeof(inodes[0]), numOfInodes, disk)){
        printf("Error: can't write to disk\n");
        return -1;
    }

    if(fseek(disk, size*1024-1, 0)){
        printf("Error: can't seek\n");
        return -1;
    }
    sign = 'q';
    if(!fwrite(&sign, sizeof(char), 1, disk)){
        printf("Error: can't write to disk\n");
        return -1;
    }

    if(fclose(disk)){
        printf("Error: can't close the disk\n");
        return -1;
    }

    printf("Virtual disk created (%d kB)\n", size);

    return 0;
}


int main(int argc, char* argv[]){
    if(argc < 2){
        printf("Give argument\n");
        return -1;
    }

    switch(argv[1][0]){
        case '1':
            if(argc>2 && atoi(argv[2])!=0)
                return createDisk(atoi(argv[2]));
            else
                printf("Give a number of kB\n");
            break;
        case '2':
            if(argc>2)
                return copyToDisk(argv[2]);
            else
                printf("Give file name\n");
            break;
        case '3':
            if(argc>2)
                return copyFromDisk(argv[2]);
            else
                printf("Give file name\n");
            break;
        case '4':
            return ls();
            break;
        case '5':
            if(argc>2)
                return removeFile(argv[2]);
            else
                printf("Give file name\n");
            break;
        case '6':
            return deleteDisk();
            break;
        case '7':
            return seeDiskMap();
            break;
        default:
            break;
    }

    return 0;
}

