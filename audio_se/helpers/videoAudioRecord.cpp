#include <iostream>
#include <opencv2/opencv.hpp>
#include <portaudio.h>
#include <vector>
#include <cstdio>

const int SAMPLE_RATE = 8000;
const int FRAMES_PER_BUFFER = 256;

// Audio callback function
int audioCallback(const void *inputBuffer, void *outputBuffer, unsigned long framesPerBuffer,
                  const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags,
                  void *userData) {
    // Store the audio data in the userData (std::vector<float>*)
    std::vector<float>* audioData = static_cast<std::vector<float>*>(userData);
    const float *in = static_cast<const float*>(inputBuffer);
    audioData->insert(audioData->end(), in, in + framesPerBuffer);
    return paContinue;
}

int main() {
    PaError err;
    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return -1;
    }

    std::vector<float> audioData; // Stores the audio data

    cv::VideoCapture cap(0); // Open the default camera (index 0)
    if (!cap.isOpened()) {
        std::cerr << "Error: Unable to open the camera." << std::endl;
        return -1;
    }

    int frame_width = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_WIDTH));
    int frame_height = static_cast<int>(cap.get(cv::CAP_PROP_FRAME_HEIGHT));

    // Set up PortAudio audio stream
    PaStream* audioStream;
    PaStreamParameters inputParameters;
    inputParameters.device = 3; // Use the default input device
    inputParameters.channelCount = 1; // Mono audio
    inputParameters.sampleFormat = paFloat32; // 32-bit floating-point format
    inputParameters.suggestedLatency = Pa_GetDeviceInfo(inputParameters.device)->defaultLowInputLatency;
    inputParameters.hostApiSpecificStreamInfo = nullptr;
    
    err = Pa_OpenStream(&audioStream, &inputParameters, nullptr, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff,
                        audioCallback, &audioData);
    
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return -1;
    }

    err = Pa_StartStream(audioStream);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
        return -1;
    }

    cv::VideoWriter videoWriter("output_video.avi", cv::VideoWriter::fourcc('X','2','6','4'), 30, cv::Size(frame_width, frame_height));
    if (!videoWriter.isOpened()) {
        std::cerr << "Error: Unable to create the video writer." << std::endl;
        return -1;
    }

    int recordingDuration = 10; // Desired recording duration in seconds
    auto startTime = std::chrono::steady_clock::now(); // Record start time


    while (true) {
        cv::Mat frame;
        cap.read(frame);

        if (frame.empty()) {
            std::cerr << "Error: Blank frame grabbed." << std::endl;
            break;
        }

        videoWriter.write(frame);

         // Check if the recording duration is reached
        auto currentTime = std::chrono::steady_clock::now();
        auto elapsedTime = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count();
        if (elapsedTime >= recordingDuration)
            break;

        // Press 'q' to exit the loop
        if (cv::waitKey(1) == 'q')
            break;
    }

    // Stop and close the audio stream
    err = Pa_StopStream(audioStream);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
    }

    err = Pa_CloseStream(audioStream);
    if (err != paNoError) {
        std::cerr << "PortAudio error: " << Pa_GetErrorText(err) << std::endl;
    }

    Pa_Terminate();

    cap.release();
    videoWriter.release();
    cv::destroyAllWindows();

    // Save the audio data to a temporary file (binary format)
    FILE* audioFile = std::fopen("temp_audio.bin", "wb");
    if (audioFile) {
        std::fwrite(audioData.data(), sizeof(float), audioData.size(), audioFile);
        std::fclose(audioFile);
    }

    // Use FFmpeg to mux the audio and video into a single video file
    std::string command = "ffmpeg -r 30 -i output_video.avi -f f32le -ar 44100 -ac 1 -i temp_audio.bin -c:v copy -c:a aac output_final.mp4";
    std::system(command.c_str());

    // Remove the temporary audio file
    std::remove("temp_audio.bin");

    return 0;
}
