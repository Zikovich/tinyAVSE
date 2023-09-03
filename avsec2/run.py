import onnxruntime
import cv2
import librosa
import numpy as np
import soundfile as sf

def get_frames(path):
    frames_list = []
    vidcap = cv2.VideoCapture(path)
    success,image = vidcap.read()
    while success:
        frame = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        frames_list.append(frame)
        success,image = vidcap.read()
    return frames_list

mp4_file = "silent.mp4"
audio = "noisy.wav"

ort_session = onnxruntime.InferenceSession("model.onnx", providers=['CPUExecutionProvider'])

audio_data = librosa.load(audio, sr=16000)[0][np.newaxis,...]
video_data = np.array(get_frames(mp4_file)).astype(np.float32)[np.newaxis, np.newaxis, ...]
video_data /= 255

data = {"noisy_audio": audio_data, "video_frames": video_data}

enhanced = ort_session.run(None, data)[0][0]
sf.write("enhanced.wav", enhanced, 16000)
