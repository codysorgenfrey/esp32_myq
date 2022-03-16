#pragma once
#ifndef __MYQAUTHENTICATIONMANAGER_H__
#define __MYQAUTHENTICATIONMANAGER_H__

#include "common.h"
#include <ArduinoJson.h>

#define SHA256_LEN 32

class MyQAuthenticationManager {
    private:
        String refreshToken;
        String codeVerifier;
        String codeChallenge;
        unsigned long tokenIssueMS = -1;
        unsigned long expiresInMS = -1;

        String base64URLEncode(uint8_t *buffer);
        void sha256(const char *inBuff, uint8_t *outBuff);
        String getAuthURL();
        bool getAuthToken(String code);
        bool refreshAuthToken();
        bool storeAuthToken(const DynamicJsonDocument &doc);
        bool writeUserData();
        bool readUserData();

    public:
        String tokenType = "Bearer";
        String accessToken;

        MyQAuthenticationManager();
        bool authorize(bool forceReauth, HardwareSerial *hwSerial, unsigned long baud);
        bool isAuthorized();
        int request(
            String url, 
            JsonDocument &doc, 
            bool auth = true, 
            const char *method = "GET", 
            String payload = "", 
            const DynamicJsonDocument &headers = StaticJsonDocument<0>(), 
            const DynamicJsonDocument &filter = StaticJsonDocument<0>(),
            const DeserializationOption::NestingLimit &nestingLimit = DeserializationOption::NestingLimit()
        );
};

#endif 