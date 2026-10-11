#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <cstdint>
#include <algorithm>
#include <cmath>

namespace prophet6::test {

// Lightweight, zero-dependency 16-bit PCM Stereo WAV Writer
class WavWriter {
public:
    static bool writeWavStereo(
        const std::string& filename,
        const std::vector<float>& leftChannel,
        const std::vector<float>& rightChannel,
        uint32_t sampleRate = 44100)
    {
        if (leftChannel.empty() || leftChannel.size() != rightChannel.size()) {
            return false;
        }

        std::ofstream file(filename, std::ios::binary);
        if (!file.is_open()) {
            return false;
        }

        const uint32_t numSamples = static_cast<uint32_t>(leftChannel.size());
        constexpr uint16_t numChannels = 2;
        constexpr uint16_t bitsPerSample = 16;
        constexpr uint16_t blockAlign = numChannels * (bitsPerSample / 8);
        const uint32_t byteRate = sampleRate * blockAlign;
        const uint32_t dataBytes = numSamples * blockAlign;
        const uint32_t riffChunkSize = 36 + dataBytes;

        // RIFF Header
        file.write("RIFF", 4);
        writeU32(file, riffChunkSize);
        file.write("WAVE", 4);

        // Format chunk ("fmt ")
        file.write("fmt ", 4);
        writeU32(file, 16); // Subchunk1Size for PCM
        writeU16(file, 1);  // AudioFormat = 1 (PCM)
        writeU16(file, numChannels);
        writeU32(file, sampleRate);
        writeU32(file, byteRate);
        writeU16(file, blockAlign);
        writeU16(file, bitsPerSample);

        // Data chunk ("data")
        file.write("data", 4);
        writeU32(file, dataBytes);

        // Interleaved 16-bit PCM samples
        for (size_t i = 0; i < numSamples; ++i) {
            // Hard clamp to [-1.0, 1.0] and scale to int16
            const float sampleL = std::clamp(leftChannel[i], -1.0f, 1.0f);
            const float sampleR = std::clamp(rightChannel[i], -1.0f, 1.0f);

            const int16_t pcmL = static_cast<int16_t>(sampleL * 32767.0f);
            const int16_t pcmR = static_cast<int16_t>(sampleR * 32767.0f);

            file.write(reinterpret_cast<const char*>(&pcmL), sizeof(pcmL));
            file.write(reinterpret_cast<const char*>(&pcmR), sizeof(pcmR));
        }

        return file.good();
    }

private:
    static void writeU32(std::ofstream& f, uint32_t v) {
        f.put(static_cast<char>(v & 0xFF));
        f.put(static_cast<char>((v >> 8) & 0xFF));
        f.put(static_cast<char>((v >> 16) & 0xFF));
        f.put(static_cast<char>((v >> 24) & 0xFF));
    }

    static void writeU16(std::ofstream& f, uint16_t v) {
        f.put(static_cast<char>(v & 0xFF));
        f.put(static_cast<char>((v >> 8) & 0xFF));
    }
};

} // namespace prophet6::test
