#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

// UNSIGNED INTEGERS
typedef unsigned long long int u64;
u64 MAX_U64 = 18446744073709551615uLL;
u64 nondet_u64() {
    u64 x;
    __ESBMC_assume(x <= MAX_U64);
    return x;
}

typedef unsigned long long int u32;
u32 MAX_U32 = 4294967295uLL;
u32 nondet_u32() {
    u32 x;
    __ESBMC_assume(x <= MAX_U32);
    return x;
}

typedef unsigned long long int u16;
u16 MAX_U16 = 65535uLL;
u16 nondet_u16() {
    u16 x;
    __ESBMC_assume(x <= MAX_U16);
    return x;
}

typedef unsigned long long int u8;
u8 MAX_U8 = 255uLL;
u8 nondet_u8() {
    u8 x;
    __ESBMC_assume(x <= MAX_U8);
    return x;
}

typedef u64 usize;
usize MAX_USIZE = 18446744073709551615uLL;
usize nondet_usize() {
    usize x;
    __ESBMC_assume(x <= MAX_USIZE);
    return x;
}

// SIGNED INTEGERS
typedef signed long long int i64;
i64 MIN_I64 = -9223372036854775807LL;
i64 MAX_I64 = 9223372036854775807LL;
i64 nondet_i64() {
    i64 x;
    __ESBMC_assume(x >= MIN_I64);
    __ESBMC_assume(x <= MAX_I64);
    return x;
}

typedef signed long long int i32;
i32 MIN_I32 = -2147483647LL;
i32 MAX_I32 = 2147483647LL;
i32 nondet_i32() {
    i32 x;
    __ESBMC_assume(x >= MIN_I32);
    __ESBMC_assume(x <= MAX_I32);
    return x;
}

typedef signed long long int i16;
i16 MIN_I16 = -32767LL;
i16 MAX_I16 = 32767LL;
i16 nondet_i16() {
    i16 x;
    __ESBMC_assume(x >= MIN_I16);
    __ESBMC_assume(x <= MAX_I16);
    return x;
}

typedef signed long long int i8;
i8 MIN_I8 = -127LL;
i8 MAX_I8 = 127LL;
i8 nondet_i8() {
    i8 x;
    __ESBMC_assume(x >= MIN_I8);
    __ESBMC_assume(x <= MAX_I8);
    return x;
}

typedef i64 isize;
isize MIN_ISIZE = -9223372036854775807LL;
isize MAX_ISIZE = 9223372036854775807LL;
isize nondet_isize() {
    isize x;
    __ESBMC_assume(x >= MIN_ISIZE);
    __ESBMC_assume(x <= MAX_ISIZE);
    return x;
}

// MATH OPERATORS
typedef struct unsigned_math_result_struct {
    unsigned long long int value;
    bool errors;
} unsigned_math_result;

typedef struct signed_math_result_struct {
    signed long long int value;
    bool errors;
} signed_math_result;

unsigned_math_result u_addition(unsigned long long int a, unsigned long long int b, unsigned long long int max) {
    unsigned_math_result result;
    result.value = a + b;
    result.errors = a > max - b;
    __ESBMC_assert(!result.errors, "Vulnerability Found: Addition Overflow");
    return result;
}

signed_math_result i_addition(signed long long int a, signed long long int b, signed long long int max, signed long long int min) {
    signed_math_result result;
    result.value = a + b;
    if (b < 0) {
        result.errors = a < min - b;
        __ESBMC_assert(!result.errors, "Vulnerability Found: Addition Underflow");
    } else {
        result.errors = a > max - b;
        __ESBMC_assert(!result.errors, "Vulnerability Found: Addition Overflow");
    }
    return result;
}

unsigned_math_result u_multiplication(unsigned long long int a, unsigned long long int b, unsigned long long int max) {
    unsigned_math_result result;
    result.value = a * b;
    if (b == 0) {
        result.errors = false;
    } else {
        result.errors = a > max / b;
        __ESBMC_assert(!result.errors, "Vulnerability Found: Multiplication Overflow");
    }
    return result;
}

signed_math_result i_multiplication(signed long long int a, signed long long int b, signed long long int max, signed long long int min) {
    signed_math_result result;
    result.value = a * b;
    if (b == 0) {
        result.errors = false;
    } else if ((b > 0) && (a > 0)) {
        result.errors = a > max / b;
        __ESBMC_assert(!result.errors, "Vulnerability Found: Multiplication Overflow");
    } else if ((b < 0) && (a < 0)) {
        result.errors = a < max / b;
        __ESBMC_assert(!result.errors, "Vulnerability Found: Multiplication Overflow");
    } else if ((b > 0) && (a < 0)) {
        result.errors = a < min / b;
        __ESBMC_assert(!result.errors, "Vulnerability Found: Multiplication Underflow");
    } else if ((b < 0) && (a > 0)) {
        result.errors = a > min / b;
        __ESBMC_assert(!result.errors, "Vulnerability Found: Multiplication Underflow");
    }
    return result;
}

// STRING
typedef char* string;
unsigned int MAX_STRING_LEN = 1024;
string nondet_string() {
    unsigned int len = nondet_uint();
    __ESBMC_assume(len < MAX_STRING_LEN);

    char* str = malloc(len + 1);
    __ESBMC_assume(str != NULL);

    unsigned int i = nondet_uint();
    __ESBMC_assume(i < len);
    str[i] = nondet_char();
    str[len] = '\0';

    return str;
}
void print(string s) {
    printf("%s\n", s);
}

// PUBKEY
typedef struct pubkey_struct {
    u8 contents[32];
} pubkey;
pubkey nondet_pubkey() {
    pubkey p;
    unsigned int i = nondet_uint();
    __ESBMC_assume(i < 32);
    p.contents[i] = nondet_u8();
    return p;
}
bool is_equal(pubkey lhs, pubkey rhs) {
    unsigned int i = nondet_uint();
    __ESBMC_assume(i < 32);
    return lhs.contents[i] == rhs.contents[i];
}

// ACCOUNT INFO
typedef struct account_info_struct {
    pubkey get0;
    u64 get1;
    u8 get2[4294967296uLL];
    pubkey get3;
    u64 get4;
    bool get5;
    bool get6;
    bool get7;
} account_info;
account_info nondet_account_info() {
    account_info info;
    info.get0 = nondet_pubkey();
    info.get1 = nondet_u64();
    info.get2[nondet_u32()] = nondet_u8();
    info.get3 = nondet_pubkey();
    info.get4 = nondet_u64();
    info.get5 = nondet_bool();
    info.get6 = nondet_bool();
    info.get7 = nondet_bool();
    return info;
}

// INFALLIBLE
typedef enum { unreachable } infallible;
infallible nondet_infallible() {
    infallible i;
    return i;
}

// CONTROL FLOW
typedef enum { _continue, _break } controlflow;
controlflow nondet_controlflow() {
    controlflow c;
    return c;
}