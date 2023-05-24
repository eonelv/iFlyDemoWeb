// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NAudioComponent.h"
#include "GameFramework/Actor.h"
#include "NAudioPlayer.generated.h"

UCLASS()
class IFLYTEKSPEECH_API ANAudioPlayer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ANAudioPlayer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;
public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	UNAudioComponent* nAudioComponent;
};
