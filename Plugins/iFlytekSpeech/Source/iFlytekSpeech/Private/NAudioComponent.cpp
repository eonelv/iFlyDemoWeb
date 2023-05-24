// Fill out your copyright notice in the Description page of Project Settings.


#include "NAudioComponent.h"

#include "Sound/SoundWaveProcedural.h"

UNAudioComponent::UNAudioComponent()
{
}

void UNAudioComponent::SetAudioData(const void* ByteArrayAudioData, int32 len)
{
	if (nullptr == SoundWaveProcedural)
	{
		SoundWaveProcedural = NewObject<USoundWaveProcedural>(this);

		// 设置音频属性
		SoundWaveProcedural->NumChannels = 1;
		SoundWaveProcedural->SetSampleRate(16000);
		SetSound(SoundWaveProcedural);
	}
	SoundWaveProcedural->QueueAudio(static_cast<const uint8*>(ByteArrayAudioData), len);
	StartPlay();
}

void UNAudioComponent::StartPlay()
{
	// 调用UAudioComponent的Play方法
	const bool bPlay = IsPlaying();
	if (!bPlay)
	{
		Play();
	}
}
