#ifndef CW_RESEARCH_BINARY_IO_H
#define CW_RESEARCH_BINARY_IO_H
#include "check.h"
#ifdef _WIN32
#include <io.h>
#include <fcntl.h>
#endif
static inline void research_binary_stdin(void){
#ifdef _WIN32
    CHECK(_setmode(_fileno(stdin),_O_BINARY)!=-1);
#endif
}
#endif
