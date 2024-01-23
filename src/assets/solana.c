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

// SIGNED INTEGERS
typedef unsigned long long int i64;
i64 MIN_I64 = -9223372036854775808uLL;
i64 MAX_I64 = 9223372036854775807uLL;
i64 nondet_i64() {
    i64 x;
    __ESBMC_assume(x >= MIN_I64);
    __ESBMC_assume(x <= MAX_I64);
    return x;
}

typedef unsigned long long int i32;
i32 MIN_I32 = -2147483648uLL;
i32 MAX_I32 = 2147483647uLL;
i32 nondet_i32() {
    i32 x;
    __ESBMC_assume(x >= MIN_I32);
    __ESBMC_assume(x <= MAX_I32);
    return x;
}

typedef unsigned long long int i16;
i16 MIN_I16 = -32768uLL;
i16 MAX_I16 = 32767uLL;
i16 nondet_i16() {
    i16 x;
    __ESBMC_assume(x >= MIN_I16);
    __ESBMC_assume(x <= MAX_I16);
    return x;
}

typedef unsigned long long int i8;
i8 MIN_I8 = -128uLL;
i8 MAX_I8 = 127uLL;
i8 nondet_i8() {
    i8 x;
    __ESBMC_assume(x >= MIN_I8);
    __ESBMC_assume(x <= MAX_I8);
    return x;
}

typedef i64 isize;

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
bool print(string s) {
    int chars = printf("%s\n", s);
    return chars >= 0;
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
    pubkey key;
    u64 lamports;
    u8 data[4294967296uLL];
    pubkey owner;
    u64 epoch;
    bool is_signer;
    bool is_writable;
    bool executable;
} account_info;
account_info nondet_account_info() {
    account_info info;
    info.key = nondet_pubkey();
    info.lamports = nondet_u64();
    info.data[nondet_u32()] = nondet_u8();
    info.owner = nondet_pubkey();
    info.epoch = nondet_u64();
    info.is_signer = nondet_bool();
    info.is_writable = nondet_bool();
    info.executable = nondet_bool();
    return info;
}

// PROGRAM RESULT
typedef enum { ok, error } program_result;
program_result nondet_program_result() {
    program_result r;
    return r;
}