#pragma once
#include "iFlyBase.h"
#include "iFWinRec.h"

class iFlyTtsHelper : public iFlyBase
{
public:
	static iFlyTtsHelper* Get();

protected:
	iFlyTtsHelper();
	virtual ~iFlyTtsHelper();
	virtual void ExitImp(int err_code) override;
	virtual FString GetHost() const override;
	virtual FString GetHostPath() const override;
public:
	virtual bool Init(const FString& appParam, const FString& session_begin_params) override;
	int32 Start(const FString& src_text);
	
	virtual int32 BeginSession() override;
	virtual int32 EndSession() override;
	virtual bool OnTick() override;
	void SetVoice(const FString& pVoice);
protected:
	virtual void OnConnected() override;
	virtual void OnSocketClose(int32 StatusCode, const FString& Reason, bool bWasClean) override;
	virtual void OnSocketMessage(const FString& Message) override;
	virtual void OnMessageSend(const FString& Message) override;

	int32 GetErrorCode() const;

	void SendData(bool bClose);
	FString CreatePackage(FString& data, int pStatus);
public:
	int Stop();
private:
	FString Voice;
	FString text;
	static iFlyTtsHelper* ins;
};
