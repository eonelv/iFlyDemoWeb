#pragma once
#include "iFlyBase.h"
#include "Interfaces/IHttpRequest.h"

class iFlyItsHelper : public iFlyBase
{
public:
	static iFlyItsHelper* Get();

protected:
	iFlyItsHelper();
	virtual ~iFlyItsHelper();
	virtual void ExitImp(int err_code) override;
	virtual FString GetHost() const override;
	virtual FString GetHostPath() const override;
	virtual FString GetProtocol() const override;
public:
	virtual bool Init(const FString& appParam, const FString& session_begin_params) override;
	int32 Start(const FString& src_text);
	
	virtual int32 BeginSession() override;
	virtual int32 EndSession() override;

	void SetLanguage(const FString& from, const FString& to);
protected:
	virtual void OnSocketMessage(const FString& Message) override;

	void OnLoadComplete(FHttpRequestPtr HttpRequest, FHttpResponsePtr HttpResponse, bool bSuccessed);
	int32 GetErrorCode() const;

	FString CreatePackage(FString& data, int pStatus);
public:
	int Stop();
private:
	FString langFrom;
	FString langTo;
	FString text;
	static iFlyItsHelper* ins;
};
