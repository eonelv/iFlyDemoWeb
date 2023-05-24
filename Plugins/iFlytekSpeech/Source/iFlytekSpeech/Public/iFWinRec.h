// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Windows/MinWindows.h"
#include <mmeapi.h>

enum {
	RECORD_ERR_BASE = 0,
	RECORD_ERR_GENERAL,
	RECORD_ERR_MEMFAIL,
	RECORD_ERR_INVAL,
	RECORD_ERR_NOT_READY
};

/* recorder object. */
struct recorder {
	void (*on_data_ind)(char *data, unsigned long len, void *user_para);
	void * user_cb_para;
	volatile int state;		/* internal record state */

	void * wavein_hdl;
	void * rec_thread_hdl;
	void * bufheader;
	unsigned int bufcount;
};

/* Do not change the sequence */
enum {
	RECORD_STATE_CREATED,	/* Init		*/
	RECORD_STATE_READY,		/* Opened	*/
	RECORD_STATE_STOPPING,	/* During Stop	*/
	RECORD_STATE_RECORDING,	/* Started	*/
};

#define SAMPLE_RATE  16000
#define SAMPLE_BIT_SIZE 16
#define FRAME_CNT   10
#define BUF_COUNT   4


static void free_rec_buffer(HWAVEIN wi, WAVEHDR *first_header, unsigned headercount);
static void data_proc(struct recorder *rec, MSG *msg);
static unsigned int  __stdcall record_thread_proc ( void * para);

static HANDLE msgqueue_ready_evt = NULL; /* signaled: the message queque has been created in the thread */

/**
 * 
 */
class IFLYTEKSPEECH_API iFWinRec
{
public:
	iFWinRec();
	~iFWinRec();
	void destroy_recorder();
	int open_recorder(unsigned int dev, WAVEFORMATEX * fmt);
	void close_recorder();
	int start_record();
	int stop_record();
	int is_record_stopped();
	int get_default_input_dev();
	unsigned int get_input_dev_num();
	int create_recorder(struct recorder ** out_rec, 
				void (*on_data_ind)(char *data, unsigned long len, void *user_cb_para), 
				void* user_cb_para);
	
public:
	recorder *rec;
};
