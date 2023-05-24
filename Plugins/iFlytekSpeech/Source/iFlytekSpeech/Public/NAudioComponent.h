// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/AudioComponent.h"
#include "NAudioComponent.generated.h"

/**
 * 
 */
UCLASS()
class IFLYTEKSPEECH_API UNAudioComponent : public UAudioComponent
{
	GENERATED_BODY()
public:
	UNAudioComponent();
	void SetAudioData(const void* ByteArrayAudioData, int32 len);
	void StartPlay();
private:
	UPROPERTY()
	class USoundWaveProcedural* SoundWaveProcedural;
};
