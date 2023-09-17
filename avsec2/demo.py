import onnxruntime
import cv2
import librosa
import numpy as np
import soundfile as sf
import subprocess
import multiprocessing

def get_frames(path):
    frames_list = []
    vidcap = cv2.VideoCapture(path)
    success,image = vidcap.read()
    while success:
        frame = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        frames_list.append(frame)
        success,image = vidcap.read()
    return frames_list


# Define the shell commands you want to execute
#command1 = "gst-launch-1.0 -e v4l2src device=/dev/video0 ! videoconvert ! omxh264enc ! qtmux ! filesink location=silent.mp4"
command1 = "./record_video_avi"
command2 = "arecord -D plughw:2,0 --duration=4 audio.wav"

# Function to execute a shell command
def run_command(command):
    subprocess.run(command, shell=True)

# Create two separate processes for each command
process1 = multiprocessing.Process(target=run_command, args=(command1,))
process2 = multiprocessing.Process(target=run_command, args=(command2,))

print("Start recording ....")
# Start both processes
process1.start()
process2.start()

# Wait for both processes to finish, but only for 2 seconds
#process1.join()
process2.join()

# Check if processes are still alive and terminate them if needed
if process1.is_alive():
    process1.terminate()
if process2.is_alive():
    process2.terminate()
process1.terminate()

proc1Terminate = ["pkill", "record_video_av"]
subprocess.run(proc1Terminate, check=True)
# Both processes have finished (or been terminated)
print("Both commands have completed or were terminated after 2 seconds.")

################

# Define the command to execute
command = ["aplay", "audio.wav", "-D", "plughw:CARD=Device,DEV=0"]

# Execute the command
try:
    subprocess.run(command, check=True)
except subprocess.CalledProcessError as e:
    print(f"Error: {e}")

#####


mp4_file = "S37890_silent.mp4"
audio = "audio.wav"#"S37890_mixed.wav"

options = onnxruntime.SessionOptions()
options.enable_profiling=False #True
#options.graph_optimization_level = onnxruntime.GraphOptimizationLevel.ORT_DISABLE_ALL

coreProvider = ['CPUExecutionProvider']#['CUDAExecutionProvider', 'CPUExecutionProvider']
ort_session = onnxruntime.InferenceSession("model-sim.onnx",sess_options=options, providers=coreProvider)

audio_data = librosa.load(audio, sr=16000)[0][np.newaxis,...]
video_data = np.array(get_frames(mp4_file)).astype(np.float32)[np.newaxis, np.newaxis, ...]
video_data /= 255

data = {"noisy_audio": audio_data, "video_frames": video_data}

enhanced = ort_session.run(None, data)[0][0]
sf.write("enhanced.wav", enhanced, 16000)

# Load your .wav file
audio_file = "enhanced.wav"

# Define the command to execute
command = ["aplay", "enhanced.wav", "-D", "plughw:CARD=Device,DEV=0"]

# Execute the command
try:
    subprocess.run(command, check=True)
except subprocess.CalledProcessError as e:
    print(f"Error: {e}")
