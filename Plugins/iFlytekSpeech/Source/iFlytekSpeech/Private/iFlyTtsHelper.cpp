#include "iFlyTtsHelper.h"

iFlyTtsHelper* iFlyTtsHelper::ins = nullptr;

iFlyTtsHelper* iFlyTtsHelper::Get()
{
	if (nullptr == ins)
	{
		ins = new iFlyTtsHelper();
	}
	return ins;
}

iFlyTtsHelper::iFlyTtsHelper()
{
	Type = XF_SDK_FUN_TYPE::TTS_WEB;
	this->Voice = TEXT("x_Steve");
}

iFlyTtsHelper::~iFlyTtsHelper()
{
}

void iFlyTtsHelper::ExitImp(int err_code)
{
}

FString iFlyTtsHelper::GetHost() const
{
	return TEXT("tts-api-sg.xf-yun.com");
}

FString iFlyTtsHelper::GetHostPath() const
{
	return TEXT("/v2/tts");
}

bool iFlyTtsHelper::Init(const FString& appParam, const FString& session_begin_params)
{
	iFlyBase::Init(appParam, session_begin_params);
	return true;
}

int32 iFlyTtsHelper::Start(const FString& src_text)
{
	if (status != INITED)
	{
		return -1;
	}
	text = src_text;
	status = STARTED;

	const FString rHost = GetReqUrl();

	CreateWebSocket(rHost, GetProtocol());
	return 0;
}

int32 iFlyTtsHelper::BeginSession()
{
	int ret = -1;
	if (-1 == iFlyBase::BeginSession())
	{
		return ret;
	}
	SendData(false);
	return 0;
}

int32 iFlyTtsHelper::EndSession()
{
	if (-1 == iFlyBase::EndSession())
	{
		return -1;
	}

	if (nullptr != AwakenHandler)
	{
		AwakenHandler->OnSpeechEnd(0);
		AwakenHandler->OnSessionEnd(Type, 0);
	}
	if (nullptr != WebSocket)
	{
		if (WebSocket->IsConnected())
		{
			WebSocket->Close();
		}
		WebSocket = nullptr;
	}
	return 0;
}

bool iFlyTtsHelper::OnTick()
{
	return iFlyBase::OnTick();
}

void iFlyTtsHelper::SetVoice(const FString& pVoice)
{
	this->Voice = pVoice;
}

void iFlyTtsHelper::OnConnected()
{
	iFlyBase::OnConnected();
	BeginSession();
}

void iFlyTtsHelper::OnSocketClose(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	iFlyBase::OnSocketClose(StatusCode, Reason, bWasClean);
}

void iFlyTtsHelper::OnSocketMessage(const FString& Message)
{
	TSharedRef<TJsonReader<TCHAR>> JsonReader = TJsonReaderFactory<TCHAR>::Create(Message);
	TSharedPtr<FJsonObject> MyJsonObject;
	bool bReaderJsonSuccess = false;
	bReaderJsonSuccess = FJsonSerializer::Deserialize(JsonReader, MyJsonObject);
	if (!bReaderJsonSuccess)
	{
		return;
	}
	const int32 Code = MyJsonObject->GetNumberField(TEXT("code"));
	if (Code != 0)
	{
		AwakenHandler->OnError(Type, Code);
		return;
	}
	TSharedPtr<FJsonObject> DataJson = MyJsonObject->GetObjectField(TEXT("data"));


	if (!DataJson->HasField(TEXT("status")))
	{
		return;
	}
	//0 means start of synthesis, 1 means proceeding of synthesis, 2 means end of synthesis
	int32 jsonStatus = DataJson->GetNumberField(TEXT("status"));
	
	FString audio = DataJson->GetStringField(TEXT("audio"));
	if (audio != TEXT(""))
	{
		TArray<uint8> BinaryData;
		FBase64::Decode(audio, BinaryData);
		// uint8* m = const_cast<uint8*>(BinaryData.GetData());
		// FString recognitionText = UTF8_TO_TCHAR(m);
						
		if (nullptr != AwakenHandler)
		{
			AwakenHandler->OnData(BinaryData.GetData(), BinaryData.Num());
		}
	}
	if (jsonStatus == 2)
	{
		EndSession();
	}
}

void iFlyTtsHelper::OnMessageSend(const FString& Message)
{
}

int32 iFlyTtsHelper::GetErrorCode() const
{
	return -1;
}

void iFlyTtsHelper::SendData(bool bClose)
{
	int tStatus = 1;
	if (bClose)
	{
		tStatus = 2;
	}
	FString DataStr = CreatePackage(text, tStatus);
	WebSocket->Send(DataStr);
}

FString iFlyTtsHelper::CreatePackage(FString& data, int pStatus)
{
	FString jsonString;
	TSharedPtr<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&jsonString);
	Writer->WriteObjectStart();

	TSharedPtr<FJsonObject> object = MakeShareable(new FJsonObject);

	FString AppID = this->my_app_login_params;
	FString strBase64 = FBase64::Encode(data);

	if (pStatus != 2)
	{
		const TSharedPtr<FJsonObject> common = MakeShareable(new FJsonObject);
		common->SetStringField("app_id", AppID);
		object->SetObjectField("common", common);

		const TSharedPtr<FJsonObject> bussiness = MakeShareable(new FJsonObject);
		bussiness->SetStringField("vcn", Voice);
		bussiness->SetStringField("aue", "raw");
		bussiness->SetNumberField("speed", 50);
		
		object->SetObjectField("business", bussiness);
	}

	const TSharedPtr<FJsonObject> DataJsonObject = MakeShareable(new FJsonObject);
	if (pStatus != 2)
	{
		DataJsonObject->SetStringField("text", strBase64);
	}
	DataJsonObject->SetNumberField("status", 2);
	object->SetObjectField("data", DataJsonObject);

	FString DataStr;
	const TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&DataStr);
	FJsonSerializer::Serialize(object.ToSharedRef(), JsonWriter);
	return DataStr;
}

int iFlyTtsHelper::Stop()
{
	if (status != WORKING)
	{
		return -1;
	}
	status = SDK_STATS::STOP;
	int ret = 0;
	const char* rslt = nullptr;
	
	SendData(true);
	return 0;
}
