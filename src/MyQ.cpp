#include "MyQ.h"
#include "MyQAuthenticationManager.h"

//
// Private member functions
//



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
        MYQ_LOG_LINE("Failed to authorize with MyQ.");
        return false;
    }

    return true;
}

void MyQ::loop() {
    if (millis() % 60000 == 0) { // check once every minute
        if (!authManager->isAuthorized()) {
            if (!authManager->authorize(_serial, _baud)) {
                MYQ_LOG_LINE("Error refreshing auth token.");
            }
        }
    }
}

int MyQ::getGarageState(String doorSerial) {
    return -1;
}

int MyQ::setGarageState(String doorSerial) {
    return -1;
}