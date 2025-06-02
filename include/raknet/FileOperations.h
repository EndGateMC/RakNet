/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/// \file FileOperations.h
///


#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_FileOperations == 1

#ifndef __FILE_OPERATIONS_H
#define __FILE_OPERATIONS_H

#include "Export.h"

bool RAKNET_API         WriteFileWithDirectories(const char* path, char* data, unsigned dataLength);
bool RAKNET_API         IsSlash(unsigned char c);
void RAKNET_API         AddSlash(char* input);
void RAKNET_API         QuoteIfSpaces(char* str);
bool RAKNET_API         DirectoryExists(const char* directory);
unsigned int RAKNET_API GetFileLength(const char* path);

#endif

#endif // _RAKNET_SUPPORT_FileOperations
