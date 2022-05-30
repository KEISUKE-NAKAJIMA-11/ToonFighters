#pragma comment(lib,"xaudio2.lib")

#include "AudioManager.h"

#include <assert.h>
#include <map>
#include <string>

namespace Audio
{
	template<int count>
	bool CheckString(const char* str1, const char* str2)
	{
		for (int i = 0; i < count; i++) {
			if (str1[i] != str2[i])return false;
		}
		return true;
	}

	struct RiffChank
	{
		char riff[4] = {};
		int size = 0;
		char fileType[4] = {};
	};

	struct Chank
	{
		char name[4] = {};
		int size = 0;
	};

	struct AudioManager
	{
		IXAudio2* xaudio = nullptr;
		IXAudio2MasteringVoice* mastering_voice = nullptr;
		std::map<std::string, MusicData> music;
		std::map<std::string, SoundData> sound;

		~AudioManager()
		{
			manager.music.clear();
			manager.sound.clear();

			if (manager.mastering_voice) {
				manager.mastering_voice->DestroyVoice();
				manager.mastering_voice = nullptr;
			}
			if (manager.xaudio) {
				manager.xaudio->Release();
				manager.xaudio = nullptr;
			}
		}
	}manager;

	void Initialize()
	{
		HRESULT hr = S_OK;

		hr = CoInitializeEx(0, COINIT_MULTITHREADED);
		assert(hr == S_OK && "CoInitializeEx failed!");

		hr = XAudio2Create(&manager.xaudio, 0);
		assert(hr == S_OK && "XAudio2Create failed!");

		hr = manager.xaudio->CreateMasteringVoice(&manager.mastering_voice);
		assert(hr == S_OK && "CreateMasteringVoice failed!");
	}

	void Update()
	{
		for (auto& music : manager.music) {
			music.second.Update();
		}
		for (auto& sound : manager.sound) {
			sound.second.Update();
		}
	}

	MusicData* LoadMusic(const char* tag, const char* filename)
	{
		assert(manager.music.find(tag) == manager.music.end() && "Musicタグの重複");

		MusicData& music = manager.music[tag];

		FILE* fp = nullptr;
		fopen_s(&fp, filename, "rb");
		assert(fp && "ファイルが見つかりませんでした");

		RiffChank riff = {};
		{
			fread(&riff, sizeof(riff), 1, fp);
			assert(CheckString<4>(riff.riff, "RIFF"));
			assert(CheckString<4>(riff.fileType, "WAVE"));
		}

		Chank formatH = {};
		WAVEFORMATEX format = {};
		{
			while (true) {
				fread(&formatH, sizeof(formatH), 1, fp);
				if (CheckString<4>(formatH.name, "fmt ")) break;
				else fseek(fp, formatH.size, SEEK_CUR);
			}
			int readSize = sizeof(format) - sizeof(format.cbSize);
			fread(&format, readSize, 1, fp);
			fseek(fp, formatH.size - readSize, SEEK_CUR);
		}

		Chank dataH = {};
		{
			while (true) {
				fread(&dataH, sizeof(dataH), 1, fp);
				if (CheckString<4>(dataH.name, "data")) break;
				else fseek(fp, dataH.size, SEEK_CUR);
			}
			music.data.resize(dataH.size);
			fread(music.data.data(), music.data.size(), 1, fp);
		}
		fclose(fp);

		manager.xaudio->CreateSourceVoice(&music.source_voice, &format);

		return &music;
	}

	void UnloadMusic(const char* tag)
	{
		auto find = manager.music.find(tag);
		if (find == manager.music.end())return;
		manager.music.erase(find);
	}

	MusicData* GetMusic(const char* tag)
	{
		auto find = manager.music.find(tag);
		if (find == manager.music.end())return nullptr;
		return &find->second;
	}

	MusicData::~MusicData()
	{
		Stop();
		while (CheckPlaying());
		source_voice->DestroyVoice();
	}

	void MusicData::Update()
	{
		if (loop_flg)return;

		XAUDIO2_VOICE_STATE state = {};
		source_voice->GetState(&state);
		if (!state.BuffersQueued)Stop();
	}

	void MusicData::Play(bool loop)
	{
		if (playing)return;
		playing = true;
		loop_flg = loop;

		XAUDIO2_BUFFER buffer = {};
		buffer.pAudioData = data.data();
		buffer.AudioBytes = (UINT32)data.size();
		if (loop) buffer.LoopCount = XAUDIO2_LOOP_INFINITE;

		source_voice->FlushSourceBuffers();
		source_voice->Start();
		source_voice->SubmitSourceBuffer(&buffer);
	}

	void MusicData::Stop()
	{
		if (!playing)return;
		playing = false;
		source_voice->Stop();
	}

	void MusicData::Resume()
	{
		if (playing)return;

		XAUDIO2_VOICE_STATE state = {};
		source_voice->GetState(&state);
		if (!state.BuffersQueued)return;

		playing = true;
		source_voice->Start();
	}

	void MusicData::SetVolume(float volume)
	{
		source_voice->SetVolume(volume);
	}

	void MusicData::SetPitch(float pitch)
	{
		source_voice->SetFrequencyRatio(pitch);
	}

	SoundData* LoadSound(const char* tag, const char* filename)
	{
		assert(manager.sound.find(tag) == manager.sound.end() && "Soundタグの重複");

		SoundData& sound = manager.sound[tag];

		FILE* fp = nullptr;
		fopen_s(&fp, filename, "rb");
		assert(fp && "ファイルが見つかりませんでした");

		RiffChank riff = {};
		{
			fread(&riff, sizeof(riff), 1, fp);
			assert(CheckString<4>(riff.riff, "RIFF"));
			assert(CheckString<4>(riff.fileType, "WAVE"));
		}

		Chank formatH = {}; {
			while (true) {
				fread(&formatH, sizeof(formatH), 1, fp);
				if (CheckString<4>(formatH.name, "fmt ")) break;
				else fseek(fp, formatH.size, SEEK_CUR);
			}
			int readSize = sizeof(sound.format) - sizeof(sound.format.cbSize);
			fread(&sound.format, readSize, 1, fp);
			fseek(fp, formatH.size - readSize, SEEK_CUR);
		}

		Chank dataH = {};
		{
			while (true) {
				fread(&dataH, sizeof(dataH), 1, fp);
				if (CheckString<4>(dataH.name, "data")) break;
				else fseek(fp, dataH.size, SEEK_CUR);
			}
			sound.data.resize(dataH.size);
			fread(sound.data.data(), sound.data.size(), 1, fp);
		}

		fclose(fp);

		sound.volume = 1.0f;
		sound.pitch = 1.0f;
		return &sound;
	}

	void UnloadSound(const char* tag)
	{
		if (manager.sound.find(tag) == manager.sound.end())return;
		manager.sound.erase(tag);
	}

	SoundData* GetSound(const char* tag)
	{
		auto find = manager.sound.find(tag);
		if (find == manager.sound.end())return nullptr;
		return &find->second;
	}

	SoundData::~SoundData()
	{
		for (auto& sound : source_voices) {
			if (!sound)continue;
			sound->Stop();
			sound->DestroyVoice();
			sound = nullptr;
		}
	}

	void SoundData::Update()
	{
		XAUDIO2_VOICE_STATE state = {};
		for (auto& sound : source_voices) {
			if (!sound)continue;
			sound->GetState(&state);
			if (state.BuffersQueued)continue;
			sound->Stop();
			sound->DestroyVoice();
			sound = nullptr;
		}
	}

	void SoundData::Play()
	{
		for (auto& sound : source_voices) {
			if (sound)continue;

			manager.xaudio->CreateSourceVoice(&sound, &format);
			XAUDIO2_BUFFER buffer = {};
			buffer.pAudioData = data.data();
			buffer.AudioBytes = (UINT32)data.size();

			sound->SetVolume(volume);
			sound->SetFrequencyRatio(pitch);
			sound->Start();
			sound->SubmitSourceBuffer(&buffer);
			break;
		}
	}

}