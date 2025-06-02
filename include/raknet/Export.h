/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

#include "RakNetDefines.h"

#ifdef _WIN32
#ifdef _RAKNET_EXPORT
#define RAKNET_API __declspec(dllexport)
#else
#ifdef _RANKET_DLL
#define RAKNET_API __declspec(dllimport)
#else
#define RAKNET_API
#endif
#endif
#else
#ifdef RAKNET_EXPORT
#define RAKNET_API __attribute__((visibility("default")))
#else
#define RAKNET_API
#endif
#endif

#define STATIC_FACTORY_DECLARATIONS(x)                                                                                 \
    static x*   GetInstance(void);                                                                                     \
    static void DestroyInstance(x* i);

#define STATIC_FACTORY_DEFINITIONS(x, y)                                                                               \
    x*   x::GetInstance(void) { return RakNet::OP_NEW<y>(_FILE_AND_LINE_); }                                           \
    void x::DestroyInstance(x* i) { RakNet::OP_DELETE((y*)i, _FILE_AND_LINE_); }
