#ifndef __MYQAUTHENTICATIONMANAGER_H__
#define __MYQAUTHENTICATIONMANAGER_H__

#include "common.h"
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>

class MyQAuthenticationManager {
    private:
        HTTPClient *https;
        WiFiClientSecure *client;
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
        bool authorize(HardwareSerial *hwSerial, unsigned long baud);
        bool isAuthorized();
        DynamicJsonDocument request(
            String url, 
            int docSize = 3072, 
            bool auth = true, 
            bool post = false, 
            String payload = "", 
            const DynamicJsonDocument &headers = StaticJsonDocument<0>(), 
            const DynamicJsonDocument &filter = StaticJsonDocument<0>(),
            const DeserializationOption::NestingLimit &nestingLimit = DeserializationOption::NestingLimit()
        );
};

#endif 