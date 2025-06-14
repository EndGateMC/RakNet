/*
 *  Copyright (c) 2014, Oculus VR, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree. An additional grant
 *  of patent rights can be found in the PATENTS file in the same directory.
 *
 */

/// \file
/// \brief If _USE_RAK_MEMORY_OVERRIDE is defined, memory allocations go through rakMalloc, rakRealloc, and rakFree
///


#ifndef __RAK_MEMORY_H
#define __RAK_MEMORY_H

#include "Export.h"
#include "RakNetDefines.h"
#include <cstddef>
#include <new>

#include "RakAlloca.h"

// #if _USE_RAK_MEMORY_OVERRIDE==1
// 	#if defined(new)
// 		#pragma push_macro("new")
// 		#undef new
// 		#define RMO_NEW_UNDEF
// 	#endif
// #endif


// These pointers are statically and globally defined in RakMemoryOverride.cpp
// Change them to point to your own allocators if you want.
// Use the functions for a DLL, or just reassign the variable if using source
extern RAKNET_API void* (*rakMalloc)(size_t size);
extern RAKNET_API void* (*rakRealloc)(void* p, size_t size);
extern RAKNET_API void (*rakFree)(void* p);
extern RAKNET_API void* (*rakMalloc_Ex)(size_t size, const char* file, unsigned int line);
extern RAKNET_API void* (*rakRealloc_Ex)(void* p, size_t size, const char* file, unsigned int line);
extern RAKNET_API void (*rakFree_Ex)(void* p, const char* file, unsigned int line);
extern RAKNET_API void (*notifyOutOfMemory)(const char* file, const long line);
extern RAKNET_API void* (*dlMallocMMap)(size_t size);
extern RAKNET_API void* (*dlMallocDirectMMap)(size_t size);
extern RAKNET_API int (*dlMallocMUnmap)(void* ptr, size_t size);

// Change to a user defined allocation function
void RAKNET_API SetMalloc(void* (*userFunction)(size_t size));
void RAKNET_API SetRealloc(void* (*userFunction)(void* p, size_t size));
void RAKNET_API SetFree(void (*userFunction)(void* p));
void RAKNET_API SetMalloc_Ex(void* (*userFunction)(size_t size, const char* file, unsigned int line));
void RAKNET_API SetRealloc_Ex(void* (*userFunction)(void* p, size_t size, const char* file, unsigned int line));
void RAKNET_API SetFree_Ex(void (*userFunction)(void* p, const char* file, unsigned int line));
// Change to a user defined out of memory function
void RAKNET_API SetNotifyOutOfMemory(void (*userFunction)(const char* file, const long line));
void RAKNET_API SetDLMallocMMap(void* (*userFunction)(size_t size));
void RAKNET_API SetDLMallocDirectMMap(void* (*userFunction)(size_t size));
void RAKNET_API SetDLMallocMUnmap(int (*userFunction)(void* ptr, size_t size));

extern RAKNET_API void* (*GetMalloc())(size_t size);
extern RAKNET_API void* (*GetRealloc())(void* p, size_t size);
extern RAKNET_API void (*GetFree())(void* p);
extern RAKNET_API void* (*GetMalloc_Ex())(size_t size, const char* file, unsigned int line);
extern RAKNET_API void* (*GetRealloc_Ex())(void* p, size_t size, const char* file, unsigned int line);
extern RAKNET_API void (*GetFree_Ex())(void* p, const char* file, unsigned int line);
extern RAKNET_API void* (*GetDLMallocMMap())(size_t size);
extern RAKNET_API void* (*GetDLMallocDirectMMap())(size_t size);
extern RAKNET_API int (*GetDLMallocMUnmap())(void* ptr, size_t size);

namespace RakNet {

template <class Type>
RAKNET_API Type* OP_NEW(const char* file, unsigned int line) {
#if _USE_RAK_MEMORY_OVERRIDE == 1
    char* buffer = (char*)(GetMalloc_Ex())(sizeof(Type), file, line);
    Type* t      = new (buffer) Type;
    return t;
#else
    (void)file;
    (void)line;
    return new Type;
#endif
}

template <class Type, class P1>
RAKNET_API Type* OP_NEW_1(const char* file, unsigned int line, const P1& p1) {
#if _USE_RAK_MEMORY_OVERRIDE == 1
    char* buffer = (char*)(GetMalloc_Ex())(sizeof(Type), file, line);
    Type* t      = new (buffer) Type(p1);
    return t;
#else
    (void)file;
    (void)line;
    return new Type(p1);
#endif
}

template <class Type, class P1, class P2>
RAKNET_API Type* OP_NEW_2(const char* file, unsigned int line, const P1& p1, const P2& p2) {
#if _USE_RAK_MEMORY_OVERRIDE == 1
    char* buffer = (char*)(GetMalloc_Ex())(sizeof(Type), file, line);
    Type* t      = new (buffer) Type(p1, p2);
    return t;
#else
    (void)file;
    (void)line;
    return new Type(p1, p2);
#endif
}

template <class Type, class P1, class P2, class P3>
RAKNET_API Type* OP_NEW_3(const char* file, unsigned int line, const P1& p1, const P2& p2, const P3& p3) {
#if _USE_RAK_MEMORY_OVERRIDE == 1
    char* buffer = (char*)(GetMalloc_Ex())(sizeof(Type), file, line);
    Type* t      = new (buffer) Type(p1, p2, p3);
    return t;
#else
    (void)file;
    (void)line;
    return new Type(p1, p2, p3);
#endif
}

template <class Type, class P1, class P2, class P3, class P4>
RAKNET_API Type*
OP_NEW_4(const char* file, unsigned int line, const P1& p1, const P2& p2, const P3& p3, const P4& p4) {
#if _USE_RAK_MEMORY_OVERRIDE == 1
    char* buffer = (char*)(GetMalloc_Ex())(sizeof(Type), file, line);
    Type* t      = new (buffer) Type(p1, p2, p3, p4);
    return t;
#else
    (void)file;
    (void)line;
    return new Type(p1, p2, p3, p4);
#endif
}


template <class Type>
RAKNET_API Type* OP_NEW_ARRAY(const int count, const char* file, unsigned int line) {
    if (count == 0) return 0;

#if _USE_RAK_MEMORY_OVERRIDE == 1
    //		Type *t;
    char* buffer      = (char*)(GetMalloc_Ex())(sizeof(int) + sizeof(Type) * count, file, line);
    ((int*)buffer)[0] = count;
    for (int i = 0; i < count; i++) {
        // t =
        new (buffer + sizeof(int) + i * sizeof(Type)) Type;
    }
    return (Type*)(buffer + sizeof(int));
#else
    (void)file;
    (void)line;
    return new Type[count];
#endif
}

template <class Type>
RAKNET_API void OP_DELETE(Type* buff, const char* file, unsigned int line) {
#if _USE_RAK_MEMORY_OVERRIDE == 1
    if (buff == 0) return;
    buff->~Type();
    (GetFree_Ex())((char*)buff, file, line);
#else
    (void)file;
    (void)line;
    delete buff;
#endif
}

template <class Type>
RAKNET_API void OP_DELETE_ARRAY(Type* buff, const char* file, unsigned int line) {
#if _USE_RAK_MEMORY_OVERRIDE == 1
    if (buff == 0) return;

    int   count = ((int*)((char*)buff - sizeof(int)))[0];
    Type* t;
    for (int i = 0; i < count; i++) {
        t = buff + i;
        t->~Type();
    }
    (GetFree_Ex())((char*)buff - sizeof(int), file, line);
#else
    (void)file;
    (void)line;
    delete[] buff;
#endif
}

void RAKNET_API* _RakMalloc(size_t size);
void RAKNET_API* _RakRealloc(void* p, size_t size);
void RAKNET_API  _RakFree(void* p);
void RAKNET_API* _RakMalloc_Ex(size_t size, const char* file, unsigned int line);
void RAKNET_API* _RakRealloc_Ex(void* p, size_t size, const char* file, unsigned int line);
void RAKNET_API  _RakFree_Ex(void* p, const char* file, unsigned int line);
void RAKNET_API* _DLMallocMMap(size_t size);
void RAKNET_API* _DLMallocDirectMMap(size_t size);
int RAKNET_API   _DLMallocMUnmap(void* p, size_t size);

} // namespace RakNet

// Call to make RakNet allocate a large block of memory, and do all subsequent allocations in that memory block
// Initial and reallocations will be done through whatever function is pointed to by yourMMapFunction, and
// yourDirectMMapFunction (default is malloc) Allocations will be freed through whatever function is pointed to by
// yourMUnmapFunction (default free)
void UseRaknetFixedHeap(
    size_t initialCapacity,
    void* (*yourMMapFunction)(size_t size)          = RakNet::_DLMallocMMap,
    void* (*yourDirectMMapFunction)(size_t size)    = RakNet::_DLMallocDirectMMap,
    int (*yourMUnmapFunction)(void* p, size_t size) = RakNet::_DLMallocMUnmap
);

// Free memory allocated from UseRaknetFixedHeap
void FreeRakNetFixedHeap(void);

// #if _USE_RAK_MEMORY_OVERRIDE==1
// 	#if defined(RMO_NEW_UNDEF)
// 	#pragma pop_macro("new")
// 	#undef RMO_NEW_UNDEF
// 	#endif
// #endif

#endif
