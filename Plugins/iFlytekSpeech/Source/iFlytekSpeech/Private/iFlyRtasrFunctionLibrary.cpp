// Fill out your copyright notice in the Description page of Project Settings.


#include "iFlyRtasrFunctionLibrary.h"
#include "iFlyRtasrHelper.h"
#include "iFlyTtsHelper.h"
#include "NAudioComponent.h"

XFRatsrHandler* XFRatsrHandler::ins = nullptr;
XFRatsrHandler* XFRatsrHandler::Get()
{
	if (nullptr == ins)
	{
		ins = new XFRatsrHandler();
	}
	return ins;
}

void XFRatsrHandler::OnSpeechMessage(const FString msg, const FString msg2)
{
	UiFlyRtasrFunctionLibrary::OnRatsrMessage(msg, msg2);
}

void XFRatsrHandler::OnSessionEnd(int32 type, int32 ErrorCode)
{
	UiFlyRtasrFunctionLibrary::OnSessionEnd(type, ErrorCode);
}

void XFRatsrHandler::OnData(const void* datas, int32 len)
{
	UiFlyRtasrFunctionLibrary::OnTtsData(datas, len);
}

void XFRatsrHandler::OnError(int32 type, int32 code)
{
	FString msg = FString::Printf(TEXT("There is something wrong. Type=%d. ErrorCode=%d"), type, code);
	UiFlyRtasrFunctionLibrary::OnRatsrMessage(msg, TEXT(""));
}

UNAudioComponent* UiFlyRtasrFunctionLibrary::AudioComponent = nullptr;
FRatsrMessageHandler UiFlyRtasrFunctionLibrary::MessageHandler;
TArray<FString> UiFlyRtasrFunctionLibrary::Messages;
TArray<FString> UiFlyRtasrFunctionLibrary::MessagesEn;
FTimerHandle UiFlyRtasrFunctionLibrary::TimerHandle;
UObject* UiFlyRtasrFunctionLibrary::TargetObject = nullptr;

void UiFlyRtasrFunctionLibrary::InitRatsr(const FString& AppID, const FString& AppKey, const FString& ApiSeceret)
{
	iFlyRtasrHelper::Get()->SetHandler(XFRatsrHandler::Get());
	iFlyRtasrHelper::Get()->SetAppSecret(ApiSeceret);
	iFlyRtasrHelper::Get()->Init(AppID, AppKey);
}

void UiFlyRtasrFunctionLibrary::StartRatsr(UObject* WorldContextObject)
{
	if (WorldContextObject == nullptr)
	{
		return;
	}
	iFlyRtasrHelper::Get()->Start(TEXT(""));
	
	TargetObject = WorldContextObject;
	if (!TimerHandle.IsValid())
	{
		TargetObject->GetWorld()->GetTimerManager().SetTimer(TimerHandle,
			[]()
			{
				iFlyRtasrHelper::Get()->OnTick();
			},0.04, true);
	}
}

void UiFlyRtasrFunctionLibrary::StopRatsr()
{
	iFlyRtasrHelper::Get()->Stop();
	if (TimerHandle.IsValid())
	{
		TargetObject->GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
		TargetObject = nullptr;
	}
}

void UiFlyRtasrFunctionLibrary::OnRatsrMessage(const FString& Message, const FString& Message2)
{
	Messages.Emplace(Message);
	MessagesEn.Emplace(Message2);
	FString savePath = FPaths::ProjectPersistentDownloadDir() + "/ratsr.txt";
	FFileHelper::SaveStringArrayToFile(Messages, *savePath);
	
	savePath = FPaths::ProjectPersistentDownloadDir() + "/ratsr_en.txt";
    FFileHelper::SaveStringArrayToFile(MessagesEn, *savePath);
	MessageHandler.ExecuteIfBound(Message, Message2);
}

void UiFlyRtasrFunctionLibrary::OnSessionEnd(int32 type, int32 ErrorCode)
{
	FString savePath = FPaths::ProjectPersistentDownloadDir() + "/ratsr_bak.txt";
	FFileHelper::SaveStringArrayToFile(Messages, *savePath);

	savePath = FPaths::ProjectPersistentDownloadDir() + "/ratsr_en_bak.txt";
	FFileHelper::SaveStringArrayToFile(MessagesEn, *savePath);
	Messages.Empty();
	MessagesEn.Empty();
}

void UiFlyRtasrFunctionLibrary::SetMessageHandler(FRatsrMessageHandler pHandler)
{
	MessageHandler = MoveTemp(pHandler);
}

void UiFlyRtasrFunctionLibrary::InitTts(const FString& AppID, const FString& AppKey, const FString& APISecret)
{
	iFlyTtsHelper::Get()->SetHandler(XFRatsrHandler::Get());
	iFlyTtsHelper::Get()->SetAppSecret(APISecret);
	iFlyTtsHelper::Get()->Init(AppID, AppKey);
}

void UiFlyRtasrFunctionLibrary::StartTts(const FString& srcText)
{
	iFlyTtsHelper::Get()->Start(srcText);
}

void UiFlyRtasrFunctionLibrary::StopTts()
{
	iFlyTtsHelper::Get()->Stop();
}

void UiFlyRtasrFunctionLibrary::SetVoice(const FString& pVoice)
{
	iFlyTtsHelper::Get()->SetVoice(pVoice);
}

void UiFlyRtasrFunctionLibrary::SetAudioComponent(UNAudioComponent* pAudioComponent)
{
	UiFlyRtasrFunctionLibrary::AudioComponent = pAudioComponent;
}

void UiFlyRtasrFunctionLibrary::OnTtsData(const void* datas, int32 len)
{
	if (nullptr != AudioComponent)
	{
		AudioComponent->SetAudioData(datas, len);
	}
}
