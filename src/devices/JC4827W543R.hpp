#pragma once
// device implementation mostly from Arduino_GFX and XPT2046
// code lifted from Arduino_ESP32QSPI and Arduino_NV3041A
#include "../device.hpp"
#include <SPI.h>
#include <XPT2046_Touchscreen.h>
#include <driver/spi_master.h>

#define NV3041A_MADCTL 0x36
#define NV3041A_COLMOD 0x3A

#define NV3041A_MADCTL_MY 0x80
#define NV3041A_MADCTL_MX 0x40
#define NV3041A_MADCTL_MV 0x20
#define NV3041A_MADCTL_ML 0x10
#define NV3041A_MADCTL_RGB 0x00

#define NV3041A_CASET 0x2A
#define NV3041A_RASET 0x2B
#define NV3041A_RAMWR 0x2C

class JC4827W543R final : public device {
  // maximum for this device
  static constexpr int dma_max_transfer_b = 32768;
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
    esp_err_t ret = spi_bus_initialize(SPI2_HOST, &bus_cfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
      ESP_ERROR_CHECK(ret);
      assert(ret);
    }

    // init graphics device
    const spi_device_interface_config_t dev_cfg = {
        .command_bits = 8,
        .address_bits = 24,
        .dummy_bits = 0,
        .mode = SPI_MODE0,
        .duty_cycle_pos = 0,
        .cs_ena_pretrans = 0,
        .cs_ena_posttrans = 0,
        .clock_speed_hz = 32000000UL,
        .input_delay_ns = 0,
        .spics_io_num = -1, // avoid use system CS control
        .flags = SPI_DEVICE_HALFDUPLEX,
        .queue_size = 1,
        .pre_cb = pre_transaction_cb,
        .post_cb = post_transaction_cb};
    ret = spi_bus_add_device(SPI2_HOST, &dev_cfg, &device_handle_);
    if (ret != ESP_OK) {
      ESP_ERROR_CHECK(ret);
      assert(ret);
    }

    assert(spi_device_acquire_bus(device_handle_, portMAX_DELAY) == ESP_OK);

    // init values that will not change
    transaction_async_.cmd = 0x32;
    transaction_async_.addr = 0x003C00;
    transaction_async_.flags = SPI_TRANS_MODE_QIO;
    transaction_async_.user = this;

    // magic numbers from from Arduino_NV3041A
    constexpr uint8_t init_commands[] = {
        0xff, 0xa5, //
        0x36, 0xc0, // read / write scanning direction of frame memory

        0x3A, 0x01, // format of RGB data, 01: 565 (default)，00: 666

        0x41, 0x03, // bus width, 01: 8bit, 03: 16bit

        0x44, 0x15, // internal scan vbp ??? 21
        0x45, 0x15, // internal scan vfp ??? 21

        0x7d, 0x03, // vdds_trim[2:0]  2.07V

        0xc1, 0xbb, // avdd_clp_en avdd_clp[1:0] avcl_clp_en avcl_clp[1:0]
                    // 0xbb 88  a2

        0xc2, 0x05, // vgl_clp_en vgl_clp[2:0] default 15h
        0xc3, 0x10, // vgl_clp_en vgl_clp[2:0] default 12h
        0xc6, 0x3e, // avdd_ratio_sel avcl_ratio_sel vgh_ratio_sel[1:0] default
                    // 35h vgl_ratio_sel[1:0] default 35h
        0xc7, 0x25, // mv_clk_sel[1:0] avdd_clk_sel[1:0] avcl_clk_sel[1:0]
                    // default 2ah
        0xc8, 0x11, // VGL_CLK_sel default 11h

        0x7a, 0x5f, // user_vgsp 4f:0.8V 3f:1.04V	5f

        0x6f, 0x44, // user_gvdd 1C:5.61  5f 53  2a  3a

        0x78, 0x70, // user_gvcl 50:-3.22  75  58  66

        0xc9, 0x00, // mv_clk_clp

        0x67, 0x21, //

        // gate_ed

        0x51, 0x0a, // gate_st_o[7:0]
        0x52, 0x76, // gate_ed_o[7:0]
        0x53, 0x0a, // gate_st_e[7:0]
        0x54, 0x76, // gate_ed_e[7:0]

        // FSM_V-Porch
        0x46, 0x0a, // fsm_hbp_o[5:0]
        0x47, 0x2a, // fsm_hfp_o[5:0]
        0x48, 0x0a, // fsm_hbp_e[5:0]
        0x49, 0x1a, // fsm_hfp_e[5:0]

        0x56, 0x43, // src_ld_wd[1:0] src_ld_st[5:0]
        0x57, 0x42, // pn_cs_en src_cs_st[5:0]
        0x58, 0x3c, // src_cs_p_wd[6:0]
        0x59, 0x64, // src_cs_n_wd[6:0]
        0x5a, 0x41, // src_pchg_st_o[6:0]
        0x5b, 0x3c, // src_pchg_wd_o[6:0]
        0x5c, 0x02, // src_pchg_st_e[6:0]
        0x5d, 0x3c, // src_pchg_wd_e[6:0]
        0x5e, 0x1f, // src_pol_sw[7:0]
        0x60, 0x80, // src_op_st_o[7:0]
        0x61, 0x3f, // src_op_st_e[7:0]
        0x62, 0x21, // src_op_ed_o[9:8] src_op_ed_e[9:8]
        0x63, 0x07, // src_op_ed_o[7:0]
        0x64, 0xe0, // src_op_ed_e[7:0]
        0x65, 0x02, // chopper

        0xca, 0x20, // avdd_mux_st_o[7:0]
        0xcb, 0x52, // avdd_mux_ed_o[7:0]
        0xcc, 0x10, // avdd_mux_st_e[7:0]
        0xcd, 0x42, // avdd_mux_ed_e[7:0]
        0xd0, 0x20, // avcl_mux_st_o[7:0]
        0xd1, 0x52, // avcl_mux_ed_o[7:0]
        0xd2, 0x10, // avcl_mux_st_e[7:0]
        0xd3, 0x42, // avcl_mux_ed_e[7:0]
        0xd4, 0x0a, // vgh_mux_st[7:0]
        0xd5, 0x32, // vgh_mux_ed[7:0]

        // 2-1
        // gammma  weihuan pianguangpian 0913
        0x80, 0x00, // gam_vrp0 0 6bit
        0xA0, 0x00, // gam_VRN0 0-

        0x81, 0x07, // gam_vrp1 1   6bit
        0xA1, 0x06, // gam_VRN1 1-

        0x82, 0x02, // gam_vrp2  2   6bit
        0xA2, 0x01, // gam_VRN2  2-

        0x86, 0x11, // gam_prp0  7bit  8 7bit
        0xA6, 0x10, // gam_PRN0  8-

        0x87, 0x27, // gam_prp1 7bit  40  7bit
        0xA7, 0x27, // gam_PRN1 40-

        0x83, 0x37, // gam_vrp3 61  6bit
        0xA3, 0x37, // gam_VRN3 61-

        0x84, 0x35, // gam_vrp4 62  6bit
        0xA4, 0x35, // gam_VRN4 62-

        0x85, 0x3f, // gam_vrp5 63  6bit
        0xA5, 0x3f, // gam_VRN5 63-
        //

        0x88, 0x0b, // gam_pkp0 4   5bit
        0xa8, 0x0b, // gam_PKN0 4-

        0x89, 0x14, // gam_pkp1 5   5bit
        0xA9, 0x14, // gam_PKN1 5-

        0x8a, 0x1a, // gam_pkp2 7   5bit
        0xaa, 0x1a, // gam_PKN2 7-

        0x8b, 0x0a, // gam_PKP3 10  5bit
        0xab, 0x0a, // gam_PKN3  10-

        0x8c, 0x14, // gam_PKP4 16  5bit
        0xac, 0x08, // gam_PKN4  16-

        0x8d, 0x17, // gam_PKP5 22  5bit
        0xad, 0x07, // gam_PKN5 22-

        0x8e, 0x16, // gam_PKP6  28  5bit
        0xae, 0x06, // gam_PKN6  28-

        0x8f, 0x1b, // gam_PKP7  34  5bit
        0xaf, 0x07, // gam_PKN7  34-

        0x90, 0x04, // gam_PKP8  46  5bit
        0xb0, 0x04, // gam_PKN8  46-

        0x91, 0x0a, // gam_PKP9  52 5bit
        0xb1, 0x0a, // gam_PKN9  52-

        0x92, 0x16, // gam_PKP10 58 5bit
        0xb2, 0x15, // gam_PKN10 58-
        // gamma done

        0xff, 0x00,

        0x11, 0x00 // turns off sleep mode
    };

    for (int i = 0; i < sizeof(init_commands); i += 2) {
      bus_write_c8d8(init_commands[i], init_commands[i + 1]);
    }

    delay(120);

    // bus_write_c8(0x20); // inversion off
    bus_write_c8(0x21); // inversion on (results in correct colors on device)
    bus_write_c8d8(0x29, 0x00); // turn on display

    delay(100);

    set_rotation(display_orientation == TFT_ORIENTATION ? 0 : 1);
    set_write_address_window(0, 0, display_width, display_height);

    // turn on backlight
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, HIGH);

    // start the spi for the touch screen and init the library
    hspi.begin(TOUCH_SCK, TOUCH_MISO, TOUCH_MOSI, TOUCH_CS);
    touch_screen.begin(hspi);
    touch_screen.setRotation(display_orientation ? 0 : 1);
  }

  bool is_display_touched() override {
    return touch_screen.tirqTouched() && touch_screen.touched();
  }

  void get_display_touch(uint16_t &x, uint16_t &y, uint8_t &pressure) override {
    touch_screen.readData(&x, &y, &pressure);
  }

  bool asyncDMAIsBusy() override {
    if (!async_busy_) {
      return false;
    }

    spi_transaction_t *t = nullptr;
    async_busy_ =
        spi_device_get_trans_result(device_handle_, &t, 0) == ESP_ERR_TIMEOUT;

    return async_busy_;
  }

  void asyncDMAWaitForCompletion() override {
    if (!async_busy_) {
      return;
    }

    spi_transaction_t *t = nullptr;
    assert(spi_device_get_trans_result(device_handle_, &t, portMAX_DELAY) ==
           ESP_OK);

    async_busy_ = false;
  }

  void asyncDMAWriteBytes(uint8_t *data, uint32_t len) override {
    asyncDMAWaitForCompletion();

    transaction_async_.tx_buffer = data;
    transaction_async_.length = len * 8; // length in bits

    async_busy_ = true;
    assert(spi_device_queue_trans(device_handle_, &transaction_async_,
                                  portMAX_DELAY) == ESP_OK);
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
    transaction_.user = this;
    assert(spi_device_polling_transmit(device_handle_, &transaction_) ==
           ESP_OK);
  }

  void bus_write_c8d8(uint8_t cmd, uint8_t data) {
    transaction_.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_MULTILINE_CMD |
                         SPI_TRANS_MULTILINE_ADDR;
    transaction_.cmd = 0x02;
    transaction_.addr = static_cast<uint32_t>(cmd) << 8;
    transaction_.tx_data[0] = data;
    transaction_.length = 8;
    transaction_.user = this;
    assert(spi_device_polling_transmit(device_handle_, &transaction_) ==
           ESP_OK);
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
    transaction_.length = 32;
    transaction_.user = this;
    assert(spi_device_polling_transmit(device_handle_, &transaction_) ==
           ESP_OK);
  }

  void set_rotation(uint8_t r) {
    switch (r) {
    case 1:
      r = NV3041A_MADCTL_MY | NV3041A_MADCTL_MV | NV3041A_MADCTL_RGB;
      break;
    case 2:
      r = NV3041A_MADCTL_RGB;
      break;
    case 3:
      r = NV3041A_MADCTL_MX | NV3041A_MADCTL_MV | NV3041A_MADCTL_RGB;
      break;
    default: // case 0:
      r = NV3041A_MADCTL_MX | NV3041A_MADCTL_MY | NV3041A_MADCTL_RGB;
      break;
    }
    bus_write_c8d8(NV3041A_MADCTL, r);
  }

  void set_write_address_window(int16_t x, int16_t y, uint16_t w, uint16_t h) {
    bus_write_c8d16d16(NV3041A_CASET, x, x + w - 1);
    bus_write_c8d16d16(NV3041A_RASET, y, y + h - 1);
    bus_write_c8(NV3041A_RAMWR);
  }

  spi_host_device_t host_device_{};
  spi_device_handle_t device_handle_{};
  spi_transaction_t transaction_{};
  spi_transaction_t transaction_async_{};
  bool async_busy_ = false;
};
