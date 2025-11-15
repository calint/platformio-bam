#pragma once
//
// sdl3 device implementation for Linux
//

// note: device implementations must define global constants:
//       `static int const display_width`
//       `static int const display_height`

#include "../src/device.hpp"

#include <SDL3/SDL.h>
#include <SDL3/SDL_endian.h>
#include <SDL3/SDL_render.h>
#include <cstring>
#include <stdexcept>

static int const display_width = 240;
static int const display_height = 320;

class device_sdl : public device {
  public:
    device_sdl() = default;

    ~device_sdl() {
        if (buffer_) {
            delete[] buffer_;
        }
        if (texture_) {
            SDL_DestroyTexture(texture_);
        }
        if (renderer_) {
            SDL_DestroyRenderer(renderer_);
        }
        if (window_) {
            SDL_DestroyWindow(window_);
        }
        SDL_Quit();
    }

    auto init() -> void override {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
            throw std::runtime_error("Failed to initialize SDL3");
        }

        window_ =
            SDL_CreateWindow("Device Screen", display_width, display_height, 0);
        if (!window_) {
            SDL_Quit();
            throw std::runtime_error("Failed to create SDL3 window");
        }

        renderer_ = SDL_CreateRenderer(window_, nullptr);
        if (!renderer_) {
            SDL_DestroyWindow(window_);
            SDL_Quit();
            throw std::runtime_error("Failed to create SDL3 renderer");
        }

        if (!SDL_SetRenderLogicalPresentation(
                renderer_, display_width, display_height,
                SDL_LOGICAL_PRESENTATION_LETTERBOX)) {
            SDL_DestroyWindow(window_);
            SDL_Quit();
            throw std::runtime_error("Failed to set aspect ratio");
        }

        texture_ = SDL_CreateTexture(renderer_, SDL_PIXELFORMAT_RGB565,
                                     SDL_TEXTUREACCESS_STREAMING, display_width,
                                     display_height);
        if (!texture_) {
            SDL_DestroyRenderer(renderer_);
            SDL_DestroyWindow(window_);
            SDL_Quit();
            throw std::runtime_error("Failed to create SDL3 texture");
        }

        size_t const buffer_size =
            display_width * display_height * sizeof(uint16_t);
        buffer_ = new uint8_t[buffer_size]();
        if (!buffer_) {
            SDL_DestroyTexture(texture_);
            SDL_DestroyRenderer(renderer_);
            SDL_DestroyWindow(window_);
            SDL_Quit();
            throw std::runtime_error("Failed to allocate display buffer");
        }
    }

    auto display_is_touched() -> bool override {
        handle_events();
        return touch_count_ > 0;
    }

    auto display_touch_count() -> uint8_t override { return touch_count_; }

    auto display_get_touch(touch touches[]) -> void override {
        handle_events();
        std::memcpy(touches, touches_, sizeof(touches_));
    }

    auto dma_write_bytes(uint8_t const* data, uint32_t len) -> void override {
        std::memcpy(buffer_ + dma_offset_, data, len);
        dma_offset_ += len;
        if (dma_offset_ == display_width * display_height * sizeof(uint16_t)) {
            copy_to_screen();
            dma_offset_ = 0;
        }
    }

    auto dma_is_busy() -> bool override { return dma_busy_; }

    auto dma_wait_for_completion() -> void override {}

    // returns true is SPIFFS present and initiated
    auto spiffs_available() const -> bool override { return false; }

    // read from SPIFFS 'path' maximum 'buf_len' into 'buf'
    // returns number of bytes read or -1 if failed
    auto spiffs_read(char const* path, char* buf, int buf_len) const
        -> int override {
        return 0;
    }

    // write to SPIFFS 'path' 'buf_len' bytes from 'buf', 'mode' "w" or "a" for
    // write or append
    // returns true if ok
    auto spiffs_write(char const* path, char const* buf, int buf_len,
                      char const* mode) const -> bool override {
        return false;
    }

    // returns total size of SPIFFS in bytes or 0 if none present
    auto spiffs_size_B() const -> size_t override { return 0; };

    // returns number of used bytes on SPIFFS or 0 if none present
    auto spiffs_used_B() const -> size_t override { return 0; };

    // returns true if path exists
    auto spiffs_path_exists(char const* path) const -> bool override {
        return false;
    }

    // returns true if SD card present and initiated
    auto sd_available() const -> bool override { return false; }

    // read from SD path 'path' maximum 'buf_len' into 'buf'
    // returns number of bytes read or -1 if failed
    auto sd_read(char const* path, char* buf, int buf_len) const
        -> int override {
        return 0;
    }

    // write to SD 'path' 'buf_len' bytes from 'buf', 'mode' "w" or "a" for
    // write or append returns true if ok
    auto sd_write(char const* path, char const* buf, int buf_len,
                  char const* mode) const -> bool override {
        return false;
    }

    // returns total size of SD card in bytes or 0 if none present
    auto sd_size_B() const -> size_t override { return 0; }

    // returns number of bytes used on SD card or 0 if none present
    auto sd_used_B() const -> size_t override { return 0; }

    // returns true if path exists
    auto sd_path_exists(char const* path) const -> bool override {
        return false;
    }

  private:
    SDL_Window* window_{};
    SDL_Renderer* renderer_{};
    SDL_Texture* texture_{};
    uint8_t* buffer_{}; // RGB565 buffer
    bool dma_busy_{};
    uint32_t dma_offset_{};

    static constexpr uint8_t MAX_TOUCHES = 10;
    touch touches_[MAX_TOUCHES]{};
    uint8_t touch_count_{};

    auto handle_events() -> void {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_EVENT_QUIT:
                exit(0);
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN: {
                float logical_x;
                float logical_y;

                touch_count_ = 0;
                if (SDL_RenderCoordinatesFromWindow(
                        renderer_, static_cast<float>(event.button.x),
                        static_cast<float>(event.button.y), &logical_x,
                        &logical_y)) {
                    touches_[0].x = static_cast<uint16_t>(logical_x);
                    touches_[0].y = static_cast<uint16_t>(logical_y);
                    touches_[0].pressure = 255;
                    touch_count_ = 1;
                }
                break;
            }
            case SDL_EVENT_MOUSE_MOTION: {
                if (touch_count_ > 0) {
                    float logical_x;
                    float logical_y;
                    if (SDL_RenderCoordinatesFromWindow(
                            renderer_, static_cast<float>(event.motion.x),
                            static_cast<float>(event.motion.y), &logical_x,
                            &logical_y)) {
                        touches_[0].x = static_cast<uint16_t>(logical_x);
                        touches_[0].y = static_cast<uint16_t>(logical_y);
                    }
                }
                break;
            }
            case SDL_EVENT_MOUSE_BUTTON_UP: {
                touch_count_ = 0;
                break;
            }

            default:
                break;
            }
        }
    }

    auto copy_to_screen() -> void {
        // byte swap to match format of resources
        uint16_t* ptr = reinterpret_cast<uint16_t*>(buffer_);
        for (int i = 0; i < display_width * display_height; i++) {
            *ptr = SDL_Swap16(*ptr);
            ptr++;
        }

        int const pitch =
            display_width * sizeof(uint16_t); // 2 bytes per pixel for RGB565
        if (!SDL_UpdateTexture(texture_, nullptr, buffer_, pitch)) {
            throw std::runtime_error("Failed to update texture");
        }
        SDL_RenderClear(renderer_);
        if (!SDL_RenderTexture(renderer_, texture_, nullptr, nullptr)) {
            throw std::runtime_error("Failed to render texture");
        }
        SDL_RenderPresent(renderer_);
    }
};
