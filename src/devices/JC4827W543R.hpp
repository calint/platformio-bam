#pragma once
// display: NV3041A
//     bus: QSPI

// code lifted and prettified from Arduino_GFX
// https://github.com/moononournation/Arduino_GFX/releases/tag/v1.4.7
// bus/Arduino_ESP32QSPI and display/Arduino_NV3041A

#include "../device.hpp"
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <driver/spi_master.h>

class JC4827W543R final : public device {
  // maximum for this device
  static constexpr int dma_max_transfer_b = 32768;
  static constexpr int nv3041a_max_clock_freq = 32000000;
  // init touch screen
  SPIClass hspi{HSPI}; // note. VSPI is used by the display
  XPT2046_Touchscreen touch_screen{TOUCH_CS, TOUCH_IRQ};

public:
  void init() override {
    // config 'chip select' pin and disable it
    pinMode(TFT_CS, OUTPUT);
    bus_chip_select_disable();

    // init bus
    const spi_bus_config_t bus_cfg = {.data0_io_num = TFT_D0,
                                      .data1_io_num = TFT_D1,
                                      .sclk_io_num = TFT_SCK,
                                      .data2_io_num = TFT_D2,
                                      .data3_io_num = TFT_D3,
                                      .data4_io_num = -1,
                                      .data5_io_num = -1,
                                      .data6_io_num = -1,
                                      .data7_io_num = -1,
                                      .max_transfer_sz = dma_max_transfer_b,
                                      .flags = SPICOMMON_BUSFLAG_MASTER |
                                               SPICOMMON_BUSFLAG_GPIO_PINS,
                                      .intr_flags = 0};
    ESP_ERROR_CHECK(spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO));

    // init graphics device
    const spi_device_interface_config_t dev_cfg = {
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
    ESP_ERROR_CHECK(spi_bus_add_device(SPI2_HOST, &dev_cfg, &device_handle_));

    ESP_ERROR_CHECK(spi_device_acquire_bus(device_handle_, portMAX_DELAY));

    // init transaction values that are constant
    transaction_.user = this;
    transaction_async_.cmd = 0x32;
    transaction_async_.addr = 0x003C00;
    transaction_async_.flags = SPI_TRANS_MODE_QIO;
    transaction_async_.user = this;

    // magic numbers from:
    // https://github.com/moononournation/Arduino_GFX/blob/master/src/display/Arduino_NV3041A.h
    // decoded using manual at:
    // https://admin.osptek.com/uploads/NV_3041_A_Datasheet_V1_2_20221011_686486a221.pdf
    constexpr uint8_t init_commands[] = {
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

        0x21, 0x00, // INVON, invert colors (displays correct colors on device)
        0x11, 0x00  // SLPOUT, turns off sleep mode, default
    };

    for (int i = 0; i < sizeof(init_commands); i += 2) {
      bus_write_c8d8(init_commands[i], init_commands[i + 1]);
    }

    delay(120);

    bus_write_c8d8(0x29, 0x00); // DISPON, turn on display

    delay(100);

    set_rotation(display_orientation == TFT_ORIENTATION ? 0 : 1);
    set_write_address_window(0, 0, display_width, display_height);

    // turn on backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // start the spi for the touch screen and init the library
    hspi.begin(TOUCH_SCK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    touch_screen.begin(hspi);
    touch_screen.setRotation(display_orientation == TFT_ORIENTATION ? 0 : 1);
  }

  bool display_is_touched() override {
    return touch_screen.tirqTouched() && touch_screen.touched();
  }

  void display_get_touch(uint16_t &x, uint16_t &y, uint8_t &pressure) override {
    touch_screen.readData(&x, &y, &pressure);
  }

  bool dma_is_busy() override {
    if (!async_busy_) {
      return false;
    }

    spi_transaction_t *t = nullptr;
    async_busy_ =
        spi_device_get_trans_result(device_handle_, &t, 0) == ESP_ERR_TIMEOUT;

    return async_busy_;
  }

  void dma_wait_for_completion() override {
    if (!async_busy_) {
      return;
    }

    spi_transaction_t *t = nullptr;
    ESP_ERROR_CHECK(
        spi_device_get_trans_result(device_handle_, &t, portMAX_DELAY));

    async_busy_ = false;
  }

  void dma_write_bytes(uint8_t *data, uint32_t len) override {
    dma_wait_for_completion();

    transaction_async_.tx_buffer = data;
    transaction_async_.length = len * 8; // length in bits

    async_busy_ = true;
    ESP_ERROR_CHECK(spi_device_queue_trans(device_handle_, &transaction_async_,
                                           portMAX_DELAY));
  }

private:
  static void pre_transaction_cb(spi_transaction_t *trans) {
    JC4827W543R *dev = static_cast<JC4827W543R *>(trans->user);
    dev->bus_chip_select_enable();
  }

  static void post_transaction_cb(spi_transaction_t *trans) {
    JC4827W543R *dev = static_cast<JC4827W543R *>(trans->user);
    dev->bus_chip_select_disable();
  }

  void bus_chip_select_enable() { digitalWrite(TFT_CS, LOW); }
  void bus_chip_select_disable() { digitalWrite(TFT_CS, HIGH); }

  void bus_write_c8(uint8_t cmd) {
    transaction_.flags = SPI_TRANS_MULTILINE_CMD | SPI_TRANS_MULTILINE_ADDR;
    transaction_.cmd = 0x02;
    transaction_.addr = static_cast<uint32_t>(cmd) << 8;
    transaction_.tx_buffer = NULL;
    transaction_.length = 0;
    ESP_ERROR_CHECK(spi_device_polling_transmit(device_handle_, &transaction_));
  }

  void bus_write_c8d8(uint8_t cmd, uint8_t data) {
    transaction_.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_MULTILINE_CMD |
                         SPI_TRANS_MULTILINE_ADDR;
    transaction_.cmd = 0x02;
    transaction_.addr = static_cast<uint32_t>(cmd) << 8;
    transaction_.tx_data[0] = data;
    transaction_.length = 8; // in bits
    ESP_ERROR_CHECK(spi_device_polling_transmit(device_handle_, &transaction_));
  }

  void bus_write_c8d16d16(uint8_t cmd, uint16_t data1, uint16_t data2) {
    transaction_.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_MULTILINE_CMD |
                         SPI_TRANS_MULTILINE_ADDR;
    transaction_.cmd = 0x02;
    transaction_.addr = static_cast<uint32_t>(cmd) << 8;
    transaction_.tx_data[0] = data1 >> 8;
    transaction_.tx_data[1] = data1;
    transaction_.tx_data[2] = data2 >> 8;
    transaction_.tx_data[3] = data2;
    transaction_.length = 32; // in bits
    ESP_ERROR_CHECK(spi_device_polling_transmit(device_handle_, &transaction_));
  }

  void set_rotation(uint8_t r) {
    constexpr uint8_t NV3041A_MADCTL_MY = 0x80;
    constexpr uint8_t NV3041A_MADCTL_MX = 0x40;
    constexpr uint8_t NV3041A_MADCTL_MV = 0x20;

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

  void set_write_address_window(int16_t x, int16_t y, uint16_t w, uint16_t h) {
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
