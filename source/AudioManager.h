#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <xaudio2.h>
#include <vector>

namespace Audio
{
	void Initialize();
	void Update();

	class MusicData
	{
		friend 	MusicData* LoadMusic(const char* tag, const char* filename);

		IXAudio2SourceVoice* source_voice = nullptr;
		std::vector<unsigned char> data;
		bool loop_flg = false;
		bool playing = false;

	public:
		~MusicData();
		void Update();

		bool CheckPlaying() { return playing; }
		void Play(bool loop = false);
		void Stop();
		void Resume();
		void SetVolume(float volume);
		void SetPitch(float pitch);
	};
	MusicData* LoadMusic(const char* tag, const char* filename);
	void UnloadMusic(const char* tag);
	MusicData* GetMusic(const char* tag);



	class SoundData
	{
		friend SoundData* LoadSound(const char* tag, const char* filename);

		static const char sound_max = 16;
		IXAudio2SourceVoice* source_voices[sound_max] = {};
		WAVEFORMATEX format = {};
		std::vector<BYTE> data;
		float volume = 1.0f;
		float pitch = 1.0f;

	public:
		~SoundData();
		void Update();

		void Play();
		void SetVolume(float volume) { this->volume = volume; }
		void SetPitch(float pitch) { this->pitch = pitch; }
	};
	SoundData* LoadSound(const char* tag, const char* filename);
	void UnloadSound(const char* tag);
	SoundData* GetSound(const char* tag);
}

#endif // !AUDIO_MANAGER_H

