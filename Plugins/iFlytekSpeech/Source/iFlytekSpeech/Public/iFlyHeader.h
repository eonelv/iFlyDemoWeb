// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#define DEFAULT_INPUT_DEVID     (-1)
#define E_SR_NOACTIVEDEVICE		1
#define E_SR_NOMEM				2
#define E_SR_INVAL				3
#define E_SR_RECORDFAIL			4
#define E_SR_ALREADY			5
#define END_REASON_VAD_DETECT	0	/* detected speech done  */

#define DEFAULT_FORMAT		\
{\
WAVE_FORMAT_PCM,	\
1,					\
16000,				\
32000,				\
2,					\
16,					\
sizeof(WAVEFORMATEX)	\
}


struct speech_rec_notifier {
	void (*on_result)(const char *result, char is_last);
	void (*on_speech_begin)();
	void (*on_speech_end)(int reason);	/* 0 if VAD.  others, error : see E_SR_xxx and msp_errors.h  */
};

struct speech_rec {
	struct speech_rec_notifier notif;
	const char * session_id;
	int ep_stat;
	int rec_stat;
	int audio_status;
	struct recorder *recorder;
	volatile int state;
	char * session_begin_params;
};

/* wav音频头部格式 */
typedef struct _wave_pcm_hdr
{
	char            riff[4];                // = "RIFF"
	int				size_8;                 // = FileSize - 8
	char            wave[4];                // = "WAVE"
	char            fmt[4];                 // = "fmt "
	int				fmt_size;				// = 下一个结构体的大小 : 16

	short int       format_tag;             // = PCM : 1
	short int       channels;               // = 通道数 : 1
	int				samples_per_sec;        // = 采样率 : 8000 | 6000 | 11025 | 16000
	int				avg_bytes_per_sec;      // = 每秒字节数 : samples_per_sec * bits_per_sample / 8
	short int       block_align;            // = 每采样点字节数 : wBitsPerSample / 8
	short int       bits_per_sample;        // = 量化比特数: 8 | 16

	char            data[4];                // = "data";
	int				data_size;              // = 纯数据长度 : FileSize - 44 
} wave_pcm_hdr;

enum
{
	MSP_EP_LOOKING_FOR_SPEECH   = 0,
	MSP_EP_IN_SPEECH            = 1,
	MSP_EP_AFTER_SPEECH         = 3,
	MSP_EP_TIMEOUT              = 4,
	MSP_EP_ERROR                = 5,
	MSP_EP_MAX_SPEECH           = 6,
	MSP_EP_IDLE                 = 7  // internal state after stop and before start
};

enum SDK_STATS : uint8
{
	NONE,
	INITED,
	STARTED,//已经开始但还没有开始Session。WebSocket有该状态
	WORKING,
	STOP,//已经停止但Session还未结束，WebSocket有该状态
	CLOSED
};

/* internal state */
enum {
	SR_STATE_INIT,
	SR_STATE_STARTED
};

/*
 *  The enumeration MSPRecognizerStatus contains the recognition status
 *  MSP_REC_STATUS_SUCCESS				- successful recognition with partial results
 *  MSP_REC_STATUS_NO_MATCH				- recognition rejected
 *  MSP_REC_STATUS_INCOMPLETE			- recognizer needs more time to compute results
 *  MSP_REC_STATUS_NON_SPEECH_DETECTED	- discard status, no more in use
 *  MSP_REC_STATUS_SPEECH_DETECTED		- recognizer has detected audio, this is delayed status
 *  MSP_REC_STATUS_COMPLETE				- recognizer has return all result
 *  MSP_REC_STATUS_MAX_CPU_TIME			- CPU time limit exceeded
 *  MSP_REC_STATUS_MAX_SPEECH			- maximum speech length exceeded, partial results may be returned
 *  MSP_REC_STATUS_STOPPED				- recognition was stopped
 *  MSP_REC_STATUS_REJECTED				- recognizer rejected due to low confidence
 *  MSP_REC_STATUS_NO_SPEECH_FOUND		- recognizer still found no audio, this is delayed status
 */
enum
{
	MSP_REC_STATUS_SUCCESS              = 0,
	MSP_REC_STATUS_NO_MATCH             = 1,
	MSP_REC_STATUS_INCOMPLETE			= 2,
	MSP_REC_STATUS_NON_SPEECH_DETECTED  = 3,
	MSP_REC_STATUS_SPEECH_DETECTED      = 4,
	MSP_REC_STATUS_COMPLETE				= 5,
	MSP_REC_STATUS_MAX_CPU_TIME         = 6,
	MSP_REC_STATUS_MAX_SPEECH           = 7,
	MSP_REC_STATUS_STOPPED              = 8,
	MSP_REC_STATUS_REJECTED             = 9,
	MSP_REC_STATUS_NO_SPEECH_FOUND      = 10,
	MSP_REC_STATUS_FAILURE = MSP_REC_STATUS_NO_MATCH,
};

/**
 *  MSPSampleStatus indicates how the sample buffer should be handled
 *  MSP_AUDIO_SAMPLE_FIRST		- The sample buffer is the start of audio
 *								  If recognizer was already recognizing, it will discard
 *								  audio received to date and re-start the recognition
 *  MSP_AUDIO_SAMPLE_CONTINUE	- The sample buffer is continuing audio
 *  MSP_AUDIO_SAMPLE_LAST		- The sample buffer is the end of audio
 *								  The recognizer will cease processing audio and
 *								  return results
 *  Note that sample statii can be combined; for example, for file-based input
 *  the entire file can be written with SAMPLE_FIRST | SAMPLE_LAST as the
 *  status.
 *  Other flags may be added in future to indicate other special audio
 *  conditions such as the presence of AGC
 */
enum
{
	MSP_AUDIO_SAMPLE_INIT           = 0x00,
	MSP_AUDIO_SAMPLE_FIRST          = 0x01,
	MSP_AUDIO_SAMPLE_CONTINUE       = 0x02,
	MSP_AUDIO_SAMPLE_LAST           = 0x04,
};

enum XF_SDK_FUN_TYPE : uint8
{
	AWAKEN,
	TTS_ONLINE,
	TTS_OFFLINE,
	TTS_WEB,//WebSocket版本
	IAT_ONLINE,
	RATSR_ONLINE,
	ISE_ONLINE,
	ITS_WEB//机器翻译
};

class XFProcessHandler
{
public:
	virtual ~XFProcessHandler(){};
	virtual void OnAwake(bool bSuccess, const FString& msg) = 0;
	virtual void AwakenNotify(const FString& msg) = 0;
	virtual void OnExit(int32 type, int32 code) = 0;
	virtual void OnSpeechBegin() = 0;
	virtual void OnSpeechEnd(int32 ErrorCode) = 0;
	virtual void OnSpeechMessage(const FString msg, const FString msg2=TEXT("")) = 0;
	virtual void OnSessionEnd(int32 type, int32 ErrorCode) = 0;
	virtual void OnError(int32 type, int32 code) = 0;
	virtual void OnData(const void* datas, int32 len) = 0;
};

class XFHandlerAdapter : public XFProcessHandler
{
public:
	virtual ~XFHandlerAdapter() override{};
	virtual void OnAwake(bool bSuccess, const FString& msg) override{};
	virtual void AwakenNotify(const FString& msg) override {};
	virtual void OnExit(int32 type, int32 code) override {};
	virtual void OnSpeechBegin() override {};
	virtual void OnSpeechEnd(int32 ErrorCode) override {};
	virtual void OnSpeechMessage(const FString msg, const FString msg2=TEXT("")) override {};
	virtual void OnSessionEnd(int32 type, int32 ErrorCode) override{};
	virtual void OnError(int32 type, int32 code) override {};
	virtual void OnData(const void* datas, int32 len) override {};
};
