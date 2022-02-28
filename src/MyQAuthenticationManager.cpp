#include "MyQAuthenticationManager.h"
#include "common.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <base64.h>
#include "./Crypto/SHA256.h"
#include <SPIFFS.h>
#include <time.h>

class AllowAllFilter : ArduinoJson6191_F1::Filter {
    bool allow() const {
        return true;
    }

    bool allowArray() const {
        return true;
    }

    bool allowObject() const {
        return true;
    }

    bool allowValue() const {
        return true;
    }

    template <typename TKey>
    AllowAllFilter operator[](const TKey &) const {
        return AllowAllFilter();
    }
};

//
// Private Member Functions
//

String MyQAuthenticationManager::base64URLEncode(uint8_t *buffer) {
    base64 bs;
    String str = bs.encode(buffer, SHA256_LEN);

    str.replace("+", "-");
    str.replace("/", "_");
    str.replace("=", "");
    
    return str;
}

void MyQAuthenticationManager::sha256(const char *inBuff, uint8_t *outBuff) {
    SHA256 hash;
    hash.reset();
    hash.update(inBuff, strlen(inBuff));
    hash.finalize(outBuff, SHA256_LEN);
}

String MyQAuthenticationManager::getAuthURL() {
    MYQ_LOG_LINE("Code Verifier:     %s", codeVerifier.c_str());
    MYQ_LOG_LINE("Code Challenge:    %s", codeChallenge.c_str());
    String url = String(MYQ_API_AUTH_URL) + "/authorize" +
        "?client_id=IOS_CGI_MYQ" +
        "&code_challenge=" + codeChallenge +
        "&code_challenge_method=S256" + 
        "&redirect_uri=" + MYQ_API_REDIRECT_URI + // need to url encode?
        "&response_type=code" + 
        "&scope=" + MYQ_API_AUTH_SCOPE; // need to url encode?
    return url;
}

bool MyQAuthenticationManager::getAuthToken(String code) {
    code.replace("\r", "");
    code.replace("\n", "");

    StaticJsonDocument<256> headers;
    headers[0]["name"] = "Content-Type";
    headers[0]["value"] = "application/x-www-form-urlencoded";
    headers[1]["name"] = "User-Agent";
    headers[1]["value"] = "null";

    String payload = String("client_id=") + MYQ_API_CLIENT_ID +
        "&client_secret=" + MYQ_API_CLIENT_SECRET +
        "&code=" + code +
        "&code_verifier=" + codeVerifier +
        "&grant_type=authorization_code" + 
        "&redirect_uri=" + MYQ_API_REDIRECT_URI +
        "&scope=" + MYQ_API_AUTH_SCOPE; // does this work here?
    
    DynamicJsonDocument res = request(MYQ_API_AUTH_URL + String("/token"), 3072, false, "POST", payload, headers);

    if (res.size() != 0)
        return storeAuthToken(res);
    else
        return false;
}

bool MyQAuthenticationManager::refreshAuthToken() {
    StaticJsonDocument<256> headers;
    headers[0]["name"] = "Content-Type";
    headers[0]["value"] = "application/x-www-form-urlencoded";
    headers[1]["name"] = "User-Agent";
    headers[1]["value"] = "null";

    String payload = String("client_id=") + MYQ_API_CLIENT_ID +
        "&client_secret=" + MYQ_API_CLIENT_SECRET +
        "&grant_type=refresh_token" + 
        "&redirect_uri=" + MYQ_API_REDIRECT_URI +
        "&refresh_token=" + refreshToken +
        "&scope=" + MYQ_API_AUTH_SCOPE; // does this work here?

    DynamicJsonDocument res = request(MYQ_API_AUTH_URL + String("/token"), 3072, false, "POST", payload, headers);

    if (res.size() != 0)
        return storeAuthToken(res);
    else
        return false;
}

bool MyQAuthenticationManager::storeAuthToken(const DynamicJsonDocument &doc) {
    accessToken = doc["access_token"].as<String>();
    refreshToken = doc["refresh_token"].as<String>();
    tokenType = doc["token_type"].as<String>();
    tokenIssueMS = millis();
    expiresInMS = doc["expires_in"].as<unsigned long>() * 1000;

    return writeUserData();
}

bool MyQAuthenticationManager::writeUserData() {
    // store accessToken, codeVerifier, refreshToken here
    DynamicJsonDocument userData(1536);
    MYQ_LOG_LINE("Created user data object");

    userData["accessToken"] = accessToken;
    userData["refreshToken"] = refreshToken;
    userData["codeVerifier"] = codeVerifier;

    if (!SPIFFS.begin(true)) {
        MYQ_LOG_LINE("Error starting SPIFFS.");
        return false;
    }

    File file = SPIFFS.open(MYQ_USER_DATA_FILE, "w");
    if (!file) {
        MYQ_LOG_LINE("Failed to open %s.", MYQ_USER_DATA_FILE);
        SPIFFS.end();
        return false;
    }

    if (serializeJson(userData, file) == 0) {
        MYQ_LOG_LINE("Failed to write data to %s.", MYQ_USER_DATA_FILE);
        file.close();
        SPIFFS.end();
        return false;
    }

    MYQ_LOG_LINE("Wrote to user data file.");

    file.close();
    SPIFFS.end();
    return true;
}

bool MyQAuthenticationManager::readUserData() {
    MYQ_LOG_LINE("Reading user data file.");
    if (!SPIFFS.begin(true)) {
        MYQ_LOG_LINE("Error starting SPIFFS.");
        return false;
    }

    File file = SPIFFS.open(MYQ_USER_DATA_FILE, "r");
    if (!file) {
        MYQ_LOG_LINE("Failed to open %s.", MYQ_USER_DATA_FILE);
        SPIFFS.end();
        return false;
    }

    DynamicJsonDocument userData(1536);
    DeserializationError err = deserializeJson(userData, file);
    if (err) {
        MYQ_LOG_LINE("Error deserializing %s.", MYQ_USER_DATA_FILE);
        MYQ_LOG_LINE("%s", err.c_str());
        file.close();
        SPIFFS.end();
        return false;
    }

    accessToken = userData["accessToken"].as<String>();
    refreshToken = userData["refreshToken"].as<String>();
    codeVerifier = userData["codeVerifier"].as<String>();

    MYQ_LOG_LINE("Found...");
    #if MYQ_DEBUG
        serializeJsonPretty(userData, Serial);
        Serial.println("");
    #endif

    file.close();
    SPIFFS.end();
    return true;
}

//
// Public Member Functions
//

MyQAuthenticationManager::MyQAuthenticationManager() {
    MYQ_LOG_LINE("Making Authorization Manager.");
    if(!readUserData()) {
        MYQ_LOG_LINE("No previous data, generating codes.");
        uint8_t randData[32]; // 32 bytes, u_int8_t is 1 byte
        esp_fill_random(randData, SHA256_LEN);
        codeVerifier = base64URLEncode(randData);

        uint8_t hashOut[SHA256_LEN];
        sha256(codeVerifier.c_str(), hashOut);
        codeChallenge = base64URLEncode(hashOut);
    }
}

bool MyQAuthenticationManager::authorize(HardwareSerial *hwSerial, unsigned long baud) {
    if (refreshToken.length() == 0) {
        MYQ_LOG_LINE("Get that damn URL code:");
        MYQ_LOG_LINE("%s", getAuthURL().c_str());
        if (!hwSerial) hwSerial->begin(baud);
        while (hwSerial->available() > 0) { hwSerial->read(); } // flush serial monitor
        while (hwSerial->available() == 0) { delay(100); } // wait for url input
        String code = hwSerial->readString();
        hwSerial->println();
        if (getAuthToken(code)) {
            MYQ_LOG_LINE("Successfully authorized Homekit with MyQ.");
            return true;
        } else { 
            MYQ_LOG_LINE("Error authorizing Homekit with MyQ.");
            return false;
        }
    }
        
    return refreshAuthToken();
}

bool MyQAuthenticationManager::isAuthorized() {
    MYQ_LOG_LINE("Issue: %u, expires: %u", tokenIssueMS, expiresInMS);
    if (tokenIssueMS == -1 || expiresInMS == -1) return false;

    unsigned long now = millis();
    unsigned long timeElapsed = max(now, tokenIssueMS) - min(now, tokenIssueMS);
    MYQ_LOG_LINE("time elapsed: %u < %u && refresh token: %s", timeElapsed, expiresInMS - MYQ_REFRESH_BUFFER, refreshToken.length() != 0 ? "true" : "false");
    return timeElapsed < (expiresInMS - MYQ_REFRESH_BUFFER) && refreshToken.length() != 0;
}

DynamicJsonDocument MyQAuthenticationManager::request(
    String url, 
    int docSize, 
    bool auth, 
    const char *method, 
    String payload, 
    const DynamicJsonDocument &headers, 
    const DynamicJsonDocument &filter,
    const DeserializationOption::NestingLimit &nestingLimit
) {
    MYQ_LOG_LINE("Requesting: %s %s", method, url.c_str());
    MYQ_LOG_LINE("Authorized: %s", auth ? "yes" : "no");
    MYQ_LOG_LINE("Payload: %s", payload.c_str());

    if (WiFi.status() != WL_CONNECTED) {
        MYQ_LOG_LINE("Not connected to WiFi.");
        return StaticJsonDocument<0>();
    }

    https = new HTTPClient();
    client = new WiFiClientSecure();
    https->useHTTP10(true); // for ArduinoJson

    if (url.indexOf("https://auth") >= 0) client->setCACert(MYQ_AUTH_CA_CERT);
    else if (url.indexOf("https://api") >= 0) client->setCACert(MYQ_API_CERT);
    else client->setInsecure();

    if (!https->begin(*client, url)){
        MYQ_LOG_LINE("Could not connect to %s.", url.c_str());
        return StaticJsonDocument<0>();
    }

    if (auth) {
        MYQ_LOG_LINE("Setting auth creds.");
        https->setAuthorization(""); // clear it out
        https->addHeader("Authorization", tokenType + " " + accessToken);
    }

    if (headers.size() != 0) {
        MYQ_LOG_LINE("Headers is bigger than 0.");
        for (int x = 0; x < headers.size(); x++) {
            MYQ_LOG_LINE("Adding %i header.", x);
            https->addHeader(headers[x]["name"], headers[x]["value"]);
            MYQ_LOG_LINE(
                "Header added... %s: %s", 
                headers[x]["name"].as<const char*>(), 
                headers[x]["value"].as<const char*>()
            );
        }
    }

    int response;
    if (strcmp(method, "POST") == 0)
        response = https->POST(payload);
    else if (strcmp(method, "PUT") == 0)
        response = https->PUT(payload);
    else if (strcmp(method, "GET") == 0)
        response = https->GET();
    else
        MYQ_LOG_LINE("Unknown request method.");
    MYQ_LOG_LINE("Request sent.");

    if (response < 200 || response > 299) {
        MYQ_LOG_LINE("Error, code: %i.", response);
        MYQ_LOG_LINE("Response: %s", https->getString().c_str());
        return StaticJsonDocument<0>();
    }
    MYQ_LOG_LINE("Response: %i", response);
    
    DynamicJsonDocument doc(docSize);
    MYQ_LOG_LINE("Created doc of %i size", docSize);
    
    DeserializationError err;
    if (filter.size() != 0) err = deserializeJson(doc, https->getStream(), DeserializationOption::Filter(filter), nestingLimit);
    else err = deserializeJson(doc, https->getStream(), nestingLimit);
    MYQ_LOG_LINE("Desearialized stream.");
    
    if (err) {
        if (err == DeserializationError::EmptyInput) doc["response"] = response; // no json response
        else MYQ_LOG_LINE("API request deserialization error: %s", err.f_str());
    } else {
        #if MYQ_DEBUG
            serializeJsonPretty(doc, Serial);
            Serial.println("");
        #endif
    }

    client->stop();
    https->end();
    delete https;
    delete client;

    return doc;
}
