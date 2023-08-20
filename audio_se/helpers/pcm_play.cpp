#include <iostream>
#include <fstream>
#include <alsa/asoundlib.h>

#define CHANNELS 1
#define FORMAT SND_PCM_FORMAT_U8

unsigned int SAMPLE_RATE = 8000;

int main() {
    int err;
    snd_pcm_t *playback_handle;
    snd_pcm_hw_params_t *hw_params;
    char *buffer;
    int buffer_size;
    std::ifstream wav_file;
    int dir;

    // Open the playback device
    if ((err = snd_pcm_open(&playback_handle, "plughw:CARD=Headphones,DEV=0", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        std::cerr << "Cannot open playback audio device: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Allocate hardware parameters object
    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
        std::cerr << "Cannot allocate hardware parameter structure: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Initialize hardware parameters with default values
    if ((err = snd_pcm_hw_params_any(playback_handle, hw_params)) < 0) {
        std::cerr << "Cannot initialize hardware parameter structure: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Set hardware parameters
    if ((err = snd_pcm_hw_params_set_access(playback_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        std::cerr << "Cannot set access type: " << snd_strerror(err) << std::endl;
        return 1;
    }

    if ((err = snd_pcm_hw_params_set_format(playback_handle, hw_params, FORMAT)) < 0) {
        std::cerr << "Cannot set audio format: " << snd_strerror(err) << std::endl;
        return 1;
    }

    if ((err = snd_pcm_hw_params_set_channels(playback_handle, hw_params, CHANNELS)) < 0) {
        std::cerr << "Cannot set channel count: " << snd_strerror(err) << std::endl;
        return 1;
    }

    if ((err = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params, &SAMPLE_RATE, 0)) < 0) {
        std::cerr << "Cannot set sample rate: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Set the hardware parameters for the playback handle
    if ((err = snd_pcm_hw_params(playback_handle, hw_params)) < 0) {
        std::cerr << "Cannot set hardware parameters: " << snd_strerror(err) << std::endl;
        return 1;
    }



    // Open the .wav file for reading
    wav_file.open("test.wav", std::ios::binary);
    if (!wav_file) {
        std::cerr << "Cannot open input.wav for reading." << std::endl;
        return 1;
    }

    // Get the size of the .wav file
    wav_file.seekg(0, std::ios::end);
    buffer_size = wav_file.tellg();
    wav_file.seekg(0, std::ios::beg);

    // Allocate memory for the buffer
    buffer = (char *) malloc(buffer_size);
    if (!buffer) {
        std::cerr << "Memory allocation failed." << std::endl;
        return 1;
    }

    // Read audio data from the .wav file
    wav_file.read(buffer, buffer_size);

    // Write audio data to the playback device
    if ((err = snd_pcm_writei(playback_handle, buffer, buffer_size / (CHANNELS * snd_pcm_format_width(FORMAT) / 8))) < 0) {
        std::cerr << "Write to audio device failed: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Wait for playback to complete
    snd_pcm_drain(playback_handle);

    // Close the .wav file
    wav_file.close();

    // Free the hardware parameters object
    snd_pcm_hw_params_free(hw_params);

    // Free the buffer and handle resources
    free(buffer);
    snd_pcm_close(playback_handle);

    return 0;
}
