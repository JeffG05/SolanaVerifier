#include <assert.h>
#include <stdbool.h>
#include "solana.h"

// Function inputs
pubkey _1;
account_info _2[256];
u8 _3[256];

// State data
program_result _0;
bool _4;
string _5;

void bb1() {
    _0 = ok;
    return;
}

void bb0() {
    _5 = "Hello, world!";
    _4 = print(_5);
    if (_4 == true) {
        return bb1();
    } else {
        assert(false);
    }
}