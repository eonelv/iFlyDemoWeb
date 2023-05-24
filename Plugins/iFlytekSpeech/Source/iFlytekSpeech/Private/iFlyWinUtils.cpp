// Fill out your copyright notice in the Description page of Project Settings.


#include "iFlyWinUtils.h"
#include "Windows/MinWindows.h"


iFlyWinUtils::iFlyWinUtils()
{
}

iFlyWinUtils::~iFlyWinUtils()
{
}

void iFlyWinUtils::UTF8_to_GB2312(const char* utf8, std::string& gb2312_str)
{
	int len = MultiByteToWideChar(CP_UTF8,0 , utf8, -1, nullptr, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_UTF8, 0, utf8, -1, wstr, len);
	len = WideCharToMultiByte(CP_ACP, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
	char* str = new char[len+1];
	memset(str,0 , len+1);
	WideCharToMultiByte(CP_ACP,0 , wstr, -1, str, len, nullptr, nullptr);
	delete[] wstr;
	gb2312_str = str;
	delete[] str;
}

void iFlyWinUtils::GB2312_to_UTF8(const char* gb2312, std::string& utf8_str)
{
	int len = MultiByteToWideChar(CP_ACP, 0, gb2312, -1, nullptr, 0);
	wchar_t* wstr = new wchar_t[len+1];
	memset(wstr, 0, len+1);
	MultiByteToWideChar(CP_ACP, 0, gb2312, -1, wstr, len);
	len = WideCharToMultiByte(CP_UTF8, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
	char* str = new char[len+1];
	memset(str, 0, len+1);
	WideCharToMultiByte(CP_UTF8, 0, wstr, -1, str, len, nullptr, nullptr);
	delete[] wstr;
	utf8_str = str;
	delete[] str;
}

const char* iFlyWinUtils::skip_space(const char* s)
{
	while (s && *s != ' ' && *s != '\0')
		s++;
	return s;
}

wave_pcm_hdr iFlyWinUtils::GetDefaultWavePcmHdr()
{
	static wave_pcm_hdr default_wav_hdr = 
	{
		{ 'R', 'I', 'F', 'F' },
		0,
		{'W', 'A', 'V', 'E'},
		{'f', 'm', 't', ' '},
		16,
		1,
		1,
		16000,
		32000,
		2,
		16,
		{'d', 'a', 't', 'a'},
		0  
	};
	return default_wav_hdr;
}
