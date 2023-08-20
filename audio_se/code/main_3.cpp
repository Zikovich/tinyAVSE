#include <iostream>
#include <vector>
#include <audio>


int main() {
  // Create a capture context.
  std::audio::capture::context context;
  context.sample_rate = 44100;
  context.channels = 1;
  context.format = std::audio::format::s16;

  // Start capturing audio data.
  std::vector<int16_t> audio_data;
  context.start(audio_data);

  // Wait for a few seconds.
  std::this_thread::sleep_for(std::chrono::seconds(5));

  // Stop capturing audio data.
  context.stop();

  // Play back the audio data.
  std::audio::play::context play_context;
  play_context.sample_rate = 44100;
  play_context.channels = 1;
  play_context.format = std::audio::format::s16;
  play_context.play(audio_data);

  return 0;
}