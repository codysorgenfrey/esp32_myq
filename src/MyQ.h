#ifndef __MYQ_H__
#define __MYQ_H__

#include "common.h"
#include "MyQAuthenticationManager.h"

enum MYQ_DOOR_GETSTATE {
    GETSTATE_UNKNOWN = -1,
    GETSTATE_CLOSED,
    GETSTATE_CLOSING,
    GETSTATE_OPEN,
    GETSTATE_OPENING,
    GETSTATE_STOPPED,
    GETSTATE_AUTOREVERSE
};

enum MYQ_DOOR_SETSTATE {
    SETSTATE_CLOSED = 0,
    SETSTATE_OPEN
};

class MyQ {
    private:
        MyQAuthenticationManager *authManager;
        HardwareSerial *_serial;
        int _baud;
        String accountId;

        String getAccountId();

    public:
        MyQ();
        bool setup(HardwareSerial *inSerial = &Serial, int inBaud = 115200);
        void loop();
        MYQ_DOOR_GETSTATE getGarageState(String doorSerial);
        MYQ_DOOR_GETSTATE setGarageState(String doorSerial, MYQ_DOOR_SETSTATE state);
};

#endif