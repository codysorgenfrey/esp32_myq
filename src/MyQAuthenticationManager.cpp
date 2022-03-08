#include "MyQAuthenticationManager.h"
#include "common.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <base64.h>
#include <SHA256.h>
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
    bool success = true;

    DynamicJsonDocument userData(1536);
    MYQ_LOG_LINE("Created user data object");

    userData["accessToken"] = accessToken;
    userData["refreshToken"] = refreshToken;
    userData["codeVerifier"] = codeVerifier;

    if (SPIFFS.begin(true)) {
        File file = SPIFFS.open(MYQ_USER_DATA_FILE, "w");
        if (file) {
            if (serializeJson(userData, file) > 0) {
                MYQ_LOG_LINE("Wrote to user data file.");
            } else {
                MYQ_ERROR_LINE("Failed to write data to %s.", MYQ_USER_DATA_FILE);
                success = false;
            }

            file.close();
        } else {
            MYQ_ERROR_LINE("Failed to open %s.", MYQ_USER_DATA_FILE);
            success = false;
        }

        SPIFFS.end();
    } else {
        MYQ_ERROR_LINE("Error starting SPIFFS.");
        success = false;
    }

    return success;
}

bool MyQAuthenticationManager::readUserData() {
    MYQ_LOG_LINE("Reading user data file.");

    bool success = true;

    if (SPIFFS.begin(true)) {
        File file = SPIFFS.open(MYQ_USER_DATA_FILE, "r");
        if (file) {
            DynamicJsonDocument userData(1536);
            DeserializationError err = deserializeJson(userData, file);
            if (err) {
                MYQ_ERROR_LINE("Error deserializing %s.", MYQ_USER_DATA_FILE);
                MYQ_ERROR_LINE("%s", err.c_str());
                success = false;
            } else {
                accessToken = userData["accessToken"].as<String>();
                refreshToken = userData["refreshToken"].as<String>();
                codeVerifier = userData["codeVerifier"].as<String>();

                if (
                    accessToken.equals("null") ||
                    refreshToken.equals("null") ||
                    codeVerifier.equals("null")
                ) {
                    MYQ_ERROR_LINE("Found file but contents are empty.");
                    accessToken = "";
                    refreshToken = "";
                    codeVerifier = "";
                    success = false;
                }

                MYQ_LOG_LINE("Found...");
                #if MYQ_DEBUG >= MYQ_DEBUG_INFO
                    serializeJsonPretty(userData, Serial);
                    Serial.println("");
                #endif
            }

            file.close();
        } else {
            MYQ_ERROR_LINE("Failed to open %s.", MYQ_USER_DATA_FILE);
            success = false;
        }

        SPIFFS.end();
    } else {
        MYQ_ERROR_LINE("Error starting SPIFFS.");
        success = false;
    }

    return success;
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
            MYQ_ERROR_LINE("Error authorizing Homekit with MyQ.");
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

    DynamicJsonDocument doc(docSize);
    MYQ_LOG_LINE("Created doc of %i size", docSize);

    if (WiFi.status() == WL_CONNECTED) {
        HTTPClient https;
        WiFiClientSecure client;
        https.useHTTP10(true); // for ArduinoJson
        client.setCACert(MYQ_CA_CERT);

        if (https.begin(client, url)){
            if (auth) {
                MYQ_LOG_LINE("Setting auth creds.");
                https.setAuthorization(""); // clear it out
                https.addHeader("Authorization", tokenType + " " + accessToken);
            }

            if (headers.size() != 0) {
                MYQ_LOG_LINE("Headers is bigger than 0.");
                for (int x = 0; x < headers.size(); x++) {
                    MYQ_LOG_LINE("Adding %i header.", x);
                    https.addHeader(headers[x]["name"], headers[x]["value"]);
                    MYQ_LOG_LINE(
                        "Header added... %s: %s", 
                        headers[x]["name"].as<const char*>(), 
                        headers[x]["value"].as<const char*>()
                    );
                }
            }

            int response;
            if (strcmp(method, "POST") == 0)
                response = https.POST(payload);
            else if (strcmp(method, "PUT") == 0)
                response = https.PUT(payload);
            else if (strcmp(method, "GET") == 0)
                response = https.GET();
            else
                MYQ_ERROR_LINE("Unknown request method.");
            MYQ_LOG_LINE("Request sent.");

            if (response >= 200 || response <= 299) {
                MYQ_LOG_LINE("Response: %i", response);
                
                DeserializationError err;
                if (filter.size() != 0) err = deserializeJson(doc, https.getStream(), DeserializationOption::Filter(filter), nestingLimit);
                else err = deserializeJson(doc, https.getStream(), nestingLimit);
                
                if (err) {
                    if (err == DeserializationError::EmptyInput) doc["response"] = response; // no json response
                    else MYQ_ERROR_LINE("API request deserialization error: %s", err.c_str());
                } else {
                    MYQ_LOG_LINE("Desearialized stream.");
                    #if MYQ_DEBUG
                        serializeJsonPretty(doc, Serial);
                        Serial.println("");
                    #endif
                }
            } else {
                MYQ_ERROR_LINE("%s Error, code: %i.", method, response);
                MYQ_ERROR_LINE("Response: %s", https.getString().c_str());
            }

            client.stop();
            https.end();
        } else {
            MYQ_ERROR_LINE("Could not connect to %s.", url.c_str());
        }
    } else {
        MYQ_ERROR_LINE("Not connected to WiFi.");
    }

    return doc;
}
