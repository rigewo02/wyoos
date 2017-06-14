#ifndef __MYOS__HANDLER__H
#define __MYOS__HANDLER__H

#include <drivers/mouse.h>
#include <drivers/keyboard.h>
#include <common/types.h>
#include <net/tcp.h>

using namespace myos;
using namespace myos::common;
using namespace myos::drivers;
using namespace myos::net;

namespace myos {

    class MouseToConsole : public MouseEventHandler {
    public:
        int8_t x, y;

        MouseToConsole();
        virtual void OnMouseMove(int xoffset, int yoffset);
    };

    class PrintfKeyboardEventHandler : public KeyboardEventHandler
    {
    public:
        void OnKeyDown(char c);
    };

    class PrintfTCPHandler : public TransmissionControlProtocolHandler
{
public:
    bool HandleTransmissionControlProtocolMessage(TransmissionControlProtocolSocket* socket,
         common::uint8_t* data, common::uint16_t size);
};

}

#endif
