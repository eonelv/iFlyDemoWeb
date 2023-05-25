#include "NHMac.h"

#include "hmac.h"

std::string NHMac::cHMacSha256(const FString& Key, const FString& Data)
{
	std::string hmac = HMac::cHMacSha256(TCHAR_TO_UTF8(*Data), TCHAR_TO_UTF8(*Key));
	return hmac;
}

std::string NHMac::cSha256(const FString& Data)
{
	unsigned char digest[SHA256_DIGEST_SIZE];
	std::string cData = TCHAR_TO_UTF8(*Data);
	sha256((const unsigned char *)cData.c_str(), cData.length(), digest);
	std::string val =  std::string{reinterpret_cast<char const*>(digest), 32};

	return val;
}
