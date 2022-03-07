#include "MyQ.h"
#include "MyQAuthenticationManager.h"

const char* MYQ_DOOR_GETSTATE_VALUES[6] = {
    "closed",
    "closing",
    "open",
    "opening",
    "stopped",
    "autoreverse"
};

const char* MYQ_DOOR_SETSTATE_VALUES[2] = {
    "close",
    "open"
};

//
// Private member functions
//

String MyQ::getAccountId() {
    if (accountId.length() != 0) {
        MYQ_LOG_LINE("Account ID already exists.");
        return accountId;
    }

    DynamicJsonDocument res = authManager->request("https://accounts.myq-cloud.com/api/v6.0/accounts", 384);

    if (res.size() > 0) {
        accountId = res["accounts"][0]["id"].as<String>();
        return accountId; 
    }

    MYQ_ERROR_LINE("Error getting account ID.");
    return "";
}

//
// Public member functions
//

MyQ::MyQ() {
    authManager = new MyQAuthenticationManager();
}

bool MyQ::setup(HardwareSerial *inSerial, int inBaud) {
    _serial = inSerial;
    _baud = inBaud;

    if(!authManager->authorize(_serial, _baud)) {
        MYQ_ERROR_LINE("Failed to authorize with MyQ.");
        return false;
    }

    return true;
}

void MyQ::loop() {
    if (millis() % 60000 == 0) { // check once every minute
        if (!authManager->isAuthorized()) {
            if (!authManager->authorize(_serial, _baud)) {
                MYQ_ERROR_LINE("Error refreshing auth token.");
            }
        }
    }
}

int MyQ::getGarageState(String doorSerial) {
    if (accountId.length() == 0) {
        if (getAccountId().length() == 0) return MYQ_DOOR_GETSTATE_UNKNOWN;
    }

    StaticJsonDocument<128> filter;
    filter["items"][0]["serial_number"] = doorSerial;
    filter["items"][0]["state"]["door_state"] = true;

    DynamicJsonDocument res = authManager->request(
        "https://devices.myq-cloud.com/api/v5.2/Accounts/" + accountId + "/Devices",
        256,
        true,
        "GET",
        "",
        StaticJsonDocument<0>(),
        filter
    );

    if (res.size() > 0) {
        const char *state = res["items"][0]["state"]["door_state"].as<const char*>();
        MYQ_LOG_LINE("Got state for %s: %s", doorSerial, state);
        for (int x = 0; x < sizeof(MYQ_DOOR_GETSTATE_VALUES) / sizeof(MYQ_DOOR_GETSTATE_VALUES[0]); x++) {
            if (strcmp(state, MYQ_DOOR_GETSTATE_VALUES[x]) == 0) return x;
        }
    }

    MYQ_ERROR_LINE("Error getting door state.");
    return MYQ_DOOR_GETSTATE_UNKNOWN;
}

int MyQ::setGarageState(String doorSerial, MYQ_DOOR_SETSTATE state) {
    StaticJsonDocument<192> headers;
    headers[0]["name"] = "Content-Length";
    headers[0]["value"] = "0";

    const char *newState = MYQ_DOOR_SETSTATE_VALUES[state];
    DynamicJsonDocument res = authManager->request(
        "https://account-devices-gdo.myq-cloud.com/api/v5.2/Accounts/" + accountId + "/door_openers/" + doorSerial + "/" + newState,
        256,
        true,
        "PUT",
        "",
        headers
    );

    if (res.size() > 0) {
        if (res["response"] == 202) {
            if (state == MYQ_DOOR_SETSTATE_CLOSE) return MYQ_DOOR_GETSTATE_CLOSING;
            else if (state == MYQ_DOOR_SETSTATE_OPEN) return MYQ_DOOR_GETSTATE_OPENING;
            else return MYQ_DOOR_GETSTATE_UNKNOWN;
        }
    }

    MYQ_ERROR_LINE("Error setting door state.");
    return MYQ_DOOR_GETSTATE_UNKNOWN;
}