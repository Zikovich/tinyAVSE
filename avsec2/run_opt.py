import onnxruntime
import cv2
import librosa
import numpy as np
import soundfile as sf
import torch
from torch.profiler import profile, record_function, ProfilerActivity
from pesq import pesq
from pystoi import stoi
from torchaudio.pipelines import SQUIM_OBJECTIVE, SQUIM_SUBJECTIVE

from memory_profiler import profile

import torchaudio
import torchaudio.functional as F

import onnx
import onnx.optimizer



def si_snr(estimate, reference, epsilon=1e-8):
    estimate = estimate - estimate.mean()
    reference = reference - reference.mean()
    reference_pow = reference.pow(2).mean(axis=1, keepdim=True)
    mix_pow = (estimate * reference).mean(axis=1, keepdim=True)
    scale = mix_pow / (reference_pow + epsilon)

    reference = scale * reference
    error = estimate - reference

    reference_pow = reference.pow(2)
    error_pow = error.pow(2)

    reference_pow = reference_pow.mean(axis=1)
    error_pow = error_pow.mean(axis=1)

    si_snr = 10 * torch.log10(reference_pow) - 10 * torch.log10(error_pow)
    return si_snr.item()

def get_frames(path):
    frames_list = []
    vidcap = cv2.VideoCapture(path)
    success,image = vidcap.read()
    while success:
        frame = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)
        frames_list.append(frame)
        success,image = vidcap.read()
    return frames_list

mp4_file = "S37890_silent.mp4"
audio = "S37890_mixed.wav"


# load model
model = onnx.load("model.onnx")

# optimize
#model = onnx.optimizer.optimize(model, ['fuse_bn_into_conv'] )
onnx.helper.strip_doc_string(model)
model = onnx.optimizer.optimize(
    model,
    [
        "eliminate_deadend",
        "eliminate_identity",
        "eliminate_nop_dropout",
        "eliminate_nop_monotone_argmax",
        "eliminate_nop_pad",
        "extract_constant_to_initializer",
        "eliminate_unused_initializer",
        "eliminate_nop_transpose",
        "fuse_add_bias_into_conv",
        "fuse_consecutive_log_softmax",
        "fuse_consecutive_reduce_unsqueeze",
        "fuse_consecutive_squeezes",
        "fuse_consecutive_transposes",
        "fuse_matmul_add_bias_into_gemm",
        "fuse_pad_into_conv",
        "fuse_transpose_into_gemm",
    ],
    fixed_point=True,
)
# save optimized model
with open('model.opt.onnx', "wb") as f:
    f.write(model.SerializeToString())



options = onnxruntime.SessionOptions()
options.enable_profiling=True
ort_session = onnxruntime.InferenceSession("model.opt.onnx",options,  providers=['CPUExecutionProvider'])

audio_data = librosa.load(audio, sr=16000)[0][np.newaxis,...]
video_data = np.array(get_frames(mp4_file)).astype(np.float32)[np.newaxis, np.newaxis, ...]
video_data /= 255

data = {"noisy_audio": audio_data, "video_frames": video_data}

@profile
def predict():

    enhanced = ort_session.run(None, data)[0][0]
    sf.write("S37890_enhanced.wav", enhanced, 16000)

predict()
#prof_file = sess_profile.end_profiling()
#print(prof_file)

WAVEFORM_ENHANCED, SAMPLE_RATE_SPEECH = torchaudio.load('S37890_enhanced_opt.wav')
WAVEFORM_NOISY, SAMPLE_RATE_SPEECH_NOISY = torchaudio.load('S37890_mixed.wav')
WAVEFORM_TAREGT, SAMPLE_RATE_SPEECH_TARGET = torchaudio.load('S37890_target.wav')
objective_model = SQUIM_OBJECTIVE.get_model()

stoi_hyp, pesq_hyp, si_sdr_hyp = objective_model(WAVEFORM_ENHANCED[0:1, :])
print(f"Estimated metrics for distorted speech are\n")
print(f"STOI: {stoi_hyp[0]}")
print(f"PESQ: {pesq_hyp[0]}")
print(f"SI-SDR: {si_sdr_hyp[0]}\n")

pesq_ref = pesq(16000, WAVEFORM_TAREGT[0].numpy(), WAVEFORM_ENHANCED[0].numpy(), mode="wb")
stoi_ref = stoi(WAVEFORM_TAREGT[0].numpy(), WAVEFORM_ENHANCED[0].numpy(), 16000, extended=False)
si_sdr_ref = si_snr(WAVEFORM_ENHANCED[0:1], WAVEFORM_TAREGT)
print(f"Reference metrics for distorted speech  are\n")
print(f"STOI: {stoi_ref}")
print(f"PESQ: {pesq_ref}")
print(f"SI-SDR: {si_sdr_ref}")

