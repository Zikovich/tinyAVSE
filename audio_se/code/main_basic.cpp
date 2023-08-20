#include <iostream>
#include <alsa/asoundlib.h>
#include <chrono>
#include <thread>

#define BUFFER_SIZE      4096 * 10
#define CHANNELS 1
#define FORMAT SND_PCM_FORMAT_U8
#define DELAY_MS 1000
unsigned int sample_rate = 8000;
unsigned int block = 0;


int err;
char *buffer;
snd_pcm_t *capture_handle;    
snd_pcm_t *playback_handle;
snd_pcm_hw_params_t *hw_params;
snd_pcm_hw_params_t *hw_params_speaker;


int openAudio ()
{

    // Open the capture (recording) device
    if ((err = snd_pcm_open(&capture_handle, "plughw:CARD=3,DEV=0", SND_PCM_STREAM_CAPTURE, 0)) < 0) {
        std::cerr << "Cannot open capture audio device: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Allocate hardware parameters object
    if ((err = snd_pcm_hw_params_malloc(&hw_params)) < 0) {
        std::cerr << "Cannot allocate hardware parameters: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Initialize hardware parameters with default values
    if ((err = snd_pcm_hw_params_any(capture_handle, hw_params)) < 0) {
        std::cerr << "Cannot initialize hardware parameters: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Set the desired parameters for the capture device (e.g., sample rate, channels, format)
    if ((err = snd_pcm_hw_params_set_access(capture_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        std::cerr << "Cannot set access type: " << snd_strerror(err) << std::endl;
        return 1;
    }
    if ((err = snd_pcm_hw_params_set_format(capture_handle, hw_params, FORMAT)) < 0) {
        std::cerr << "Cannot set sample format: " << snd_strerror(err) << std::endl;
        return 1;
    }

    if ((err = snd_pcm_hw_params_set_rate_near(capture_handle, hw_params, &sample_rate, 0)) < 0) {
        std::cerr << "Cannot set sample rate: " << snd_strerror(err) << std::endl;
        return 1;
    }
    if ((err = snd_pcm_hw_params_set_channels(capture_handle, hw_params, CHANNELS)) < 0) {
        std::cerr << "Cannot set channel count: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Apply the configured hardware parameters to the capture device
    if ((err = snd_pcm_hw_params(capture_handle, hw_params)) < 0) {
        std::cerr << "Cannot set hardware parameters: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Allocate buffer for audio data
    buffer = new char[BUFFER_SIZE];


    if ((err = snd_pcm_open(&playback_handle, "plughw:CARD=Headphones,DEV=0", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
        std::cerr << "Cannot open play audio device: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Allocate hardware parameters object
    if ((err = snd_pcm_hw_params_malloc(&hw_params_speaker)) < 0) {
        std::cerr << "Cannot allocate hardware parameter structure: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Initialize hardware parameters with default values
    if ((err = snd_pcm_hw_params_any(playback_handle, hw_params_speaker)) < 0) {
        std::cerr << "Cannot initialize hardware parameter structure: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Set hardware parameters
    if ((err = snd_pcm_hw_params_set_access(playback_handle, hw_params_speaker, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
        std::cerr << "Cannot set access type: " << snd_strerror(err) << std::endl;
        return 1;
    }

    if ((err = snd_pcm_hw_params_set_format(playback_handle, hw_params_speaker, FORMAT)) < 0) {
        std::cerr << "Cannot set audio format: " << snd_strerror(err) << std::endl;
        return 1;
    }

    if ((err = snd_pcm_hw_params_set_channels(playback_handle, hw_params_speaker, CHANNELS)) < 0) {
        std::cerr << "Cannot set channel count: " << snd_strerror(err) << std::endl;
        return 1;
    }

    if ((err = snd_pcm_hw_params_set_rate_near(playback_handle, hw_params_speaker, &sample_rate, 0)) < 0) {
        std::cerr << "Cannot set sample rate: " << snd_strerror(err) << std::endl;
        return 1;
    }

    // Set the hardware parameters for the playback handle
    if ((err = snd_pcm_hw_params(playback_handle, hw_params_speaker)) < 0) {
        std::cerr << "Cannot set hardware parameters: " << snd_strerror(err) << std::endl;
        return 1;
    }
    return 0;
}

void readAudio()
{
    printf("\n=== Now recording!! Please speak into the microphone. ===\n");
    if ((err = snd_pcm_readi(capture_handle, buffer, BUFFER_SIZE / (CHANNELS * snd_pcm_format_width(FORMAT) / 8))) < 0)
        {
        std::cerr << "Read from audio device failed: " << snd_strerror(err) << std::endl;

        err = snd_pcm_recover(capture_handle, err, 0);
        err = snd_pcm_state(capture_handle);
        printf("after recovery snd_pcm_state capture_handle: %d\n",err);
    }
}

void writeAudio()
{
    printf("\n=== Now playing back. ===\n");
    if ((err = snd_pcm_writei(playback_handle, buffer, BUFFER_SIZE / (CHANNELS * snd_pcm_format_width(FORMAT) / 8))) < 0)
    {
        std::cerr << "Write to audio device failed: " << snd_strerror(err) << std::endl;
        err = snd_pcm_recover(playback_handle, err, 0);
        err = snd_pcm_state(playback_handle);
        printf("after recovery snd_pcm_state capture_handle: %d\n",err);
    }

}

void closeAudio()
{
        // Clean up resources
    delete[] buffer;
    // Free the hardware parameters object
    
    snd_pcm_drain(playback_handle);

    snd_pcm_drain(capture_handle);
    snd_pcm_hw_params_free(hw_params);
    snd_pcm_hw_params_free(hw_params_speaker);
    snd_pcm_close(capture_handle);
    snd_pcm_close(playback_handle);

}

int main() {


    // Start the audio capture and playback loop
    while (true) {
        openAudio();

        readAudio();

        writeAudio();

        closeAudio();


    }


    return 0;
}
