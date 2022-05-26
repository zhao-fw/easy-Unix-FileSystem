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

void Buf::addFlag(BufFlag flag) {
    b_flags = b_flags | flag;
}

void Buf::delFlag(BufFlag flag) {
    b_flags = b_flags & ~(flag);
}

void Buf::clrFlag() {
    b_flags = b_flags & ~(Buf::B_DELWRI | Buf::B_DONE);
}

void Buf::debugContent() {
    using namespace std;
    debugMark();
    for (int i = 0; i < BufferManager::BUFFER_SIZE; i += 32) {
        cout << "  " << setfill('0') << setw(4) << hex << i << ": ";
        for (int j = i; j < i + 32; ++j) {
            cout << ((j + 1 - i % 8 == 0) ? " - " : " ");
            cout << setfill('0') << setw(2) << hex << (unsigned int)(unsigned char)b_addr[j];
        }
        cout << "  " << setw(4) << *(int*)(b_addr + i) << " ";
        for (int j = i + 4; j < i + 32; ++j) {
            cout << (isprint(b_addr[j]) ? (char)b_addr[j] : '.');
        }
        cout << endl;
    }
    cout << dec;
}

void Buf::debugMark() {
    using namespace std;
    cout << "no = " << no << " blkno = " << b_blkno << " flag = " << (b_flags & B_DONE ? " DONE " : " ") << (b_flags & B_DELWRI ? " DELWRI " : " ") << endl;
}