#include "iFlyItsHelper.h"

#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

iFlyItsHelper* iFlyItsHelper::ins = nullptr;

iFlyItsHelper* iFlyItsHelper::Get()
{
	if (nullptr == ins)
	{
		ins = new iFlyItsHelper();
	}
	return ins;
}

iFlyItsHelper::iFlyItsHelper()
{
	Type = XF_SDK_FUN_TYPE::ITS_WEB;
	this->langFrom = TEXT("cn");
	this->langTo = TEXT("en");
}

iFlyItsHelper::~iFlyItsHelper()
{
}

void iFlyItsHelper::ExitImp(int err_code)
{
}

FString iFlyItsHelper::GetHost() const
{
	return TEXT("its-api-sg.xf-yun.com");
}

FString iFlyItsHelper::GetHostPath() const
{
	return TEXT("/v2/its");
}

FString iFlyItsHelper::GetProtocol() const
{
	return TEXT("http");
}

bool iFlyItsHelper::Init(const FString& appParam, const FString& session_begin_params)
{
	iFlyBase::Init(appParam, session_begin_params);
	return true;
}

int32 iFlyItsHelper::Start(const FString& src_text)
{
	if (status != INITED)
	{
		return -1;
	}
	text = src_text;
	status = STARTED;

	BeginSession();
	
	const FString Host = GetHost();
	const FString HostPath = GetHostPath();
	const FString Protocol = GetProtocol();
	
	FString httpDateStr = FDateTime::UtcNow().ToHttpDate();
	
	const FString body = CreatePackage(text, 1);
	
	const FString digestBase64 = "SHA-256=" + Sha256(body);
	const FString sign = FString::Printf(TEXT("host: %s\ndate: %s\nPOST %s HTTP/1.1\ndigest: %s"), *Host, *httpDateStr, *HostPath, *digestBase64);

	const FString sha = GetAuthString(APISecret, sign);

	const FString AppID = this->my_app_login_params;
	const FString AppKey = this->my_session_begin_params;
	const FString authUrl = FString::Printf(TEXT("api_key=\"%s\", algorithm=\"%s\", headers=\"%s\", signature=\"%s\""),
		*AppKey,
		TEXT("hmac-sha256"),
		TEXT("host date request-line digest"),
		*sha);

	const FString authorization = FBase64::Encode(authUrl);
	const FString relHost = FString::Printf(TEXT("%s://%s%s"), *Protocol, *Host, *HostPath);
	
	//发送HTTP请求
	const TSharedRef<IHttpRequest, ESPMode::ThreadSafe> HttpRequest = FHttpModule::Get().CreateRequest();
	HttpRequest->SetURL(relHost);
	HttpRequest->SetVerb(TEXT("POST"));
	HttpRequest->SetHeader(TEXT("Authorization"), authUrl);
	HttpRequest->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
	HttpRequest->SetHeader(TEXT("Accept"), TEXT("application/json,version=1.0"));
	HttpRequest->SetHeader(TEXT("Host"), Host);
	HttpRequest->SetHeader(TEXT("Date"), httpDateStr);
	HttpRequest->SetHeader(TEXT("Digest"), digestBase64);

	HttpRequest->SetContentAsString(body);
	HttpRequest->OnProcessRequestComplete().BindRaw(this, &iFlyItsHelper::OnLoadComplete);
	HttpRequest->ProcessRequest();
	return 0;
}

int32 iFlyItsHelper::BeginSession()
{
	const int ret = iFlyBase::BeginSession();
	if (-1 == ret)
	{
		return ret;
	}
	return 0;
}

int32 iFlyItsHelper::EndSession()
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
	return 0;
}

void iFlyItsHelper::SetLanguage(const FString& from, const FString& to)
{
	this->langFrom = from;
	this->langTo = to;
}

void iFlyItsHelper::OnSocketMessage(const FString& Message)
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
	TSharedPtr<FJsonObject> ResultDataJson = DataJson->GetObjectField(TEXT("result"));
	TSharedPtr<FJsonObject> TransResultJson = ResultDataJson->GetObjectField(TEXT("trans_result"));
	
	if (nullptr != AwakenHandler)
	{
		const FString src = TransResultJson->GetStringField("src");
		const FString dst = TransResultJson->GetStringField("dst");
		AwakenHandler->OnSpeechMessage(dst, src);
	}
}

void iFlyItsHelper::OnLoadComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccessed)
{
	if (!HttpRequest.IsValid() || !HttpResponse.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadMgr Request or Response is not valid!"));
		return;
	}
	if (bSuccessed && EHttpResponseCodes::IsOk(HttpResponse->GetResponseCode()))
	{
		FString msg;
		TArray<uint8> data = HttpResponse->GetContent();
		FFileHelper::BufferToString(msg, data.GetData(), HttpResponse->GetContentLength());
		OnSocketMessage(msg);
	}
	else
	{
		const FString message = FString::Printf(TEXT("Response Code=%d. ContentLength=%d. \nError Message=%s"), HttpResponse->GetResponseCode(), HttpResponse->GetContent().Num(), *HttpResponse->GetContentAsString());
		UE_LOG(LogTemp, Warning, TEXT("LoadError::%s"), *message);
		if (nullptr != AwakenHandler)
		{
			AwakenHandler->OnError(Type, HttpResponse->GetResponseCode());
		}
	}
	HttpRequest->OnProcessRequestComplete().Unbind();
	HttpRequest->OnRequestProgress().Unbind();
	EndSession();
}

int32 iFlyItsHelper::GetErrorCode() const
{
	return -1;
}

FString iFlyItsHelper::CreatePackage(FString& data, int pStatus)
{
	FString jsonString;
	const TSharedPtr<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&jsonString);
	Writer->WriteObjectStart();

	TSharedPtr<FJsonObject> object = MakeShareable(new FJsonObject);

	const FString AppID = this->my_app_login_params;
	const FString strBase64 = FBase64::Encode(data);

	const TSharedPtr<FJsonObject> common = MakeShareable(new FJsonObject);
	common->SetStringField("app_id", AppID);
	object->SetObjectField("common", common);

	const TSharedPtr<FJsonObject> bussiness = MakeShareable(new FJsonObject);
	bussiness->SetStringField("from", langFrom);
	bussiness->SetStringField("to", langTo);
	object->SetObjectField("business", bussiness);

	const TSharedPtr<FJsonObject> DataJsonObject = MakeShareable(new FJsonObject);
	DataJsonObject->SetStringField("text", strBase64);
	object->SetObjectField("data", DataJsonObject);

	FString DataStr;
	const TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&DataStr);
	FJsonSerializer::Serialize(object.ToSharedRef(), JsonWriter);
	return DataStr;
}

int iFlyItsHelper::Stop()
{
	if (status != WORKING)
	{
		return -1;
	}
	status = SDK_STATS::STOP;
	
	return 0;
}
