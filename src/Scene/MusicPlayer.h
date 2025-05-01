#pragma once
#include <Utilty/miniaudio.h>
#include <string>
#include <vector>
#include <stdio.h>

namespace CG
{
    class MusicPlayer {
    public:
        MusicPlayer();
        ~MusicPlayer();

        bool Play(const std::string& filepath);
        void Stop();
        void SetVolume(float volume);
        void SetLooping(bool loop);
        bool IsPlaying() const;
        const std::string& GetCurrentTrack() const;

    private:
        ma_engine engine;
        std::string currentTrack;
        ma_sound currentSound;
        bool hasSound = false;
        bool isPlaying;
    };
}