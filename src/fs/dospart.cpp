#include <fs/dospart.h>
#include <monitor.h>
#include <fs/fat.h>

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::fs;

void MSDOSPartitionTable::ReadPartitions(drivers::AdvancedTechnologyAttachment *hd) {

    MasterBootRecord mbr;
    hd->Read28(0, (uint8_t*)&mbr, sizeof(MasterBootRecord));

/*
    printf("MBR: ");
    for (int i = 446; i <= 447 + 4 * 16; i++) {
        printfHex(((uint8_t*)&mbr)[i]);
        printf(" ");
    }

    */
    printf("\n");

    if (mbr.magicNumber != 0xAA55) {
        printf("Illegal MBR!\n");
        return;
    }

    for (int i = 0; i < 4; i++) {
        printf("Partition ");
        printfHex(i & 0xFF);

        if (mbr.primaryPartitions[i].bootable == 0x80) {
            printf(" bootable type ");
        }
        else {
            printf(" unbootable type ");
        }

        printfHex(mbr.primaryPartitions[i].partition_id);
        printf("\n");

        if (mbr.primaryPartitions[i].partition_id != 0)
            FileAllocationTable32 fat32(hd, mbr.primaryPartitions[i].start_lba);
    }

}
