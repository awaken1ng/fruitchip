#ifndef __USLEEP_H__
#define __USLEEP_H__

#include <stdbool.h>

#include <tamtypes.h>

// taken from Crystal Chip firmware sources and converted to C

#ifdef _EE
inline static void cop0_cycle_count_clear() {
    asm volatile(
        "mtc0 $zero, $9\n" // EE_COP0_Count
        "sync.p\n"
    );
}

inline static u32 cop0_cycle_count_get() {
    u32 count;
    asm volatile(
        "mfc0 %0, $9\n" // EE_COP0_Count
        : "=r" (count)
    );
    return count;
}

void usleep(u32 us) {
    u32 wait_cycles = us * 300;

    cop0_cycle_count_clear();

    while (true) {
        u32 current_cycle_count = cop0_cycle_count_get();
        if (current_cycle_count >= wait_cycles)
            break;
    }
}
#endif // _EE

#ifdef _IOP
void usleep(volatile u32 us) {
    for (; us > 0; us--) {
        for (volatile u32 i = 16; i > 0; i--) {}
    }
}
#endif // _IOP

#endif // __USLEEP_H__
