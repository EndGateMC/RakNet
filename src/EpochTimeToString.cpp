/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "EpochTimeToString.h"
#include "FormatString.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
// localtime
#include "LinuxStrings.h"
#include <time.h>

char* EpochTimeToString(long long time) {
    static int  textIndex = 0;
    static char text[4][64];

    if (++textIndex == 4) textIndex = 0;

    struct tm* timeinfo;
    time_t     t = time;
    timeinfo     = localtime(&t);
    strftime(text[textIndex], 64, "%c.", timeinfo);

    /*
    time_t
    // Copied from the docs
    struct tm *newtime;
    newtime = _localtime64(& time);
    asctime_s( text[textIndex], sizeof(text[textIndex]), newtime );

    while (text[textIndex][0] && (text[textIndex][strlen(text[textIndex])-1]=='\n' ||
    text[textIndex][strlen(text[textIndex])-1]=='\r')) text[textIndex][strlen(text[textIndex])-1]=0;
        */

    return text[textIndex];
}
