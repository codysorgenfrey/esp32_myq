#ifndef __MYQ_H__
#define __MYQ_H__

#include "common.h"
#include "MyQAuthenticationManager.h"

class MyQ {
    private:
        MyQAuthenticationManager *authManager;
        HardwareSerial *_serial;
        int _baud;

    public:
        MyQ();
        bool setup(String email, String password, HardwareSerial *inSerial = &Serial, int inBaud = 115200);
        void loop();
        int getGarageState(String serial);
        int setGarageState(String serial);
};

#endif