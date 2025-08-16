#ifndef __UTILS_H__
#define __UTILS_H__

#include "hardware/pio.h"

inline static void pio_interrupt_set(PIO pio, uint pio_interrupt_num) {
    check_pio_param(pio);
    invalid_params_if(HARDWARE_PIO, pio_interrupt_num >= 8);
    pio->irq_force = (1u << pio_interrupt_num);
}

// inline static void pio_sm_wait_for_tx_fifo_empty(PIO pio, uint sm) {
//     while (!pio_sm_is_tx_fifo_empty(pio, sm)) tight_loop_contents();
//     __compiler_memory_barrier();
// }

inline static bool pio_sm_is_tx_fifo_stalled(PIO pio, uint sm) {
    uint32_t fdebug_tx_stall = 1u << (PIO_FDEBUG_TXSTALL_LSB + sm);
    return (pio->fdebug & fdebug_tx_stall) != 0;
}

// inline static void pio_sm_wait_for_tx_stall(PIO pio, uint sm) {
//     uint32_t fdebug_tx_stall = 1u << (PIO_FDEBUG_TXSTALL_LSB + sm);
//     while (!(pio->fdebug & fdebug_tx_stall)) tight_loop_contents();
//     __compiler_memory_barrier();
// }

#endif /* __UTILS_H__ */
