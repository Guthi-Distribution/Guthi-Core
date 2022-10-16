#pragma once
#include <stdio.h>


#if defined(__gnu_linux__) || defined(__linux__) || defined(linux) || defined(__linux)
#include <string_view>
#include <cstring>
#include <string>
#include <runtime/sys_info.hpp>
#include <stdio.h>


#define ArrayCount(a) (int)(sizeof(a) / sizeof(a[0]))
#define minimum(a, b) a < b? a: b
#define is_char(c) (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z')
#define is_num(c) (c >= '0' && c <= '9')


enum TokenType {
    TOKEN_CPU,
    TOKEN_MHZ,
    TOKEN_MAX,
    TOKEN_ID,
    TOKEN_NUMBER,
    TOKEN_INTEGER,
    TOKEN_FLOAT,
    TOKEN_COLON,
    TOKEN_END
};

struct Tokenizer {
    const char *ptr;
    TokenType type;
    double value;
    std::string_view id;
};

struct Parser {
    Tokenizer tokenizer;
    bool is_parsing;
};


void GetSystemInfo(Runtime::ProcessorStatus *processor_status);
#endif