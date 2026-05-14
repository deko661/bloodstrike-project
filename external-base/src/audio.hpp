#pragma once

// Simple audio player for intro MP3
namespace audio {
    // Initialize audio system
    bool Init();
    
    // Play the embedded intro MP3
    void PlayIntroMusic();
    
    // Stop audio
    void Stop();
    
    // Cleanup
    void Shutdown();
    
    // Check if still playing
    bool IsPlaying();
}
