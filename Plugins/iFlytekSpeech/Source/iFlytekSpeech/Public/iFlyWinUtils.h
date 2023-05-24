// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <string>

#include "CoreMinimal.h"
#include "iFlyHeader.h"

/**
 * 
 */
class IFLYTEKSPEECH_API iFlyWinUtils
{
public:
	iFlyWinUtils();
	~iFlyWinUtils();
	static void UTF8_to_GB2312(const char* utf8, std::string& gb2312_str);
	static void GB2312_to_UTF8(const char* gb2312, std::string& utf8_str);
	static const char * skip_space(const char *s);
	static wave_pcm_hdr GetDefaultWavePcmHdr();
};
