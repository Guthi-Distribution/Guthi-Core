#include <stdio.h>
#include "parser_cpuinfo.hpp"
#if defined(__gnu_linux__) || defined(__linux__) || defined(linux) || defined(__linux)
#include <sys/stat.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

constexpr std::string_view keywords[] = {
    "cpu",
    "mhz",
    "max"
};

static bool is_equal_case_insensitive(std::string_view s1, const std::string_view s2){
    const int size_1 = s1.size();
    const int size_2 = s2.size();

    if (size_1 != size_2){
        return false;
    } 
    for(int i = 0; i < size_1; i++){
        if(tolower(s1[i]) != tolower(s2[i])){
            return false;
        }
    }

    return true;
}


static bool tokenize(Tokenizer *t) {
    const char *ptr = t->ptr;
    while (ptr) {
        if (*ptr == '\0') {
            t->type = TOKEN_END,
            ptr ++;
            t->ptr = ptr;
            return true;
        }
        if(*ptr == ' ' || *ptr == '\t') {
            while(*ptr == ' ' || *ptr == '\t') {
                ptr++; 
            }
            continue;
        }

        if (*ptr == ':') {
            t->type = TOKEN_COLON,
            t->id = ":";
            ptr ++;
            t->ptr = ptr;
            return true;
        }

        if(is_char(*ptr)){
            const char *start = ptr;
            while(is_char(*ptr) || *ptr == '_' || is_num(*ptr)){
                ptr++;
            }
            const char *end = ptr;
            t->id = std::string_view(start, end - start);
            t->type = TOKEN_ID;

            for(int i = 0; i < (int)ArrayCount(keywords); i++){
                if(is_equal_case_insensitive(keywords[i], t->id)){
                    t->type = (TokenType) i;

                    break;
                }
            }

            t->ptr = ptr;
            return true;
        }

        if(is_num(*ptr)){
            t->type = TOKEN_NUMBER;
            const char *start = ptr;
            while(is_num(*ptr)){
                ptr++;
            }
            t->value = strtoull(start, (char **)&t->ptr, 10);
            t->ptr = ptr;
            return true;
        }

        ptr++;
    }
}

static bool getToken(Parser *parser){
    parser->is_parsing = tokenize(&parser->tokenizer);

    return parser->is_parsing;
}


static bool acceptToken(Parser *parser, TokenType type, TokenType *out = nullptr){
    if(parser->tokenizer.type == type){
        if (out) {
            *out = type;
        }
        return getToken(parser);;
    }

    return false;
}


static bool expectToken(Parser *parser, TokenType type, TokenType *out = NULL){
    if (acceptToken(parser, type, out)){
        return true;
    }
    parser->is_parsing = false;

    return false;
}

static void parse_cpu_info(Parser *parser, Runtime::ProcessorStatus *processor_status) {
    tokenize(&parser->tokenizer);
    int index = 0;
    while (parser->is_parsing) {
        if (acceptToken(parser, TOKEN_CPU)) {
            if (acceptToken(parser, TOKEN_MHZ)) {
                if (expectToken(parser, TOKEN_COLON)) {
                    int mhz_value = parser->tokenizer.value;
                    if (expectToken(parser, TOKEN_NUMBER)) {
                        processor_status->processor_count++;
                        processor_status->processors[index].processor_number = index;
                        processor_status->processors[index].current_mhz = mhz_value; 
                        index++;
                    }      
                }
            }
        } else if (acceptToken(parser, TOKEN_ID)) {

        }  else if (acceptToken(parser, TOKEN_COLON)) {

        } else if (acceptToken(parser, TOKEN_NUMBER)) {

        } else if (acceptToken(parser, TOKEN_END)) {
            break;
        }
    }
} 

static void parse_maximum_cpu_hz(Parser *parser, Runtime::ProcessorStatus *processor_status) {
    tokenize(&parser->tokenizer);
    int value = 0;
    while (parser->is_parsing) {
        if (acceptToken(parser, TOKEN_CPU)) {
            if (acceptToken(parser, TOKEN_MAX)) {
                if (acceptToken(parser, TOKEN_MHZ)) {
                    if (expectToken(parser, TOKEN_COLON)) {
                        value = parser->tokenizer.value;
                        if (expectToken(parser, TOKEN_NUMBER)) {

                        }      
                    }
            }
            }
        } else if (acceptToken(parser, TOKEN_ID)) {

        }  else if (acceptToken(parser, TOKEN_COLON)) {

        } else if (acceptToken(parser, TOKEN_NUMBER)) {

        } else if (acceptToken(parser, TOKEN_END)) {
            break;
        }
    }

    for (int i = 0; i < processor_status->processor_count; i++) {
        processor_status->processors->total_mhz = value;
    }
} 

void GetSystemInfo(Runtime::ProcessorStatus *processor_status) {
#if 0
    int fd = open("/proc/cpuinfo", O_RDONLY);
    const int size = sysconf(_SC_PAGESIZE);
    char *content = new char[size + 1];
    memset(content, 0, size);
    content[size] = '\0';
    read(fd, content, 1024);

    Parser parser;
    parser.is_parsing = true;
    parser.tokenizer.ptr = content;
    parse(&parser, processor_status);
    close(fd);
    delete content;
#endif
    FILE *cpu_info = popen("cat /proc/cpuinfo | grep MHz", "r");
    if (!cpu_info) {
        perror("Command execution error");
    }
    const int buff_size = 1024;
    char *buffer = new char[buff_size + 1];
    memset(buffer, 0, buff_size);
    buffer[buff_size] = '\0';
    buffer = new char[buff_size + 1];
    read(fileno(cpu_info), buffer, buff_size);
    buffer[buff_size] = '\0';

    Parser parser;
    parser.is_parsing = true;
    parser.tokenizer.ptr = buffer;
    parse_cpu_info(&parser, processor_status);
    pclose(cpu_info);

    cpu_info = popen("lscpu | grep \"CPU max MHz\" ", "r");
    if (!cpu_info) {
        perror("Command execution error");
    }
    memset(buffer, 0, buff_size);
    buffer[buff_size] = '\0';
    buffer = new char[buff_size + 1];
    read(fileno(cpu_info), buffer, buff_size);
    buffer[buff_size] = '\0';
    parser.tokenizer.ptr = buffer;
    parse_maximum_cpu_hz(&parser, processor_status);
    pclose(cpu_info);
    free(buffer);
}

#endif