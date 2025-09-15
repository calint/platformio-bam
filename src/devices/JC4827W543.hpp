#pragma once
// display: NV3041A
//     bus: QSPI

// code lifted and prettified from Arduino_GFX
// https://github.com/moononournation/Arduino_GFX/releases/tag/v1.4.7
// bus/Arduino_ESP32QSPI and display/Arduino_NV3041A

#include "../device.hpp"
#include <SPI.h>
#include <driver/spi_master.h>

// define display dimensions depending on orientation using device natural
// orientation and requested orientation
// note: `display_width` and `display_height` are necessary constants for the
//       framework
//       `display_orientation` defined in `game/defs.hpp`
static int constexpr display_width = display_orientation == 1 ? 480 : 272;
static int constexpr display_height = display_orientation == 1 ? 272 : 480;

/// @brief Abstract class for resistive and capacitive versions of the device
class JC4827W543 : public device {
    // maximum for this device
    static int constexpr dma_max_transfer_b = 32768;
    static int constexpr nv3041a_max_clock_freq = 32000000;

    static int constexpr tft_width = 480;
    static int constexpr tft_height = 272;
    static int constexpr tft_orientation = 1; // native orientation is landscape
    static int constexpr tft_cs = 45;
    static int constexpr tft_sck = 47;
    static int constexpr tft_d0 = 21;
    static int constexpr tft_d1 = 48;
    static int constexpr tft_d2 = 40;
    static int constexpr tft_d3 = 39;
    static int constexpr tft_bl = 1;

    static int constexpr sd_mosi = 11;
    static int constexpr sd_miso = 13;
    static int constexpr sd_sck = 12;
    static int constexpr sd_cs = 10;

  protected:
    SPIClass spi2{SPI2_HOST};

    virtual auto init_touch_screen() -> void = 0;

  public:
    auto init() -> void override {
        pinMode(tft_cs, OUTPUT);
        bus_chip_select_disable();

        // init bus
        spi_bus_config_t const bus_cfg = {.data0_io_num = tft_d0,
                                          .data1_io_num = tft_d1,
                                          .sclk_io_num = tft_sck,
                                          .data2_io_num = tft_d2,
                                          .data3_io_num = tft_d3,
                                          .data4_io_num = -1,
                                          .data5_io_num = -1,
                                          .data6_io_num = -1,
                                          .data7_io_num = -1,
                                          .max_transfer_sz = dma_max_transfer_b,
                                          .flags = SPICOMMON_BUSFLAG_MASTER |
                                                   SPICOMMON_BUSFLAG_GPIO_PINS,
                                          .intr_flags = 0};
        ESP_ERROR_CHECK(
            spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

        // init graphics device
        spi_device_interface_config_t const dev_cfg = {
            .command_bits = 8,
            .address_bits = 24,
            .dummy_bits = 0,
            .mode = SPI_MODE0,
            .duty_cycle_pos = 0,
            .cs_ena_pretrans = 0,
            .cs_ena_posttrans = 0,
            .clock_speed_hz = nv3041a_max_clock_freq,
            .input_delay_ns = 0,
            .spics_io_num = -1, // avoid use system CS control
            .flags = SPI_DEVICE_HALFDUPLEX,
            .queue_size = 1,
            .pre_cb = pre_transaction_cb,
            .post_cb = post_transaction_cb};
        ESP_ERROR_CHECK(
            spi_bus_add_device(SPI2_HOST, &dev_cfg, &device_handle_));

        ESP_ERROR_CHECK(spi_device_acquire_bus(device_handle_, portMAX_DELAY));

        // init transaction values that are constant
        transaction_.user = this;

        // configuration of DMA transaction found in:
        // `src/databus/Arduino_ESP32QSPI.cpp` function: writePixels
        transaction_async_.cmd = 0x32;
        transaction_async_.addr = 0x003C00;
        transaction_async_.flags = SPI_TRANS_MODE_QIO;
        transaction_async_.user = this;

        // magic numbers from: `src/display/Arduino_NV3041A.h`
        // decoded using manual at:
        // https://admin.osptek.com/uploads/NV_3041_A_Datasheet_V1_2_20221011_686486a221.pdf
        uint8_t constexpr init_commands[] = {
            0xff, 0xa5, // ?

            // gammma  weihuan pianguangpian 0913
            0x80, 0x00, // gam_vrp0[5:0]
            0xa0, 0x00, // gam_vrn0[5:0]

            0x81, 0x07, // gam_vrp1[5:0]
            0xa1, 0x06, // gam_vrn1[5:0]

            0x82, 0x02, // gam_vrp2[5:0]
            0xa2, 0x01, // gam_vrn2[5:0]

            0x86, 0x11, // gam_prp0[6:0]
            0xa6, 0x10, // gam_prn0[6:0]

            0x87, 0x27, // gam_prp1[6:0]
            0xa7, 0x27, // gam_prn1[6:0]

            0x83, 0x37, // gam_vrp3[5:0]
            0xa3, 0x37, // gam_vrn3[5:0]

            0x84, 0x35, // gam_vrp4[5:0]
            0xa4, 0x35, // gam_vrn4[5:0]

            0x85, 0x3f, // gam_vrp5[5:0]
            0xa5, 0x3f, // gam_vrn5[5:0]

            0x88, 0x0b, // gam_pkp0[4:0]
            0xa8, 0x0b, // gam_pkn0[4:0]

            0x89, 0x14, // gam_pkp1[4:0]
            0xa9, 0x14, // gam_pkn1[4:0]

            0x8a, 0x1a, // gam_pkp2[4:0]
            0xaa, 0x1a, // gam_pkn2[4:0]

            0x8b, 0x0a, // gam_pkp3[4:0]
            0xab, 0x0a, // gam_pkn3[4:0]

            0x8c, 0x14, // gam_pkp4[4:0]
            0xac, 0x08, // gam_pkn4[4:0]

            0x8d, 0x17, // gam_pkp5[4:0]
            0xad, 0x07, // gam_pkn5[4:0]

            0x8e, 0x16, // gam_pkp6[4:0]
            0xae, 0x06, // gam_pkn6[4:0]

            0x8f, 0x1b, // gam_pkp7[4:0]
            0xaf, 0x07, // gam_pkn7[4:0]

            0x90, 0x04, // gam_pkp8[4:0]
            0xb0, 0x04, // gam_pkn8[4:0]

            0x91, 0x0a, // gam_pkp9[4:0]
            0xb1, 0x0a, // gam_pkn9[4:0]

            0x92, 0x16, // gam_pkp10[4:0]
            0xb2, 0x15, // gam_pkn10[4:0]

            // gamma done

            // INVON, invert colors (displays correct colors on device)
            0x21, 0x00,

            // SLPOUT, turns off sleep mode, default
            0x11, 0x00};

        for (int i = 0; i < sizeof(init_commands); i += 2) {
            bus_write_c8d8(init_commands[i], init_commands[i + 1]);
        }

        delay(120); // manual page 70 specifies the delay before other commands

        bus_write_c8d8(0x29, 0x00); // DISPON, turn on display

        delay(100);

        set_rotation(display_orientation == tft_orientation ? 0 : 1);
        set_write_address_window(0, 0, display_width, display_height);

        // turn on backlight
        pinMode(tft_bl, OUTPUT);
        digitalWrite(tft_bl, HIGH);

        spi2.begin(sd_sck, sd_miso, sd_mosi);

        init_touch_screen();

        init_sd_spiffs(spi2, sd_cs, 80000000);
    }

    auto dma_is_busy() -> bool override {
        if (!async_busy_) {
            return false;
        }

        spi_transaction_t* t = nullptr;
        async_busy_ = spi_device_get_trans_result(device_handle_, &t, 0) ==
                      ESP_ERR_TIMEOUT;

        return async_busy_;
    }

    auto dma_wait_for_completion() -> void override {
        if (!async_busy_) {
            return;
        }

        spi_transaction_t* t = nullptr;
        ESP_ERROR_CHECK(
            spi_device_get_trans_result(device_handle_, &t, portMAX_DELAY));

        async_busy_ = false;
    }

    auto dma_write_bytes(uint8_t const* data, uint32_t const len)
        -> void override {
        dma_wait_for_completion();

        transaction_async_.tx_buffer = data;
        transaction_async_.length = len * 8; // length in bits

        async_busy_ = true;
        ESP_ERROR_CHECK(spi_device_queue_trans(
            device_handle_, &transaction_async_, portMAX_DELAY));
    }

  private:
    static auto pre_transaction_cb(spi_transaction_t* trans) -> void {
        JC4827W543* dev = static_cast<JC4827W543*>(trans->user);
        dev->bus_chip_select_enable();
    }

    static auto post_transaction_cb(spi_transaction_t* trans) -> void {
        JC4827W543* dev = static_cast<JC4827W543*>(trans->user);
        dev->bus_chip_select_disable();
    }

    auto bus_chip_select_enable() -> void { digitalWrite(tft_cs, LOW); }
    auto bus_chip_select_disable() -> void { digitalWrite(tft_cs, HIGH); }

    auto bus_write_c8(uint8_t const cmd) -> void {
        transaction_.flags = SPI_TRANS_MULTILINE_CMD | SPI_TRANS_MULTILINE_ADDR;
        transaction_.cmd = 0x02;
        transaction_.addr = static_cast<uint32_t>(cmd) << 8;
        transaction_.tx_buffer = nullptr;
        transaction_.length = 0;
        ESP_ERROR_CHECK(
            spi_device_polling_transmit(device_handle_, &transaction_));
    }

    auto bus_write_c8d8(uint8_t const cmd, uint8_t const data) -> void {
        transaction_.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_MULTILINE_CMD |
                             SPI_TRANS_MULTILINE_ADDR;
        transaction_.cmd = 0x02;
        transaction_.addr = static_cast<uint32_t>(cmd) << 8;
        transaction_.tx_data[0] = data;
        transaction_.length = 8; // in bits
        ESP_ERROR_CHECK(
            spi_device_polling_transmit(device_handle_, &transaction_));
    }

    auto bus_write_c8d16d16(uint8_t const cmd, uint16_t const data1,
                            uint16_t const data2) -> void {
        transaction_.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_MULTILINE_CMD |
                             SPI_TRANS_MULTILINE_ADDR;
        transaction_.cmd = 0x02;
        transaction_.addr = static_cast<uint32_t>(cmd) << 8;
        transaction_.tx_data[0] = data1 >> 8;
        transaction_.tx_data[1] = data1;
        transaction_.tx_data[2] = data2 >> 8;
        transaction_.tx_data[3] = data2;
        transaction_.length = 32; // in bits
        ESP_ERROR_CHECK(
            spi_device_polling_transmit(device_handle_, &transaction_));
    }

    auto set_rotation(uint8_t r) -> void {
        uint8_t constexpr NV3041A_MADCTL_MY = 0x80;
        uint8_t constexpr NV3041A_MADCTL_MX = 0x40;
        uint8_t constexpr NV3041A_MADCTL_MV = 0x20;

        switch (r) {
        case 1:
            r = NV3041A_MADCTL_MY | NV3041A_MADCTL_MV;
            break;
        case 2:
            r = 0;
            break;
        case 3:
            r = NV3041A_MADCTL_MX | NV3041A_MADCTL_MV;
            break;
        default: // case 0:
            r = NV3041A_MADCTL_MX | NV3041A_MADCTL_MY;
            break;
        }
        bus_write_c8d8(0x36, r);
    }

    auto set_write_address_window(int16_t const x, int16_t const y,
                                  uint16_t const w, uint16_t const h) -> void {
        bus_write_c8d16d16(0x2a, x, x + w - 1);
        bus_write_c8d16d16(0x2b, y, y + h - 1);
        bus_write_c8(0x2c);
    }

    spi_host_device_t host_device_{};
    spi_device_handle_t device_handle_{};
    spi_transaction_t transaction_{};
    spi_transaction_t transaction_async_{};
    bool async_busy_ = false;
};
