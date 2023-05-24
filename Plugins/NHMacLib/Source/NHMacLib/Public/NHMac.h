#pragma once

class NHMACLIB_API NHMac
{
public:
	static std::string cHMacSha256(const FString& Key, const FString & Data);
};
