#ifndef __MYQ_H__
#define __MYQ_H__

#include "common.h"
#include "AuthManager.h"

class MyQ {
    private:
        MyQAuthenticationManager *authManager;

    public:
        MyQ();
        bool setup(String email, String password);
        void loop();
}

#endif