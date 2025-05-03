#include <Utilty/miniaudio.h>

#include "./MusicPlayer.h"

namespace CG
{
	MusicPlayer::MusicPlayer() {
		isPlaying = false;
		hasSound = false;

		if (ma_engine_init(NULL, &engine) != MA_SUCCESS) {
			printf("無法初始化 miniaudio 引擎\n");
		}
	}
	MusicPlayer::~MusicPlayer() {
		if (hasSound) {
			ma_sound_uninit(&currentSound);
		}
		ma_engine_uninit(&engine);
	}

	bool MusicPlayer::Play(const std::string& filepath)	{
		if (hasSound) {
			ma_sound_stop(&currentSound);
			ma_sound_uninit(&currentSound);
			hasSound = false;
		}

		if (ma_sound_init_from_file(&engine, filepath.c_str(), 0, NULL, NULL, &currentSound) == MA_SUCCESS) {
			ma_sound_start(&currentSound);
			currentTrack = filepath;
			isPlaying = true;
			hasSound = true;
			return true;
		}

		return false;
	}

	void MusicPlayer::Stop() {
		if (hasSound) {
			ma_sound_stop(&currentSound);
			ma_sound_uninit(&currentSound);
			hasSound = false;
		}
		isPlaying = false;
		currentTrack = "";
	}

	void MusicPlayer::SetVolume(float volume) {
		ma_engine_set_volume(&engine, volume);
	}

	bool MusicPlayer::IsPlaying() const {
		return isPlaying;
	}

	void MusicPlayer::SetLooping(bool loop) {
		if (hasSound) {
			ma_sound_set_looping(&currentSound, loop ? MA_TRUE : MA_FALSE);
		}
	}

	const std::string& MusicPlayer::GetCurrentTrack() const {
		return currentTrack;
	}
}