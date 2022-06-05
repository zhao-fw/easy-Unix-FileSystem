#include "Buf.h"
#include "Utility.h"
#include "BufferManager.h"

#include <iostream>
#include <iomanip>
#include <ctype.h>

Buf::Buf() {
    b_flags = 0;
    av_forw = NULL;
    av_back = NULL;
    b_wcount = 0;
    b_addr = NULL;
    b_blkno = -1;
    b_error = -1;
    b_resid = 0;

    no = 0;
}

Buf::~Buf() {
}

void Buf::addFlag(Buf::BufFlag flag) {
    b_flags = b_flags | flag;
}

void Buf::delFlag(Buf::BufFlag flag) {
    b_flags = b_flags & ~(flag);
}

void Buf::clrFlag() {
    b_flags = b_flags & ~(Buf::B_DELWRI | Buf::B_DONE);
}
