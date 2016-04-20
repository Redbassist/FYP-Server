#include "VoiceReceiver.h"

static bool instanceFlag = false;
static VoiceReceiver* instance = NULL;

VoiceReceiver* VoiceReceiver::GetInstance() {
	if (!instanceFlag) {
		instance = new VoiceReceiver();
		instanceFlag = true;
	}
	return instance;
}

VoiceReceiver::VoiceReceiver() {
	voice = NULL;
	hr = CoInitializeEx(0, COINIT_MULTITHREADED);
	Reset();
}

void VoiceReceiver::Reset() {
	if (cpStream != NULL) {
		cpStream->Close();
		cpStream = NULL;

	}
	if (voice != NULL) {
		ReleaseVoice();
		voice = NULL;
	}
	hr = CoCreateInstance(CLSID_SpVoice, NULL, CLSCTX_ALL, IID_ISpVoice, (void **)&voice);

	if (SUCCEEDED(hr))
	{
		hr = cAudioFmt.AssignFormat(SPSF_22kHz16BitMono);
	}

	//Call SPBindToFile, a SAPI helper method,  to bind the audio stream to the file
	if (SUCCEEDED(hr))
	{
		hr = SPBindToFile(L"test.wav", SPFM_CREATE_ALWAYS,
			&cpStream, &cAudioFmt.FormatId(), cAudioFmt.WaveFormatExPtr());
	}

	//set the output to cpStream so that the output audio data will be stored in cpStream
	if (SUCCEEDED(hr))
	{
		hr = voice->SetOutput(cpStream, TRUE);
	}
}

void VoiceReceiver::ReleaseVoice() {
	voice->Release();
}

void VoiceReceiver::CloseStream() {
	voice->Release();
	if (cpStream != NULL) {
		cpStream->Close();
		cpStream = NULL;
	}
}

void VoiceReceiver::WriteTextToFile(std::string text) {
	Reset();
	if (SUCCEEDED(hr))
	{
		size_t size = strlen(text.c_str()) + 1;
		wchar_t* wtext = new wchar_t[size];
		size_t outSize;
		mbstowcs_s(&outSize, wtext, size, text.c_str(), size - 1);
		LPWSTR ptr = wtext;
		hr = voice->Speak(wtext, SPF_DEFAULT, NULL);
	} 
	CloseStream();
	AudioManager::GetInstance()->playVoice();
}