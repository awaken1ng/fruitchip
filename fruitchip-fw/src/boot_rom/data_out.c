#include <stdio.h>

#include <boot_rom/data_out.h>

#define xconcat(X, Y) concat(X, Y)
#define concat(X, Y) X ## Y
#define reg(prefix, name, value) ((value << xconcat(prefix ## _, xconcat(name, _LSB))) & xconcat(prefix ## _, xconcat(name, _BITS)))
#define dma_ctrl_reg_field(name, value) reg(DMA_CH0_CTRL_TRIG, name, value)
#define dma_sniff_ctrl_reg_field(name, value) reg(DMA_SNIFF_CTRL, name, value)

static uint8_t __not_in_flash("zero") ZERO = 0x00;

static uint32_t __not_in_flash("status_code") data_out_status_code;

static uint32_t __not_in_flash("busy_code") data_out_busy_code = MODCHIP_CMD_RESULT_BUSY;

// region: control blocks
typedef struct control_block {
    const volatile void *read_addr;
    volatile void *write_addr;
    uint32_t trans_count;
    uint32_t ctrl;
} control_block_t;

const uint32_t DMA_CTRL_32 = (
    dma_ctrl_reg_field(IRQ_QUIET, true) |
    dma_ctrl_reg_field(DATA_SIZE, DMA_SIZE_32) |
    dma_ctrl_reg_field(EN, true)
);

const uint32_t DMA_CTRL_DATA_OUT = (
    dma_ctrl_reg_field(IRQ_QUIET, true) |
    dma_ctrl_reg_field(TREQ_SEL, PIO_DREQ_NUM(BOOT_ROM_PIO, BOOT_ROM_DATA_OUT_SM, true)) |
    dma_ctrl_reg_field(INCR_READ, true) |
    dma_ctrl_reg_field(DATA_SIZE, DMA_SIZE_8) |
    dma_ctrl_reg_field(HIGH_PRIORITY, true) |
    dma_ctrl_reg_field(EN, true)
);

static __not_in_flash("dma_sniff_setup") struct {
    uint32_t sniff_ctrl;
    uint32_t sniff_data;
} DMA_SNIFF_SETUP = {
    .sniff_ctrl = dma_sniff_ctrl_reg_field(CALC, 0) |
                  dma_sniff_ctrl_reg_field(DMACH, BOOT_ROM_DATA_OUT_DATA2_CHAN) |
                  dma_sniff_ctrl_reg_field(EN, true),
    .sniff_data = 0xFFFFFFFF,
};

#define CONTROL_BLOCK_SNIFFER_SETUP(channel) {                 \
    .ctrl = (                                                  \
        DMA_CTRL_32 |                                          \
        dma_ctrl_reg_field(TREQ_SEL, DREQ_FORCE) |             \
        dma_ctrl_reg_field(INCR_READ, true) |                  \
        dma_ctrl_reg_field(INCR_WRITE, true) |                 \
        dma_ctrl_reg_field(CHAIN_TO, channel)                  \
    ),                                                         \
    .read_addr = &DMA_SNIFF_SETUP,                             \
    .write_addr = &dma_hw->sniff_ctrl,                         \
    .trans_count = sizeof(DMA_SNIFF_SETUP) / sizeof(uint32_t), \
}

#define CONTROL_BLOCK_STATUS_CODE(channel) {        \
    .ctrl = (                                       \
        DMA_CTRL_DATA_OUT |                         \
        dma_ctrl_reg_field(CHAIN_TO, channel)       \
    ),                                              \
    .read_addr = &data_out_status_code,             \
    .write_addr = &BOOT_ROM_PIO->txf[BOOT_ROM_DATA_OUT_SM], \
    .trans_count = sizeof(data_out_status_code),    \
}

const control_block_t CONTROL_BLOCK_CRC = {
    .ctrl = (
        DMA_CTRL_DATA_OUT |
        dma_ctrl_reg_field(CHAIN_TO, BOOT_ROM_DATA_OUT_CTRL1_CHAN)
    ),
    .read_addr = &dma_hw->sniff_data,
    .write_addr = &BOOT_ROM_PIO->txf[BOOT_ROM_DATA_OUT_SM],
    .trans_count = sizeof(dma_hw->sniff_data)
};

// pad the data with a dummy byte.
// checking if TX FIFO is empty doesn't tell if data is on the bus or not,
// by adding one more byte to the transfer, when this byte is pulled from FIFO
// we know that the last byte of data was on the bus and it's safe to stop data out
const control_block_t CONTROL_BLOCK_EOT = {
    .ctrl = (
        DMA_CTRL_DATA_OUT |
        dma_ctrl_reg_field(CHAIN_TO, BOOT_ROM_DATA_OUT_CTRL1_CHAN)
    ),
    .read_addr = &ZERO,
    .write_addr = &BOOT_ROM_PIO->txf[BOOT_ROM_DATA_OUT_SM],
    .trans_count = sizeof(ZERO)
};

// null trigger to end chain
const control_block_t CONTROL_BLOCK_NULL = {
    .ctrl = 0,
    .read_addr = NULL,
    .write_addr = NULL,
    .trans_count = 0,
};

// endregion: control blocks

// region: control lists

static control_block_t __not_in_flash("control_blocks.with_status_with_data_with_crc") WITH_STATUS_WITH_DATA_WITH_CRC[] = {
    CONTROL_BLOCK_SNIFFER_SETUP(BOOT_ROM_DATA_OUT_CTRL1_CHAN),
    CONTROL_BLOCK_STATUS_CODE(BOOT_ROM_DATA_OUT_CTRL2_CHAN),
    // data out on second control block
    CONTROL_BLOCK_CRC,
    CONTROL_BLOCK_EOT,
    CONTROL_BLOCK_NULL,
};

static control_block_t __not_in_flash("control_blocks.no_status_with_data_with_crc") NO_STATUS_WITH_DATA_WITH_CRC[] = {
    CONTROL_BLOCK_SNIFFER_SETUP(BOOT_ROM_DATA_OUT_CTRL2_CHAN),
    // data out on second control block
    CONTROL_BLOCK_CRC,
    CONTROL_BLOCK_EOT,
    CONTROL_BLOCK_NULL,
};

static control_block_t __not_in_flash("control_blocks.no_status_with_data_no_crc") NO_STATUS_WITH_DATA_NO_CRC[] = {
    // data out on second control block
    CONTROL_BLOCK_EOT,
    CONTROL_BLOCK_NULL,
};

static control_block_t __not_in_flash("control_blocks.with_status_with_data_no_crc") WITH_STATUS_WITH_DATA_NO_CRC[] = {
    CONTROL_BLOCK_STATUS_CODE(BOOT_ROM_DATA_OUT_CTRL2_CHAN),
    // data out on second control block
    CONTROL_BLOCK_EOT,
    CONTROL_BLOCK_NULL,
};

static control_block_t __not_in_flash("control_blocks.with_status_no_data_no_crc") WITH_STATUS_NO_DATA_NO_CRC[] = {
    CONTROL_BLOCK_STATUS_CODE(BOOT_ROM_DATA_OUT_CTRL1_CHAN),
    CONTROL_BLOCK_EOT,
    CONTROL_BLOCK_NULL,
};

static control_block_t __not_in_flash("control_blocks.data_out") CONTROL_BLOCKS_DATA_OUT[] = {
    {
        .ctrl = (
            DMA_CTRL_DATA_OUT |
            dma_ctrl_reg_field(SNIFF_EN, true) |
            dma_ctrl_reg_field(CHAIN_TO, BOOT_ROM_DATA_OUT_CTRL1_CHAN)
        ),
        .read_addr = NULL, // set by start function
        .write_addr = &BOOT_ROM_PIO->txf[BOOT_ROM_DATA_OUT_SM],
        .trans_count = 0, // set by start function
    },
    CONTROL_BLOCK_NULL,
};

// endregion: control lists

void __isr __time_critical_func(byte_out_irq_handler)()
{
    if (!pio_sm_is_tx_fifo_empty(BOOT_ROM_PIO, BOOT_ROM_DATA_OUT_SM))
        goto exit;

    boot_rom_data_out_stop();
    boot_rom_sniffers_start();
    boot_rom_data_out_reset();

exit:
    pio_interrupt_clear(BOOT_ROM_PIO, BOOT_ROM_BYTE_OUT_IRQ);
}

void __time_critical_func(boot_rom_data_out_init)()
{
    gpio_pull_up(BOOT_ROM_Q0_PIN + 0);
    gpio_pull_up(BOOT_ROM_Q0_PIN + 1);
    gpio_pull_up(BOOT_ROM_Q0_PIN + 2);
    gpio_pull_up(BOOT_ROM_Q0_PIN + 3);
    gpio_pull_up(BOOT_ROM_Q0_PIN + 4);
    gpio_pull_up(BOOT_ROM_Q0_PIN + 5);
    gpio_pull_up(BOOT_ROM_Q0_PIN + 6);
    gpio_pull_up(BOOT_ROM_Q0_PIN + 7);
    gpio_pull_up(BOOT_ROM_CE_PIN);
    gpio_pull_up(BOOT_ROM_OE_PIN);

    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 0, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 1, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 2, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 3, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 4, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 5, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 6, GPIO_DRIVE_STRENGTH_12MA);
    gpio_set_drive_strength(BOOT_ROM_Q0_PIN + 7, GPIO_DRIVE_STRENGTH_12MA);

    // assign hardcoded SM indices to avoid runtime calculations of masks
    pio_sm_claim(BOOT_ROM_PIO, BOOT_ROM_READ_SNIFFER_SM);
    pio_sm_claim(BOOT_ROM_PIO, BOOT_ROM_WRITE_SNIFFER_SM);
    pio_sm_claim(BOOT_ROM_PIO, BOOT_ROM_DATA_OUT_SM);

    // load programs in specific order to match the statically calculated offsets
#ifdef PICO_DEFAULT_LED_PIN
    int offset = pio_add_program(BOOT_ROM_PIO, &boot_rom_read_sniffer_with_led_program);
#else
    int offset = pio_add_program(BOOT_ROM_PIO, &boot_rom_read_sniffer_program);
#endif
    boot_rom_read_sniffer_sm_init_and_start(BOOT_ROM_PIO, BOOT_ROM_READ_SNIFFER_SM, boot_rom_read_sniffer_offset);
    if (boot_rom_read_sniffer_offset != offset) panic("Read sniffer loaded at unexpected offset");

    offset = pio_add_program(BOOT_ROM_PIO, &boot_rom_write_sniffer_program);
    boot_rom_write_sniffer_sm_init_and_start(BOOT_ROM_PIO, BOOT_ROM_WRITE_SNIFFER_SM, boot_rom_write_sniffer_offset);
    if (boot_rom_write_sniffer_offset != offset) panic("Write sniffer loaded at unexpected offset");

#ifdef PICO_DEFAULT_LED_PIN
    offset = pio_add_program(BOOT_ROM_PIO, &boot_rom_data_out_with_led_program);
#else
    offset = pio_add_program(BOOT_ROM_PIO, &boot_rom_data_out_program);
#endif
    boot_rom_data_out_sm_init(BOOT_ROM_PIO, BOOT_ROM_DATA_OUT_SM, boot_rom_data_out_offset);
    if (boot_rom_data_out_offset != offset) panic("Data out loaded at unexpected offset");
}

void __time_critical_func(boot_rom_data_out_init_dma)()
{
    dma_channel_config dma_conf;
    dma_channel_claim(BOOT_ROM_DATA_OUT_CTRL1_CHAN);
    dma_conf = dma_channel_get_default_config(BOOT_ROM_DATA_OUT_CTRL1_CHAN);
    channel_config_set_transfer_data_size(&dma_conf, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_conf, true);
    channel_config_set_write_increment(&dma_conf, true);
    channel_config_set_ring(&dma_conf, true, 4); // 1 << 4
    dma_channel_set_write_addr(BOOT_ROM_DATA_OUT_CTRL1_CHAN, &dma_hw->ch[BOOT_ROM_DATA_OUT_DATA1_CHAN].read_addr, false);
    dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_CTRL1_CHAN, 4, false);
    dma_channel_set_config(BOOT_ROM_DATA_OUT_CTRL1_CHAN, &dma_conf, false);

    dma_channel_claim(BOOT_ROM_DATA_OUT_CTRL2_CHAN);
    channel_config_set_chain_to(&dma_conf, BOOT_ROM_DATA_OUT_CTRL2_CHAN);
    dma_channel_set_write_addr(BOOT_ROM_DATA_OUT_CTRL2_CHAN, &dma_hw->ch[BOOT_ROM_DATA_OUT_DATA2_CHAN].read_addr, false);
    dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_CTRL2_CHAN, 4, false);
    dma_channel_set_config(BOOT_ROM_DATA_OUT_CTRL2_CHAN, &dma_conf, false);

    dma_channel_claim(BOOT_ROM_DATA_OUT_DATA1_CHAN);
    dma_channel_claim(BOOT_ROM_DATA_OUT_DATA2_CHAN);

    dma_channel_config dma_tx_busy_ping_conf;
    dma_channel_claim(BOOT_ROM_DATA_OUT_BUSY_PING);
    dma_tx_busy_ping_conf = dma_channel_get_default_config(BOOT_ROM_DATA_OUT_BUSY_PING);
    channel_config_set_transfer_data_size(&dma_tx_busy_ping_conf, DMA_SIZE_8);
    channel_config_set_read_increment(&dma_tx_busy_ping_conf, true);
    channel_config_set_write_increment(&dma_tx_busy_ping_conf, false);
    channel_config_set_dreq(&dma_tx_busy_ping_conf, pio_get_dreq(BOOT_ROM_PIO, BOOT_ROM_DATA_OUT_SM, true));
    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_BUSY_PING, &data_out_busy_code, false);
    dma_channel_set_write_addr(BOOT_ROM_DATA_OUT_BUSY_PING, &BOOT_ROM_PIO->txf[BOOT_ROM_DATA_OUT_SM], false);
    dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_BUSY_PING, sizeof(data_out_busy_code), false);
    dma_channel_set_config(BOOT_ROM_DATA_OUT_BUSY_PING, &dma_tx_busy_ping_conf, false);

    static uint32_t *data_out_busy_code_ptr = &data_out_busy_code;
    dma_channel_config dma_tx_busy_pong_conf;
    dma_channel_claim(BOOT_ROM_DATA_OUT_BUSY_PONG);
    dma_tx_busy_pong_conf = dma_channel_get_default_config(BOOT_ROM_DATA_OUT_BUSY_PONG);
    channel_config_set_transfer_data_size(&dma_tx_busy_pong_conf, DMA_SIZE_32);
    channel_config_set_read_increment(&dma_tx_busy_pong_conf, false);
    channel_config_set_write_increment(&dma_tx_busy_pong_conf, false);
    channel_config_set_dreq(&dma_tx_busy_pong_conf, DREQ_FORCE);
    channel_config_set_chain_to(&dma_tx_busy_pong_conf, BOOT_ROM_DATA_OUT_BUSY_PING);
    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_BUSY_PONG, &data_out_busy_code_ptr, false);
    dma_channel_set_write_addr(BOOT_ROM_DATA_OUT_BUSY_PONG, &dma_channel_hw_addr(BOOT_ROM_DATA_OUT_BUSY_PING)->read_addr, false);
    dma_channel_set_transfer_count(BOOT_ROM_DATA_OUT_BUSY_PONG, 1, false);
    dma_channel_set_config(BOOT_ROM_DATA_OUT_BUSY_PONG, &dma_tx_busy_pong_conf, false);

    pio_set_irq0_source_enabled(BOOT_ROM_PIO, pis_interrupt0 + BOOT_ROM_BYTE_OUT_IRQ, true);
    irq_set_exclusive_handler(PIO0_IRQ_0, byte_out_irq_handler);
    irq_set_enabled(PIO0_IRQ_0, true);
}

__force_inline void __time_critical_func(boot_rom_data_out_set_data)(const volatile void *read_addr, uint32_t encoded_transfer_count)
{
    CONTROL_BLOCKS_DATA_OUT[0].read_addr = read_addr;
    CONTROL_BLOCKS_DATA_OUT[0].trans_count = encoded_transfer_count;
    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_CTRL2_CHAN, &CONTROL_BLOCKS_DATA_OUT[0], false);
}

__force_inline void __time_critical_func(boot_rom_data_out_start_data_without_status_code)(bool crc)
{
    boot_rom_sniffers_stop();

    if (crc)
    {
        dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_CTRL1_CHAN, NO_STATUS_WITH_DATA_WITH_CRC, true);
    }
    else
    {
        dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_CTRL1_CHAN, NO_STATUS_WITH_DATA_NO_CRC, false);
        dma_channel_start(BOOT_ROM_DATA_OUT_CTRL2_CHAN);
    }

    boot_rom_data_out_start();
    boot_rom_sniffers_reset();
}

__force_inline void __time_critical_func(boot_rom_data_out_start_data_with_status_code)(uint32_t status_code, const volatile void *read_addr, uint32_t encoded_transfer_count, bool crc)
{
    boot_rom_sniffers_stop();

    data_out_status_code = status_code;
    boot_rom_data_out_set_data(read_addr, encoded_transfer_count);

    const volatile void *control_blocks = crc
        ? &WITH_STATUS_WITH_DATA_WITH_CRC[0]
        : &WITH_STATUS_WITH_DATA_NO_CRC[0];

    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_CTRL2_CHAN, &CONTROL_BLOCKS_DATA_OUT[0], false);
    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_CTRL1_CHAN, control_blocks, true);

    boot_rom_data_out_start();
    boot_rom_sniffers_reset();
}

__force_inline void __time_critical_func(boot_rom_data_out_start_status_code)(uint32_t status_code)
{
    boot_rom_sniffers_stop();

    data_out_status_code = status_code;
    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_CTRL1_CHAN, &WITH_STATUS_NO_DATA_NO_CRC[0], true);

    boot_rom_data_out_start();
    boot_rom_sniffers_reset();
}

__force_inline void __time_critical_func(boot_rom_data_out_start_busy_code)()
{
    boot_rom_sniffers_stop();
    dma_channel_set_chain_to(BOOT_ROM_DATA_OUT_BUSY_PONG, BOOT_ROM_DATA_OUT_BUSY_PING, false); // busy <- reset
    dma_channel_set_chain_to(BOOT_ROM_DATA_OUT_BUSY_PING, BOOT_ROM_DATA_OUT_BUSY_PONG, true); // busy -> reset
    boot_rom_data_out_start();
    boot_rom_sniffers_reset();
}

__force_inline void __time_critical_func(boot_rom_data_out_stop_busy_code)(uint32_t status_code)
{
    data_out_status_code = status_code;

    // chain reset -> status -> null to end data out
    dma_channel_set_read_addr(BOOT_ROM_DATA_OUT_CTRL1_CHAN, &WITH_STATUS_NO_DATA_NO_CRC[0], false);
    dma_channel_set_chain_to(BOOT_ROM_DATA_OUT_BUSY_PONG, BOOT_ROM_DATA_OUT_CTRL1_CHAN, false);
}
