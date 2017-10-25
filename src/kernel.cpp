
#include <common/types.h>
#include <gdt.h>
#include <memorymanagement.h>
#include <hardwarecommunication/interrupts.h>
#include <syscalls.h>
#include <hardwarecommunication/pci.h>
#include <drivers/driver.h>
#include <drivers/keyboard.h>
#include <drivers/mouse.h>
#include <drivers/vga.h>
#include <drivers/ata.h>
#include <gui/desktop.h>
#include <gui/window.h>
#include <multitasking.h>

#include <drivers/amd_am79c973.h>
#include <net/etherframe.h>
#include <net/arp.h>
#include <net/ipv4.h>
#include <net/icmp.h>
#include <net/udp.h>
#include <net/tcp.h>

#include <fs/dospart.h>

#include <handler.h>
#include <list.h>


//#define GRAPHICSMODE
//#define NETWORK

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::hardwarecommunication;
using namespace myos::gui;
using namespace myos::net;
using namespace myos::fs;

void printf(const char* str);
void printfHex(uint8_t key);
void printfHex16(uint16_t key);
void printfHex32(uint32_t key);





void sysprintf(char* str)
{
    asm("int $0x80" : : "a" (4), "b" (str));
}

void taskA()
{
    while(true)
        sysprintf("A");
}

void taskB()
{
    while(true)
        sysprintf("B");
}

typedef void (*constructor)();
extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors()
{
    for(constructor* i = &start_ctors; i != &end_ctors; i++)
        (*i)();
}

extern "C" void kernelMain(const void* multiboot_structure, uint32_t /*multiboot_magic*/)
{
    printf("Hello World! --- http://www.AlgorithMan.de\n");

    GlobalDescriptorTable gdt;


    uint32_t* memupper = (uint32_t*)(((size_t)multiboot_structure) + 8);
    size_t heap = 10*1024*1024;
    MemoryManager memoryManager(heap, (*memupper)*1024 - heap - 10*1024);

    TaskManager taskManager;

    InterruptManager interrupts(0x20, &gdt, &taskManager);
    SyscallHandler syscalls(&interrupts, 0x80);

    printf("Initializing Hardware, Stage 1\n");

    #ifdef GRAPHICSMODE
        Desktop desktop(320,200, 0x00,0x00,0xA8);
    #endif

    DriverManager drvManager;

        #ifdef GRAPHICSMODE
            KeyboardDriver keyboard(&interrupts, &desktop);
        #else
            PrintfKeyboardEventHandler kbhandler;
            KeyboardDriver keyboard(&interrupts, &kbhandler);
        #endif
        drvManager.AddDriver(&keyboard);


        #ifdef GRAPHICSMODE
            MouseDriver mouse(&interrupts, &desktop);
        #else
            MouseToConsole mousehandler;
            MouseDriver mouse(&interrupts, &mousehandler);
        #endif
        drvManager.AddDriver(&mouse);

        PeripheralComponentInterconnectController PCIController;
        PCIController.SelectDrivers(&drvManager, &interrupts);

        #ifdef GRAPHICSMODE
            VideoGraphicsArray vga;
        #endif

    printf("Initializing Hardware, Stage 2\n");
        drvManager.ActivateAll();

    printf("Initializing Hardware, Stage 3\n");

    #ifdef GRAPHICSMODE
        vga.SetMode(320,200,8);
        Window win1(&desktop, 10,10,20,20, 0xA8,0x00,0x00);
        desktop.AddChild(&win1);
        Window win2(&desktop, 40,15,30,30, 0x00,0xA8,0x00);
        desktop.AddChild(&win2);
    #endif

    uint16_t ataprt[] = {0x1F0, 0x170, 0x1E8, 0x168};
    LinkedList<AdvancedTechnologyAttachment*> atas;

    AdvancedTechnologyAttachment* ata;

    for (int i = 0; i < sizeof(ataprt) / sizeof(ataprt[0]); i++) {
        ata = new AdvancedTechnologyAttachment(true, ataprt[i]);
        if (ata->Identify()) {
            atas.add(ata);
            printf("S-ATA master ");
            printfHex(i);
            printf("\n");
            MSDOSPartitionTable::ReadPartitions(ata);
        }

        ata = new AdvancedTechnologyAttachment(false, ataprt[i]);
        if (ata->Identify()) {
            atas.add(ata);
            printf("S-ATA slave ");
            printfHex(i);
            printf("\n");
            MSDOSPartitionTable::ReadPartitions(ata);
        }
    }
    printf("Total number of ATAs: ");
    printfHex(atas.size());
    printf("\n");

/*
    Task task1(&gdt, taskA);
    taskManager.AddTask(&task1);
    Task task2(&gdt, taskB);
    taskManager.AddTask(&task2);
*/

  #ifdef NETWORK
        initializeNetwork(&drvManager, &interrupts);
    #else
        interrupts.Activate();
    #endif


    while(1)
    {
        #ifdef GRAPHICSMODE
            desktop.Draw(&vga);
        #endif
    }
}

void initializeNetwork(DriverManager* drvManager, InterruptManager* interrupts) {
        amd_am79c973* eth0 = (amd_am79c973*)(drvManager->drivers[2]);


        // IP Address
        uint8_t ip1 = 10, ip2 = 0, ip3 = 2, ip4 = 15;
        uint32_t ip_be = ((uint32_t)ip4 << 24)
                    | ((uint32_t)ip3 << 16)
                    | ((uint32_t)ip2 << 8)
                    | (uint32_t)ip1;
        eth0->SetIPAddress(ip_be);
        EtherFrameProvider etherframe(eth0);
        AddressResolutionProtocol arp(&etherframe);


        // IP Address of the default gateway
        uint8_t gip1 = 10, gip2 = 0, gip3 = 2, gip4 = 2;
        uint32_t gip_be = ((uint32_t)gip4 << 24)
                       | ((uint32_t)gip3 << 16)
                       | ((uint32_t)gip2 << 8)
                       | (uint32_t)gip1;

        uint8_t subnet1 = 255, subnet2 = 255, subnet3 = 255, subnet4 = 0;
        uint32_t subnet_be = ((uint32_t)subnet4 << 24)
                       | ((uint32_t)subnet3 << 16)
                       | ((uint32_t)subnet2 << 8)
                       | (uint32_t)subnet1;

        InternetProtocolProvider ipv4(&etherframe, &arp, gip_be, subnet_be);
        InternetControlMessageProtocol icmp(&ipv4);
        UserDatagramProtocolProvider udp(&ipv4);
        TransmissionControlProtocolProvider tcp(&ipv4);

        printf("\n\n\n\n");
        interrupts->Activate();


        arp.BroadcastMACAddress(gip_be);


        PrintfTCPHandler tcphandler;
        TransmissionControlProtocolSocket* tcpsocket = tcp.Listen(1234);
        tcp.Bind(tcpsocket, &tcphandler);
        //tcpsocket->Send((uint8_t*)"Hello TCP!", 10);


        //icmp.RequestEchoReply(gip_be);

        //PrintfUDPHandler udphandler;
        //UserDatagramProtocolSocket* udpsocket = udp.Connect(gip_be, 1234);
        //udp.Bind(udpsocket, &udphandler);
        //udpsocket->Send((uint8_t*)"Hello UDP!", 10);

        //UserDatagramProtocolSocket* udpsocket = udp.Listen(1234);
        //udp.Bind(udpsocket, &udphandler);
}
