#ifndef __MYQCOMMON_H__
#define __MYQCOMMON_H__

#ifndef MYQ_DEBUG
#define MYQ_DEBUG false
#endif

// API constants
#define MYQ_API_AUTH_URL "https://partner-identity.myq-cloud.com/connect"
#define MYQ_API_CLIENT_ID "IOS_CGI_MYQ"
#define MYQ_API_CLIENT_SECRET "VUQ0RFhuS3lQV3EyNUJTdw=="
#define MYQ_API_REDIRECT_URI "com.myqops%3A%2F%2Fios"
#define MYQ_API_AUTH_SCOPE "MyQ_Residential%20offline_access"

#define MYQ_USER_DATA_FILE "/MYQ_USER_DATA.json"

#define MYQ_TIME_GMT_OFFSET -8 * 3600 // - 8 hours PST
#define MYQ_DST_OFFSET 1 * 3600
#define MYQ_NTP_SERVER "pool.ntp.org"

#define MYQ_REFRESH_BUFFER 300000 // 5 minutes

// Logging
#if MYQ_DEBUG
#define MYQ_LOG(message, ...) printf(">>> [%7d][%.2fkb] MyQ: " message , millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#define MYQ_LOG_LINE(message, ...) printf(">>> [%7d][%.2fkb] MyQ: " message "\n", millis(), (esp_get_free_heap_size() * 0.001f), ##__VA_ARGS__)
#else
#define MYQ_LOG(message, ...)
#define MYQ_LOG_LINE(message, ...)
#endif

#define MYQ_AUTH_CA_CERT \
"-----BEGIN CERTIFICATE-----\n\
MIIDxTCCAq2gAwIBAgIBADANBgkqhkiG9w0BAQsFADCBgzELMAkGA1UEBhMCVVMx\n\
EDAOBgNVBAgTB0FyaXpvbmExEzARBgNVBAcTClNjb3R0c2RhbGUxGjAYBgNVBAoT\n\
EUdvRGFkZHkuY29tLCBJbmMuMTEwLwYDVQQDEyhHbyBEYWRkeSBSb290IENlcnRp\n\
ZmljYXRlIEF1dGhvcml0eSAtIEcyMB4XDTA5MDkwMTAwMDAwMFoXDTM3MTIzMTIz\n\
NTk1OVowgYMxCzAJBgNVBAYTAlVTMRAwDgYDVQQIEwdBcml6b25hMRMwEQYDVQQH\n\
EwpTY290dHNkYWxlMRowGAYDVQQKExFHb0RhZGR5LmNvbSwgSW5jLjExMC8GA1UE\n\
AxMoR28gRGFkZHkgUm9vdCBDZXJ0aWZpY2F0ZSBBdXRob3JpdHkgLSBHMjCCASIw\n\
DQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAL9xYgjx+lk09xvJGKP3gElY6SKD\n\
E6bFIEMBO4Tx5oVJnyfq9oQbTqC023CYxzIBsQU+B07u9PpPL1kwIuerGVZr4oAH\n\
/PMWdYA5UXvl+TW2dE6pjYIT5LY/qQOD+qK+ihVqf94Lw7YZFAXK6sOoBJQ7Rnwy\n\
DfMAZiLIjWltNowRGLfTshxgtDj6AozO091GB94KPutdfMh8+7ArU6SSYmlRJQVh\n\
GkSBjCypQ5Yj36w6gZoOKcUcqeldHraenjAKOc7xiID7S13MMuyFYkMlNAJWJwGR\n\
tDtwKj9useiciAF9n9T521NtYJ2/LOdYq7hfRvzOxBsDPAnrSTFcaUaz4EcCAwEA\n\
AaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMCAQYwHQYDVR0OBBYE\n\
FDqahQcQZyi27/a9BUFuIMGU2g/eMA0GCSqGSIb3DQEBCwUAA4IBAQCZ21151fmX\n\
WWcDYfF+OwYxdS2hII5PZYe096acvNjpL9DbWu7PdIxztDhC2gV7+AJ1uP2lsdeu\n\
9tfeE8tTEH6KRtGX+rcuKxGrkLAngPnon1rpN5+r5N9ss4UXnT3ZJE95kTXWXwTr\n\
gIOrmgIttRD02JDHBHNA7XIloKmf7J6raBKZV8aPEjoJpL1E/QYVN8Gb5DKj7Tjo\n\
2GTzLH4U/ALqn83/B2gX2yKQOC16jdFU8WnjXzPKej17CuPKf1855eJ1usV2GDPO\n\
LPAvTK33sefOT6jEm0pUBsV/fdUID+Ic/n4XuKxe9tQWskMJDE32p2u0mYRlynqI\n\
4uJEvlz36hz1\n\
-----END CERTIFICATE-----\n"

#define MYQ_API_CERT \
""

#endif