// Fill out your copyright notice in the Description page of Project Settings.


#include "iFlyBase.h"

#include <string>

#include "NHMac.h"
#include "WebSocketsModule.h"
#include "GenericPlatform/GenericPlatformHttp.h"


void iFlyBase::SetHandler(XFProcessHandler* Handler)
{
	this->AwakenHandler = Handler;
}

bool iFlyBase::IsWorking() const
{
	return status != INITED;
}

void iFlyBase::SetAppSecret(const FString& pAPISecret)
{
	APISecret = pAPISecret;
}

iFlyBase::iFlyBase(): Type(), status(0), session_id(nullptr), sse_hints{}, AwakenHandler(nullptr), lastTickTime(0),
                      WebSocket(nullptr)
{
}

iFlyBase::~iFlyBase()
{
}

void iFlyBase::Exit(int err_code)
{
	if (SDK_STATS::NONE == status)
	{
		return;
	}
	status = SDK_STATS::INITED;
	ExitImp(err_code);
	// Logout();
	if (AwakenHandler != nullptr)
	{
		const FString msg = FString::Printf(TEXT("ErrorCode=%d"), err_code);
		AwakenHandler->AwakenNotify(msg);
		if (err_code == 0)
		{
			AwakenHandler->OnExit(Type, err_code);
		}
	}
}

void iFlyBase::Close()
{
	status = SDK_STATS::NONE;
	AwakenHandler = nullptr;
	this->Exit(-1);
}

bool iFlyBase::OnTick()
{
	const double dnow = FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64());
	if (lastTickTime == 0 || dnow - lastTickTime < 200)
	{
		return false;
	}
	lastTickTime = dnow;
	return true;
}

FString iFlyBase::GetAuthString(const FString& Key, const FString& Data)
{
	std::string hmac = NHMac::cHMacSha256(Key, Data);
	TArray<uint8> datas;
	datas.AddUninitialized(32);
	FMemory::Memcpy(datas.GetData(), hmac.c_str(), 32);

	FString strBase64 = FBase64::Encode(datas);
	return strBase64;
}

FString iFlyBase::GetReqUrl()
{
	FString Host = GetHost();
	FString HostPath = GetHostPath();
	FString Protocol = GetProtocol();
	
	FString httpDateStr = FDateTime::UtcNow().ToHttpDate();
	FString sign = FString::Printf(TEXT("host: %s\ndate: %s\nGET %s HTTP/1.1"), *Host, *httpDateStr, *HostPath);

	const FString sha = GetAuthString(APISecret, sign);

	FString AppID = this->my_app_login_params;
	FString AppKey = this->my_session_begin_params;
	FString authUrl = FString::Printf(
		TEXT("api_key=\"%s\", algorithm=\"%s\", headers=\"%s\", signature=\"%s\""), *AppKey,
		TEXT("hmac-sha256"), TEXT("host date request-line"), *sha);

	const FString authorization = FBase64::Encode(authUrl);
	// UE_LOG(LogTemp, Warning, TEXT("authorization: %s"), *authorization);

	const FString relHost = FString::Printf(TEXT("%s://%s%s"), *Protocol, *Host, *HostPath);
	
	httpDateStr = FGenericPlatformHttp::UrlEncode(httpDateStr);
	FString rHost = FString::Printf(
		TEXT("%s?host=%s&date=%s&authorization=%s"), *relHost, *Host, *httpDateStr, *authorization);
	return rHost;
}

FString iFlyBase::GetProtocol() const
{
	return TEXT("ws");
}

FString iFlyBase::GetHost() const
{
	return TEXT("");
}

FString iFlyBase::GetHostPath() const
{
	return TEXT("");
}

void iFlyBase::CreateWebSocket(const FString& pHost, const FString& pServerProtocal)
{
	FModuleManager::Get().LoadModuleChecked("WebSockets");
	const TSharedRef<IWebSocket> WebSocket1 = FWebSocketsModule::Get().CreateWebSocket(pHost, pServerProtocal);
	WebSocket = WebSocket1.operator->();

	WebSocket1->OnConnected().AddLambda([this]()
	{
		OnConnected();
	});
	WebSocket1->OnConnectionError().AddLambda([this](const FString& Message)
	{
		OnConnectionError(Message);
	});
	WebSocket1->OnMessage().AddLambda([this](const FString& Message)
	{
		OnSocketMessage(Message);
	});
	WebSocket1->OnMessageSent().AddLambda([this](const FString& Message)
	{
		OnMessageSend(Message);
	});
	WebSocket1->OnClosed().AddLambda([this](int32 StatusCode, const FString& Reason, bool bWasClean)
	{
		OnSocketClose(StatusCode, Reason, bWasClean);
	});

	WebSocket1->Connect();
}

void iFlyBase::OnSocketClose(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	UE_LOG(LogTemp, Warning, TEXT("OnSocketClose:Status=%d, Reason=%s"), StatusCode, *Reason);
}

void iFlyBase::OnSocketMessage(const FString& Message)
{
}

void iFlyBase::OnConnected()
{
	UE_LOG(LogTemp, Warning, TEXT("OnConnected:%s"), *FString(__FUNCTION__));
}

void iFlyBase::OnConnectionError(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("OnConnectionError:%s"), *Message);
}

void iFlyBase::OnMessageSend(const FString& Message)
{
	UE_LOG(LogTemp, Warning, TEXT("OnMessageSend:%s"), *Message);
}

bool iFlyBase::Init(const FString& appParam, const FString& session_begin_params)
{
	if (this->status != SDK_STATS::NONE && this->status != SDK_STATS::CLOSED)
	{
		return true;
	}
	this->my_app_login_params = appParam;
	this->my_session_begin_params = session_begin_params;
	status = SDK_STATS::INITED;
	return true;
}

int32 iFlyBase::BeginSession()
{
	if (status != STARTED && status != INITED)
	{
		return -1;
	}
	lastTickTime = FPlatformTime::ToMilliseconds64(FPlatformTime::Cycles64());
	status = SDK_STATS::WORKING;
	return 0;
}

int32 iFlyBase::EndSession()
{
	lastTickTime = 0;
	if (status == INITED)
	{
		return -1;
	}
	status = SDK_STATS::INITED;
	return 0;
}
