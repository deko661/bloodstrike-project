#include "audio.hpp"
#include "audio_embedded.hpp"
#include <Windows.h>
#include <mmsystem.h>
#include <fstream>
#include <string>
#include <thread>
#include <atomic>

#pragma comment(lib, "winmm.lib")

static std::atomic<bool> playing{false};
static std::string tempFilePath;
static std::thread audioThread;

// Extract embedded MP3 to temp file and play it
static void PlayFromTempFile() {
    // Create temp file path
    char tempPath[MAX_PATH];
    GetTempPathA(MAX_PATH, tempPath);
    tempFilePath = std::string(tempPath) + "deko_intro.mp3";
    
    // Write MP3 data to temp file
    std::ofstream file(tempFilePath, std::ios::binary);
    if (!file) return;
    file.write((const char*)intro_mp3_data, intro_mp3_size);
    file.close();
    
    // Play using MCI
    std::string openCmd = "open \"" + tempFilePath + "\" type mpegvideo alias introMusic";
    mciSendStringA(openCmd.c_str(), NULL, 0, NULL);
    mciSendStringA("play introMusic wait", NULL, 0, NULL); // 'wait' makes it block until done
    
    // When playback finishes, close and cleanup
    mciSendStringA("close introMusic", NULL, 0, NULL);
    DeleteFileA(tempFilePath.c_str());
    playing = false;
}

bool audio::Init() {
    return true; // MCI doesn't need initialization
}

void audio::PlayIntroMusic() {
    if (playing) return;
    playing = true;
    
    // Start playback in a separate thread so it doesn't block
    audioThread = std::thread(PlayFromTempFile);
    audioThread.detach();
}

void audio::Stop() {
    if (playing) {
        mciSendStringA("stop introMusic", NULL, 0, NULL);
        mciSendStringA("close introMusic", NULL, 0, NULL);
        DeleteFileA(tempFilePath.c_str());
        playing = false;
    }
}

void audio::Shutdown() {
    Stop();
}

bool audio::IsPlaying() {
    return playing;
}
