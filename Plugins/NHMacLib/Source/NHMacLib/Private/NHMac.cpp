#include "NHMac.h"

#include "hmac.h"

std::string NHMac::cHMacSha256(const FString& Key, const FString& Data)
{
	std::string hmac = HMac::cHMacSha256(TCHAR_TO_UTF8(*Data), TCHAR_TO_UTF8(*Key));
	return hmac;
}
