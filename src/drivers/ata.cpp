#include <drivers/ata.h>
#include <monitor.h>

#define LOGACTION 0

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;


AdvancedTechnologyAttachment::AdvancedTechnologyAttachment(bool master, common::uint16_t portBase)
:   dataPort(portBase),
    errorPort(portBase + 0x1),
    sectorCountPort(portBase + 0x2),
    lbaLowPort(portBase + 0x3),
    lbaMidPort(portBase + 0x4),
    lbaHiPort(portBase + 0x5),
    devicePort(portBase + 0x6),
    commandPort(portBase + 0x7),
    controlPort(portBase + 0x206)
{
    this->master = master;
}

AdvancedTechnologyAttachment::~AdvancedTechnologyAttachment()
{
}

void AdvancedTechnologyAttachment::Identify()
{
    devicePort.Write(master ? 0xA0 : 0xB0);
    controlPort.Write(0);

    devicePort.Write(0xA0);
    uint8_t status = commandPort.Read();
    if(status == 0xFF)
        return;


    devicePort.Write(master ? 0xA0 : 0xB0);
    sectorCountPort.Write(0);
    lbaLowPort.Write(0);
    lbaMidPort.Write(0);
    lbaHiPort.Write(0);
    commandPort.Write(0xEC); // identify command


    status = commandPort.Read();
    if(status == 0x00)
        return;

    while(((status & 0x80) == 0x80)
       && ((status & 0x01) != 0x01))
        status = commandPort.Read();

    if(status & 0x01)
    {
        printf("ERROR Identifying");
        return;
    }

    for(int i = 0; i < 256; i++)
    {
        uint16_t data = dataPort.Read();
        char *text = "  \0";
        text[0] = (data >> 8) & 0xFF;
        text[1] = data & 0xFF;
        //printf(text);
    }
    //printf("\n");
}

void AdvancedTechnologyAttachment::Read28(common::uint32_t sectorNum, uint8_t* buffer, int count)
{
    if(sectorNum > 0x0FFFFFFF)
        return;

    devicePort.Write( (master ? 0xE0 : 0xF0) | ((sectorNum & 0x0F000000) >> 24) );
    errorPort.Write(0);
    sectorCountPort.Write(1);
    lbaLowPort.Write(  sectorNum & 0x000000FF );
    lbaMidPort.Write( (sectorNum & 0x0000FF00) >> 8);
    lbaHiPort.Write( (sectorNum & 0x00FF0000) >> 16 );
    commandPort.Write(0x20);

    uint8_t status = commandPort.Read();
    while(((status & 0x80) == 0x80)
       && ((status & 0x01) != 0x01))
        status = commandPort.Read();

    if(status & 0x01)
    {
        printf("ERROR Reading!\n");
        return;
    }

    #if LOGACTIOn
        printf("Reading ATA Drive: ");
        printfHex32(sectorNum);
        printf("\n");
    #endif

    for(int i = 0; i < count; i += 2)
    {
        uint16_t wdata = dataPort.Read();

        buffer[i] = wdata & 0xFF;

        if(i+1 < count)
            buffer[i+1] = (wdata >> 8) & 0xFF;
        else
            buffer[i+1] = '\0';
    }

    for(int i = count + (count%2); i < 512; i += 2)
        dataPort.Read();
}

void AdvancedTechnologyAttachment::Write28(common::uint32_t sectorNum, common::uint8_t* data, common::uint32_t count)
{
    if(sectorNum > 0x0FFFFFFF)
        return;
    if(count > 512)
        return;


    devicePort.Write( (master ? 0xE0 : 0xF0) | ((sectorNum & 0x0F000000) >> 24) );
    errorPort.Write(0);
    sectorCountPort.Write(1);
    lbaLowPort.Write(  sectorNum & 0x000000FF );
    lbaMidPort.Write( (sectorNum & 0x0000FF00) >> 8);
    lbaHiPort.Write( (sectorNum & 0x00FF0000) >> 16 );
    commandPort.Write(0x30);

    #if LOGACTIOn
        printf("Writing to ATA Drive: ");
    #endif

    for(int i = 0; i < count; i += 2)
    {
        uint16_t wdata = data[i];
        if(i+1 < count)
            wdata |= ((uint16_t)data[i+1]) << 8;
        dataPort.Write(wdata);

        char *text = "  \0";
        text[0] = (wdata >> 8) & 0xFF;
        text[1] = wdata & 0xFF;
        printf(text);
    }

    for(int i = count + (count%2); i < 512; i += 2)
        dataPort.Write(0x0000);

}

void AdvancedTechnologyAttachment::Flush()
{
    devicePort.Write( master ? 0xE0 : 0xF0 );
    commandPort.Write(0xE7);

    uint8_t status = commandPort.Read();
    if(status == 0x00)
        return;

    while(((status & 0x80) == 0x80)
       && ((status & 0x01) != 0x01))
        status = commandPort.Read();

    if(status & 0x01)
    {
        printf("ERROR");
        return;
    }
}
