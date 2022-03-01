#ifndef __MYQ_H__
#define __MYQ_H__

#include "common.h"
#include "MyQAuthenticationManager.h"

enum MYQ_DOOR_GETSTATE {
    MYQ_DOOR_GETSTATE_UNKNOWN = -1,
    MYQ_DOOR_GETSTATE_CLOSED,
    MYQ_DOOR_GETSTATE_CLOSING,
    MYQ_DOOR_GETSTATE_OPEN,
    MYQ_DOOR_GETSTATE_OPENING,
    MYQ_DOOR_GETSTATE_STOPPED,
    MYQ_DOOR_GETSTATE_AUTOREVERSE
};

enum MYQ_DOOR_SETSTATE {
    MYQ_DOOR_SETSTATE_CLOSE = 0,
    MYQ_DOOR_SETSTATE_OPEN
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
        int getGarageState(String doorSerial);
        int setGarageState(String doorSerial, MYQ_DOOR_SETSTATE state);
};

#endif