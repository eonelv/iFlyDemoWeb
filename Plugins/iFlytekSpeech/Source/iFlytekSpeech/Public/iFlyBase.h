// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "iFlyHeader.h"
#include "IWebSocket.h"

/**
 * 
 */
class IFLYTEKSPEECH_API iFlyBase
{
public:
	virtual bool Init(const FString& appParam, const FString& session_begin_params);
	virtual int32 BeginSession();
	virtual int32 EndSession();
	void Exit(int err_code);
	
	void SetHandler(XFProcessHandler* Handler);
	bool IsWorking() const;

	void SetAppSecret(const FString& pAPISecret);
protected:
	iFlyBase();
	~iFlyBase();
	virtual void ExitImp(int err_code) = 0;
	
	virtual void Close();
	virtual bool OnTick();
	virtual FString GetAuthString(const FString& Key, const FString & Data);
	virtual FString GetReqUrl();
	virtual FString Sha256(const FString & Data);

	virtual FString GetProtocol() const;
	virtual FString GetHost() const;
	virtual FString GetHostPath() const;
	virtual void CreateWebSocket(const FString& pHost, const FString & pServerProtocal);

	virtual void OnSocketClose(int32 StatusCode, const FString& Reason, bool bWasClean);
	virtual void OnSocketMessage(const FString& Message);
	virtual void OnConnected();
	virtual void OnConnectionError(const FString& Message);
	virtual void OnMessageSend(const FString& Message);
protected:
	XF_SDK_FUN_TYPE Type;
	int status;
	const char *session_id;
	char sse_hints[128];
	FString my_app_login_params;
	FString my_session_begin_params;
	XFProcessHandler* AwakenHandler;
	double lastTickTime;
	FString srcText;
	FString APISecret;
	IWebSocket* WebSocket;
};
