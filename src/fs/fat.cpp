#include <fs/fat.h>
#include <monitor.h>

using namespace myos;
using namespace myos::common;
using namespace myos::fs;
using namespace myos::drivers;

myos::fs::FileAllocationTable32::FileAllocationTable32(AdvancedTechnologyAttachment *hd, uint32_t partitionOffset) {

    BiosParameterBlock32 bpb;
    hd->Read28(partitionOffset, (uint8_t*)&bpb, sizeof(BiosParameterBlock32));

    fatStart = partitionOffset + bpb.reservedSectors;

    uint32_t fatSize = bpb.tableSize;
    uint32_t dataStart = fatStart + fatSize*bpb.fatCopies;

    uint32_t rootStart = dataStart + bpb.sectorsPerCluster*(bpb.rootCluster - 2);

    DirectoryEntry32 dirent[16];
    hd->Read28(rootStart, (uint8_t*)&dirent, 16*sizeof(DirectoryEntry32));

    for (int i = 0; i < 16; i++) {
        if (dirent[i].name[0] == 0) {
            break;
        }

        if ((dirent[i].attributes & 0x0F) == 0x0F) {
            continue;
        }

        char* foo = "        ";

        for (int j = 0; j < 8; j++)
            foo[j] = dirent[i].name[j];

        printf(foo);

        if ((dirent[i].attributes & 0x10) == 0x10) //if the attribute has 5th bit set -> directory
            continue;

        uint32_t firstFileCluster = ((uint32_t)dirent[i].firstClusterHi << 16)
                            | ((uint32_t)dirent[i].firstClusterLow);

        int32_t SIZE = dirent[i].size;
        int32_t nextFileCluster = firstFileCluster;

        uint8_t buffer[513];
        uint8_t fatbuffer[513];

        //loop through clusters of file
        while(SIZE > 0) {
            uint32_t fileSector = dataStart + bpb.sectorsPerCluster * (nextFileCluster - 2);
            int sectorOffset = 0;

            //loop through sectors of current cluster
            for ( ; SIZE > 0; SIZE -= 512) {

                //read the current sector in the buffer
                hd->Read28(fileSector + sectorOffset, buffer, 512);

                //current iteration exceeded current cluster, get next cluster
                if (++sectorOffset > bpb.sectorsPerCluster) {
                    break;
                }

                //after last character, put a zero and printf it
                buffer[dirent[i].size > 512 ? 512 : dirent[i].size] = '\0';
                printf((char*)buffer);
            }

            //which sector is the table entry for the current cluster in? read it!
            uint32_t sectorForCurrentCluster = nextFileCluster / (512/sizeof(uint32_t));
            hd->Read28(fatStart + sectorForCurrentCluster, fatbuffer, 512);

            //TODO: Check if the table entry for the next cluster already is in the fatbuffer

            //at which offset?
            uint32_t fatOffsetInSectorForCurrentCluster = nextFileCluster % (512/sizeof(uint32_t));
            nextFileCluster = ((uint32_t*)&fatbuffer)[fatOffsetInSectorForCurrentCluster];
        }
    }
}
