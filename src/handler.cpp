#include <handler.h>
#include <monitor.h>

void PrintfKeyboardEventHandler::OnKeyDown(char c)
{
    char* foo = " ";
    foo[0] = c;
    printf(foo);
}



MouseToConsole::MouseToConsole()
{
    uint16_t* VideoMemory = (uint16_t*)0xb8000;
    x = 40;
    y = 12;
    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                        | (VideoMemory[80*y+x] & 0xF000) >> 4
                        | (VideoMemory[80*y+x] & 0x00FF);
}

void MouseToConsole::OnMouseMove(int xoffset, int yoffset)
{
    static uint16_t* VideoMemory = (uint16_t*)0xb8000;
    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                        | (VideoMemory[80*y+x] & 0xF000) >> 4
                        | (VideoMemory[80*y+x] & 0x00FF);

    x += xoffset;
    if(x >= 80) x = 79;
    if(x < 0) x = 0;
    y += yoffset;
    if(y >= 25) y = 24;
    if(y < 0) y = 0;

    VideoMemory[80*y+x] = (VideoMemory[80*y+x] & 0x0F00) << 4
                        | (VideoMemory[80*y+x] & 0xF000) >> 4
                        | (VideoMemory[80*y+x] & 0x00FF);
}

bool PrintfTCPHandler::HandleTransmissionControlProtocolMessage(TransmissionControlProtocolSocket* socket,
     common::uint8_t* data, common::uint16_t size)
{
    char* foo = " ";
    for(int i = 0; i < size; i++)
    {
        foo[0] = data[i];
        printf(foo);
    }



    if(size > 9
        && data[0] == 'G'
        && data[1] == 'E'
        && data[2] == 'T'
        && data[3] == ' '
        && data[4] == '/'
        && data[5] == ' '
        && data[6] == 'H'
        && data[7] == 'T'
        && data[8] == 'T'
        && data[9] == 'P'
    )
    {
        socket->Send((uint8_t*)"HTTP/1.1 200 OK\r\nServer: MyOS\r\nContent-Type: text/html\r\n\r\n<html><head><title>My Operating System</title></head><body><b>My Operating System</b> http://www.AlgorithMan.de</body></html>\r\n",184);
        socket->Disconnect();
    }


    return true;
    }
