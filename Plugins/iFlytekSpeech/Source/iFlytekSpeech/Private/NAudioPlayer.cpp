// Fill out your copyright notice in the Description page of Project Settings.


#include "NAudioPlayer.h"

// Sets default values
ANAudioPlayer::ANAudioPlayer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	nAudioComponent = CreateDefaultSubobject<UNAudioComponent>(TEXT("Root"));
	RootComponent = nAudioComponent;
}

// Called when the game starts or when spawned
void ANAudioPlayer::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ANAudioPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

