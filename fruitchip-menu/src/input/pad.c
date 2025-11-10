#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <kernel.h>
#include <libpad.h>

#include "pad.h"

static unsigned char padBuffer[2][256] ALIGNED(64);
static unsigned int prevInputs[2] = {0, 0};

static int init = 0;

// Polls the gamepad and returns currently pressed buttons
static int pollPad(int port, int slot)
{
  struct padButtonStatus buttons;
  if (padRead(port, slot, &buttons) != 0)
  {
    prevInputs[port] = 0xffff ^ buttons.btns;
    return prevInputs[port];
  }

  return 0;
}

// Initializes gamepad input driver
void input_pad_start()
{
    padInit(0);
    padPortOpen(0, 0, padBuffer[0]);
    padPortOpen(1, 0, padBuffer[1]);

    prevInputs[0] = 0;
    prevInputs[1] = 0;
}

bool input_pad_is_ready()
{
    int port0 = padGetState(0, 0);
    int port1 = padGetState(1, 0);

    // nothing to wait for if no controllers are connected
    if (port0 == PAD_STATE_DISCONN && port1 == PAD_STATE_DISCONN)
        return true;

    return port0 == PAD_STATE_STABLE || port1 == PAD_STATE_STABLE;
}

int input_pad_poll()
{
    if (!input_pad_is_ready()) return 0;

    return pollPad(0, 0) | pollPad(1, 0);
}

// Closes gamepad gamepad input driver
void input_pad_stop()
{
    padPortClose(0, 0);
    padPortClose(1, 0);
    padEnd();
    init = 0;
}
