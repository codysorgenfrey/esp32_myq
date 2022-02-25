#include "MyQ.h"
#include "AuthManager.h"

//
// Private member functions
//



//
// Public member functions
//

MyQ::MyQ() {
    authManager = new MyQAuthenticationManger();
}

bool MyQ::setup(String email, String password) {

}

void MyQ::loop() {

}