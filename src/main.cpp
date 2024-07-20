#include <array>
#include <atomic>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <numbers>
#include <thread>

#include <SDL.h>
#include <rtrb.h>

constexpr float sample_rate = 44100;
constexpr float a4_freq = 440.0;
constexpr float volume = 0.5;

std::atomic_flag flag = ATOMIC_FLAG_INIT;

void SDLCALL audio_callback(void *user_data, std::uint8_t *stream, int len)
{
    auto *rb = static_cast<rtrb *>(user_data);

    auto read = rtrb_read(rb, stream, len);

    // Zero leftover bytes, if any.
    for (size_t i = read; i < len; ++i)
    {
        stream[i] = 0;
    }

    flag.test_and_set();
    flag.notify_one();
}

int main(int argc, char *argv[])
{
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_EVENTS))
    {
        std::cout << "failed to initialize SDL: " << SDL_GetError() << '\n';
        return EXIT_FAILURE;
    }

    rtrb *rb = rtrb_new(4096 * sizeof(float));

    std::thread thread(
        [](rtrb *rb) {
            std::array<float, 4096> sample_buf;

            float step = 2.0f * std::numbers::pi_v<float> / (sample_rate / a4_freq);
            float t = 0;

            for (;;)
            {
                auto available_bytes = rtrb_write_available(rb);
                auto samples = available_bytes / sizeof(float);

                for (size_t i = 0; i < samples; ++i)
                {
                    t += step;
                    sample_buf[i] = std::sinf(t) * volume;
                }

                // Accessing the pointer after the cast to std::uint8_t* might
                // be UB?
                rtrb_write(rb, reinterpret_cast<std::uint8_t *>(sample_buf.data()), available_bytes);

                flag.wait(false);
                flag.clear();
            }
        },
        rb);

    SDL_AudioSpec desired, obtained;
    std::memset(&desired, 0, sizeof(desired));
    desired.freq = 44100;
    desired.format = AUDIO_F32;
    desired.channels = 1;
    desired.samples = 4096;
    desired.callback = audio_callback;
    desired.userdata = rb;

    auto audio_device_id = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    // Unpause audio playback.
    SDL_PauseAudioDevice(audio_device_id, 0);

    for (;;)
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            if (event.type == SDL_QUIT)
            {
                return 0;
            }
        }
    }
}
