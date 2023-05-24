// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "iFlyHeader.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "iFlyRtasrFunctionLibrary.generated.h"

DECLARE_DYNAMIC_DELEGATE_TwoParams(FRatsrMessageHandler, FString, Value, FString, Value2);

class XFRatsrHandler : public XFHandlerAdapter
{
public:
	static XFRatsrHandler* Get();
public:
	virtual void OnSpeechMessage(const FString msg, const FString msg2=TEXT("")) override;
	virtual void OnSessionEnd(int32 type, int32 ErrorCode) override;
	virtual void OnData(const void* datas, int32 len) override;
	virtual void OnError(int32 type, int32 code) override;
private:
	static XFRatsrHandler* ins;
};

class UNAudioComponent;
/**
 * 
 */
UCLASS()
class IFLYTEKSPEECH_API UiFlyRtasrFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category="XF|Ratsr")
	static void InitRatsr(const FString& AppID, const FString& AppKey, const FString& APISecret);
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject") , Category="XF|Ratsr")
	static void StartRatsr(UObject* WorldContextObject);
	UFUNCTION(BlueprintCallable, Category="XF|Ratsr")
	static void StopRatsr();

	static void OnRatsrMessage(const FString& Message, const FString& Message2);
	static void OnSessionEnd(int32 type, int32 ErrorCode);
	
	UFUNCTION(BlueprintCallable, Category="iFly|tts")
	static void SetMessageHandler(FRatsrMessageHandler pHandler);

	UFUNCTION(BlueprintCallable, Category="iFly|tts")
	static void InitTts(const FString& AppID, const FString& AppKey, const FString& APISecret);
	
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject") , Category="iFly|tts")
	static void StartTts(const FString& srcText);
	
	UFUNCTION(BlueprintCallable, Category="iFly|tts")
	static void StopTts();

	UFUNCTION(BlueprintCallable, Category="iFly|tts")
	static void SetVoice(const FString& pVoice);
	
	UFUNCTION(BlueprintCallable, Category="iFly|tts")
	static void SetAudioComponent(UNAudioComponent* pAudioComponent);
	static void OnTtsData(const void* datas, int32 len);
private:
	static FRatsrMessageHandler MessageHandler;
	static TArray<FString> Messages;
	static TArray<FString> MessagesEn;

	static FTimerHandle TimerHandle;
	static UObject* TargetObject;

	static UNAudioComponent* AudioComponent;
};
