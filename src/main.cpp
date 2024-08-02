#include <array>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <numbers>
#include <print>
#include <thread>

#include <SDL.h>
#include <SDL_audio.h>
#include <SDL_error.h>
#include <SDL_platform.h>
#include <rtrb.h>

constexpr int sample_rate = 44100;
constexpr std::size_t num_samples = 4096;
constexpr float a4_freq = 440.0;
constexpr float volume = 0.5;

std::atomic_flag consumer_needs_samples = ATOMIC_FLAG_INIT;
std::atomic_flag exit_producer_thread = ATOMIC_FLAG_INIT;

void SDLCALL audio_callback(void *user_data, std::uint8_t *stream, int len)
{
    auto *rb = static_cast<rtrb *>(user_data);

    auto read = rtrb_read(rb, stream, static_cast<std::size_t>(len));

    // Zero leftover bytes, if any.
    for (std::size_t i = read; i < static_cast<std::size_t>(len); ++i)
    {
        stream[i] = 0;
    }

    consumer_needs_samples.test_and_set();
    consumer_needs_samples.notify_one();
}

int main([[maybe_unused]] int argc, [[maybe_unused]] char *argv[])
{
    if (SDL_Init(SDL_INIT_AUDIO) != 0)
    {
        std::println("failed to initialize SDL: {}", SDL_GetError());
        return EXIT_FAILURE;
    }

    rtrb *rb = rtrb_new(num_samples * sizeof(float));

    SDL_AudioSpec desired{};
    SDL_AudioSpec obtained{};
    desired.freq = sample_rate;
    desired.format = AUDIO_F32;
    desired.channels = 1;
    desired.samples = num_samples;
    desired.callback = audio_callback;
    desired.userdata = rb;

    auto audio_device_id = SDL_OpenAudioDevice(nullptr, 0, &desired, &obtained, 0);
    if (audio_device_id == 0)
    {
        std::println("failed to open audio device: {}", SDL_GetError());
        return EXIT_FAILURE;
    }

    std::thread thread(
        [](rtrb *rb) {
            float step = 2.0F * std::numbers::pi_v<float> / (sample_rate / a4_freq);
            float t = 0;

            // The rtrb API doesn't provide direct access to its buffer. So, we
            // write the samples to an intermediate buffer and pass that to
            // rtrb_write.
            std::array<float, num_samples> sample_buffer{};

            while (!exit_producer_thread.test())
            {
                auto available_bytes = rtrb_write_available(rb);
                auto samples = available_bytes / sizeof(float);

                for (size_t i = 0; i < samples; ++i)
                {
                    t += step;
                    sample_buffer[i] = std::sin(t) * volume;
                }

                rtrb_write(rb, reinterpret_cast<std::uint8_t *>(sample_buffer.data()), available_bytes);

                // Check if the thread should exit before waiting.
                if (exit_producer_thread.test())
                {
                    return;
                }

                consumer_needs_samples.wait(false);
                consumer_needs_samples.clear();
            }
        },
        rb);

    // Unpause audio playback.
    SDL_PauseAudioDevice(audio_device_id, 0);

    std::this_thread::sleep_for(std::chrono::seconds(4));

    SDL_CloseAudioDevice(audio_device_id);

    exit_producer_thread.test_and_set();

    // Try to wake up the producer thread in case the exit flag was set just
    // before it went to sleep.
    consumer_needs_samples.test_and_set();
    consumer_needs_samples.notify_one();

    thread.join();

    rtrb_free(rb);

    SDL_Quit();

    return EXIT_SUCCESS;
}
