#pragma once
#include <string>

class NHMACLIB_API NHMac
{
public:
	static std::string cHMacSha256(const FString& Key, const FString & Data);
	static std::string cSha256(const FString & Data);
};
