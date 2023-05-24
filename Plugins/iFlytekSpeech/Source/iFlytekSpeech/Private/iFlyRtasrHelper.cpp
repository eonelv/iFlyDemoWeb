// Fill out your copyright notice in the Description page of Project Settings.


#include "iFlyRtasrHelper.h"

#include <string>
#include "iFlyWinUtils.h"
#include "WebSockets/Public/IWebSocket.h"

void on_result_ratsr(const char* result, char is_last)
{
	std::string msg;
	iFlyWinUtils::GB2312_to_UTF8(result, msg);
}

void on_speech_begin_ratsr()
{
}

void on_speech_end_ratsr(int reason)
{
}

void iat_cb_ratsr(char* data, unsigned long len, void* user_para)
{
	int errcode;
	struct speech_rec* sr;

	if (len == 0 || data == NULL)
		return;

	sr = (struct speech_rec*)user_para;

	if (sr == NULL || sr->ep_stat >= MSP_EP_AFTER_SPEECH)
		return;

#ifdef __FILE_SAVE_VERIFY__
	loopwrite_to_file(data, len);
#endif

	errcode = iFlyRtasrHelper::Get()->sr_write_audio_data(data, len);
}

static int update_format_from_sessionparam_ratsr(const char* session_para, WAVEFORMATEX* wavefmt)
{
	const char* s = strstr(session_para, "sample_rate");
	if (s != nullptr)
	{
		s = strstr(s, "=");
		if (s != nullptr)
		{
			s = iFlyWinUtils::skip_space(s);
			if (s && *s)
			{
				wavefmt->nSamplesPerSec = atoi(s);
				wavefmt->nAvgBytesPerSec = wavefmt->nBlockAlign * wavefmt->nSamplesPerSec;
			}
		}
		else
			return -1;
	}
	else
	{
		return -1;
	}

	return 0;
}

iFlyRtasrHelper* iFlyRtasrHelper::ins = nullptr;

iFlyRtasrHelper* iFlyRtasrHelper::Get()
{
	if (nullptr == ins)
	{
		ins = new iFlyRtasrHelper();
	}
	return ins;
}

iFlyRtasrHelper::iFlyRtasrHelper() : sr(nullptr), WinRec(nullptr), Seq(0), totalLength(0)
{
	Type = XF_SDK_FUN_TYPE::RATSR_ONLINE;
	recordDatas.AddUninitialized(1024 * 1024 * 10);
}


iFlyRtasrHelper::~iFlyRtasrHelper()
{
}

void iFlyRtasrHelper::ExitImp(int err_code)
{
	end_sr_on_vad();
	EndSession();

	if (nullptr != sr->recorder)
	{
		if (!WinRec->is_record_stopped())
			WinRec->stop_record();
		WinRec->close_recorder();
		WinRec->destroy_recorder();
		sr->recorder = nullptr;
	}

	if (nullptr != WinRec)
	{
		delete WinRec;
		WinRec = nullptr;
	}
}

FString iFlyRtasrHelper::GetHost() const
{
	return TEXT("ist-api-sg.xf-yun.com");
}

FString iFlyRtasrHelper::GetHostPath() const
{
	return TEXT("/v2/ist");
}

bool iFlyRtasrHelper::Init(const FString& appParam, const FString& session_begin_params)
{
	iFlyBase::Init(appParam, session_begin_params);
	sr = new speech_rec();
	int errcode;
	size_t param_size;
	WAVEFORMATEX wavfmt = DEFAULT_FORMAT;

	memset(sr, 0, sizeof(struct speech_rec));
	sr->state = SR_STATE_INIT;
	sr->ep_stat = MSP_EP_LOOKING_FOR_SPEECH;
	sr->rec_stat = MSP_REC_STATUS_SUCCESS;
	sr->audio_status = MSP_AUDIO_SAMPLE_FIRST;

	param_size = strlen(TCHAR_TO_UTF8(*session_begin_params)) + 1;
	sr->session_begin_params = (char*)malloc(param_size);
	if (sr->session_begin_params == NULL)
	{
		//sr_dbg("mem alloc failed\n");
		return false;
	}
	
	// strncpy(sr->session_begin_params, TCHAR_TO_UTF8(*session_begin_params), param_size - 1);
	strncpy_s(sr->session_begin_params, param_size, TCHAR_TO_UTF8(*session_begin_params), param_size - 1);

	struct speech_rec_notifier recnotifier = {
		on_result_ratsr,
		on_speech_begin_ratsr,
		on_speech_end_ratsr
	};
	sr->notif = recnotifier;

	WinRec = new iFWinRec();
	if (WinRec->get_input_dev_num() == 0)
	{
		return false;
	}
	errcode = WinRec->create_recorder(&sr->recorder, iat_cb_ratsr, (void*)sr);
	if (sr->recorder == NULL || errcode != 0)
	{
		//sr_dbg("create recorder failed: %d\n", errcode);
		errcode = -E_SR_RECORDFAIL;
		this->Exit(errcode);
	}
	update_format_from_sessionparam_ratsr(TCHAR_TO_UTF8(*session_begin_params), &wavfmt);

	WinRec->rec = sr->recorder;
	errcode = WinRec->open_recorder(DEFAULT_INPUT_DEVID, &wavfmt);
	if (errcode != 0)
	{
		//sr_dbg("recorder open failed: %d\n", errcode);
		errcode = -E_SR_RECORDFAIL;
		this->Exit(errcode);
	}

	return false;
}

int32 iFlyRtasrHelper::Start(const FString& src_text)
{
	if (status != INITED)
	{
		return -1;
	}
	status = STARTED;
	Seq = 0;
	
	const FString rHost = GetReqUrl();
	UE_LOG(LogTemp, Warning, TEXT("rHost: %s"), *rHost);

	CreateWebSocket(rHost, GetProtocol());
	return 0;
}

int32 iFlyRtasrHelper::BeginSession()
{
	int ret = -1;
	if (-1 == iFlyBase::BeginSession())
	{
		return ret;
	}
	// sr->session_id = session_id;
	sr->ep_stat = MSP_EP_LOOKING_FOR_SPEECH;
	sr->rec_stat = MSP_REC_STATUS_SUCCESS;
	sr->audio_status = MSP_AUDIO_SAMPLE_FIRST;

	ret = WinRec->start_record();
	UE_LOG(LogTemp, Warning, TEXT("start_record:%d"), ret);
	if (ret != 0)
	{
		//sr_dbg("start record failed: %d\n", ret);
		EndSession();
		return ret;
	}

#ifdef __FILE_SAVE_VERIFY__
	open_stored_file(VERIFY_FILE_NAME);
#endif

	sr->state = SR_STATE_STARTED;

	if (sr->notif.on_speech_begin)
	{
		sr->notif.on_speech_begin();
	}
	return 0;
}

int32 iFlyRtasrHelper::EndSession()
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

bool iFlyRtasrHelper::OnTick()
{
	if (status != WORKING)
	{
		return false;
	}
	SendData(false, false);
	return iFlyBase::OnTick();
}

void iFlyRtasrHelper::OnBegin(const FString& Message)
{
}

void iFlyRtasrHelper::OnConnected()
{
	BeginSession();
}

void iFlyRtasrHelper::OnSocketClose(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	end_sr_on_vad();
	EndSession();
}

void iFlyRtasrHelper::OnSocketMessage(const FString& Message)
{
	//UE_LOG(LogTemp,Warning,TEXT("5%s"),*Message);

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
	int32 jsonStatus = DataJson->GetNumberField(TEXT("status"));
	TSharedPtr<FJsonObject> resultJson = DataJson->GetObjectField(TEXT("result"));

	if (!resultJson->HasField(TEXT("ws")))
	{
		return;
	}
	TArray<TSharedPtr<FJsonValue>> wsArray = resultJson->GetArrayField("ws");

	FString msg;
	for (const auto& Item : wsArray)
	{
		TArray<TSharedPtr<FJsonValue>> cwArray = Item->AsObject()->GetArrayField("cw");
		for (const auto& ItemCw : cwArray)
		{
			FString word = ItemCw->AsObject()->GetStringField("w");
			msg = msg + word;
		}
	}
	if (nullptr != AwakenHandler)
	{
		AwakenHandler->OnSpeechMessage(msg, "");
	}
	if (jsonStatus == 2)
	{
		end_sr_on_vad();
		EndSession();
	}
}

void iFlyRtasrHelper::OnMessageSend(const FString& Message)
{
}

int32 iFlyRtasrHelper::GetErrorCode() const
{
	return -1;
}

bool iFlyRtasrHelper::SendData(bool bClose, bool bForce)
{
	bool bRecent = true;
	int len = 1280;
	TArray<uint8> data;
	{
		mutex.Lock();
		if ((bForce && totalLength > 0) && (bClose && totalLength > 0) || totalLength > len)
		{
			if (totalLength < len)
			{
				len = totalLength;
				bRecent = false;
			}
			data.AddUninitialized(len);
			FMemory::Memcpy(data.GetData(), recordDatas.GetData(), len);
			totalLength -= len;
			if (totalLength != 0)
			{
				FMemory::Memcpy(recordDatas.GetData(), recordDatas.GetData() + len, totalLength);
			}
		}
		else
		{
			bRecent = false;
			mutex.Unlock();
			return !bRecent;
		}
		mutex.Unlock();
	}

	int tStatus = Seq == 0 ? 0 : 1;
	if (bClose)
	{
		tStatus = 2;
	}

	FString DataStr = CreatePackage(data, tStatus);
	Seq++;
	WebSocket->Send(DataStr);
	return !bRecent;
}

FString iFlyRtasrHelper::CreatePackage(TArray<uint8> data, int pStatus)
{
	FString jsonString;
	TSharedPtr<TJsonWriter<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>> Writer =
		TJsonWriterFactory<TCHAR, TCondensedJsonPrintPolicy<TCHAR>>::Create(&jsonString);
	Writer->WriteObjectStart();

	TSharedPtr<FJsonObject> object = MakeShareable(new FJsonObject);

	FString AppID = this->my_app_login_params;
	FString From = TEXT("en");
	FString To = TEXT("ar");
	FString Voice = TEXT("x2_catherine");
	FString strBase64 = FBase64::Encode(data);

	if (pStatus == 0)
	{
		const TSharedPtr<FJsonObject> common = MakeShareable(new FJsonObject);
		common->SetStringField("app_id", AppID);
		object->SetObjectField("common", common);

		const TSharedPtr<FJsonObject> bussiness = MakeShareable(new FJsonObject);
		bussiness->SetStringField("domain", "ist_open");
		bussiness->SetStringField("language", "en_us");
		bussiness->SetStringField("accent", "mandarin");
		bussiness->SetStringField("domain", "ist_open");
		object->SetObjectField("business", bussiness);
	}

	const TSharedPtr<FJsonObject> DataJsonObject = MakeShareable(new FJsonObject);
	DataJsonObject->SetNumberField("status", pStatus);
	DataJsonObject->SetStringField("format", "audio/L16;rate=16000");
	DataJsonObject->SetStringField("encoding", "raw");
	DataJsonObject->SetStringField("audio", strBase64);

	object->SetObjectField("data", DataJsonObject);

	FString DataStr;
	const TSharedRef<TJsonWriter<TCHAR>> JsonWriter = TJsonWriterFactory<TCHAR>::Create(&DataStr);
	FJsonSerializer::Serialize(object.ToSharedRef(), JsonWriter);
	return DataStr;
}

int iFlyRtasrHelper::Stop()
{
	if (status != WORKING)
	{
		return -1;
	}
	status = SDK_STATS::STOP;
	int ret = 0;
	const char* rslt = nullptr;

	if (sr->state < SR_STATE_STARTED)
	{
		return 0;
	}

	ret = WinRec->stop_record();
#ifdef __FILE_SAVE_VERIFY__
	safe_close_file();
#endif
	if (ret != 0)
	{
		// sr_dbg("Stop failed! \n");
		return -E_SR_RECORDFAIL;
	}
	sr->state = SR_STATE_INIT;
	while (SendData(false, true))
	{
		
	}
	SendData(true, false);
	return 0;
}

int iFlyRtasrHelper::sr_write_audio_data(char* data, unsigned len)
{
	const char* rslt = NULL;
	int ret = 0;
	if (!sr)
	{
		end_sr_on_error(ret);
		return -E_SR_INVAL;
	}

	if (!data || !len)
	{
		return 0;
	}

	{
		mutex.Lock();
		FMemory::Memcpy(recordDatas.GetData() + totalLength, data, len);
		totalLength += len;
		mutex.Unlock();
	}

	sr->audio_status = MSP_AUDIO_SAMPLE_CONTINUE;

	if (MSP_EP_AFTER_SPEECH == sr->ep_stat)
		end_sr_on_vad();

	return 0;
}

void iFlyRtasrHelper::end_sr_on_error(int errcode)
{
	if (WinRec != nullptr)
	{
		WinRec->stop_record();
	}

	if (sr->session_id)
	{
		if (sr->notif.on_speech_end)
			sr->notif.on_speech_end(errcode);
	}
	EndSession();
	sr->state = SR_STATE_INIT;
#ifdef __FILE_SAVE_VERIFY__
	safe_close_file();
#endif
}

void iFlyRtasrHelper::wait_for_rec_stop(unsigned timeout_ms)
{
	while (!WinRec->is_record_stopped())
	{
		Sleep(1);
		if (timeout_ms != (unsigned int)-1)
			if (0 == timeout_ms--)
				break;
	}
}

void iFlyRtasrHelper::end_sr_on_vad()
{
	if (nullptr != WinRec)
	{
		WinRec->stop_record();
	}
	wait_for_rec_stop(3);
	if (sr->session_id)
	{
		if (sr->notif.on_speech_end)
			sr->notif.on_speech_end(END_REASON_VAD_DETECT);
		//EndSession();
	}
	sr->state = SR_STATE_INIT;
#ifdef __FILE_SAVE_VERIFY__
	safe_close_file();
#endif
}
