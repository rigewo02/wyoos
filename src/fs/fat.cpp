#include <fs/fat.h>
#include <monitor.h>
#include <util.h>
#include <memorymanagement.h>

using namespace myos;
using namespace myos::common;
using namespace myos::fs;
using namespace myos::drivers;

myos::fs::FileAllocationTable32::FileAllocationTable32(AdvancedTechnologyAttachment *hd, uint32_t partitionOffset) {

    hd->Read28(partitionOffset, (uint8_t*)&bpb, sizeof(BiosParameterBlock32));

    fatStart = partitionOffset + bpb.reservedSectors;
    sectorsPerCluster = bpb.sectorsPerCluster;
    fatSize = bpb.tableSize;
    dataStart = fatStart + fatSize*bpb.fatCopies;
    rootStart = dataStart + sectorsPerCluster * (bpb.rootCluster - 2);
    hd->Read28(rootStart, (uint8_t*)&dirent, 16*sizeof(DirectoryEntry32));
}

void* myos::fs::FileAllocationTable32::Read(const char* fileName, uint32_t* bufferSize)
{

    for (int i = 0; i < 16; i++) {
        if (dirent[i].name[0] == 0) {
            break;
        }

        if ((dirent[i].attributes & 0x0F) == 0x0F) {
            continue;
        }

        if ((dirent[i].attributes & 0x10) == 0x10) //if the attribute has 5th bit set -> directory
            continue;


        if (!strcmp(fileName, (const char*)dirent[i].name, true)) {
            printf(fileName);
            printf("(");
            for (int j = 0; j < 8; j++) {
                printfHex(fileName[j]);
            }
            printf(")");
            printf(" is ");
            printf((const char*)dirent[i].name);
            printf("(");
            for (int j = 0; j < 8; j++) {
                printfHex(dirent[i].name[j]);
            }
            printf(")\n");
            continue;
        }


        printf("Reading file ");
        printf((const char*) dirent[i].name);
        printf("\n");

        uint32_t firstFileCluster = ((uint32_t)dirent[i].firstClusterHi << 16)
                            | ((uint32_t)dirent[i].firstClusterLow);

        int32_t SIZE = 512;//dirent[i].size;
        *bufferSize = SIZE;
        int32_t nextFileCluster = firstFileCluster;

        uint8_t fatbuffer[513];

        //uint8_t* returnBuffer = new uint8_t[SIZE];
        uint8_t returnBuffer[512];
        //loop through clusters of file
        while(SIZE > 0) {
            uint32_t fileSector = dataStart + sectorsPerCluster * (nextFileCluster - 2);
            int sectorOffset = 0;

            //loop through sectors of current cluster
            for ( ; SIZE > 0; SIZE -= 512) {

                //read the current sector in the buffer
                hd->Read28(fileSector + sectorOffset, returnBuffer + sizeof(returnBuffer) - SIZE, 512);

                //current iteration exceeded current cluster, get next cluster
                if (++sectorOffset > sectorsPerCluster) {
                    break;
                }
            }

            //which sector is the table entry for the current fatbufferfatbuffercluster in? read it!
            uint32_t sectorForCurrentCluster = nextFileCluster / (512/sizeof(uint32_t));
            hd->Read28(fatStart + sectorForCurrentCluster, fatbuffer, 512);

            //TODO: Check if the table entry for the next cluster already is in the fatbuffer

            //at which offset?
            uint32_t fatOffsetInSectorForCurrentCluster = nextFileCluster % (512/sizeof(uint32_t));
            nextFileCluster = ((uint32_t*)&fatbuffer)[fatOffsetInSectorForCurrentCluster];
        }
        return returnBuffer;
    }
}
