// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "iFlyBase.h"
#include "iFWinRec.h"
#include "iFlyHeader.h"

/**
 * 
 */
class IFLYTEKSPEECH_API iFlyRtasrHelper : public iFlyBase
{
public:
	static iFlyRtasrHelper* Get();

protected:
	iFlyRtasrHelper();
	virtual ~iFlyRtasrHelper();
	virtual void ExitImp(int err_code) override;
	virtual FString GetHost() const override;
	virtual FString GetHostPath() const override;
public:
	virtual bool Init(const FString& appParam, const FString& session_begin_params) override;
	int32 Start(const FString& src_text);
	
	virtual int32 BeginSession() override;
	virtual int32 EndSession() override;
	virtual bool OnTick() override;

	void OnBegin(const FString& Message);

	virtual void OnConnected() override;
	virtual void OnSocketClose(int32 StatusCode, const FString& Reason, bool bWasClean) override;
	virtual void OnSocketMessage(const FString& Message) override;
	virtual void OnMessageSend(const FString& Message) override;

	int32 GetErrorCode() const;

	bool SendData(bool bClose, bool bForce);
	FString CreatePackage(TArray<uint8> data, int pStatus);
public:
	int Stop();
	int sr_write_audio_data(char *data, unsigned int len);
	void end_sr_on_error(int errcode);
	void wait_for_rec_stop(unsigned int timeout_ms);
	void end_sr_on_vad();
private:
	static iFlyRtasrHelper* ins;
	speech_rec* sr;
	iFWinRec* WinRec;
	int Seq;
	TArray<uint8> recordDatas;
	int64 totalLength;
	FCriticalSection mutex;
};
