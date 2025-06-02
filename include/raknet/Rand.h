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
/// \brief \b [Internal] Random number generator
///


#ifndef __RAND_H
#define __RAND_H

#include "Export.h"

/// Initialise seed for Random Generator
/// \note not threadSafe, use an instance of RakNetRandom if necessary per thread
/// \param[in] seed The seed value for the random number generator.
extern void RAKNET_API seedMT(unsigned int seed);

/// \internal
/// \note not threadSafe, use an instance of RakNetRandom if necessary per thread
extern unsigned int RAKNET_API reloadMT(void);

/// Gets a random unsigned int
/// \note not threadSafe, use an instance of RakNetRandom if necessary per thread
/// \return an integer random value.
extern unsigned int RAKNET_API randomMT(void);

/// Gets a random float
/// \note not threadSafe, use an instance of RakNetRandom if necessary per thread
/// \return 0 to 1.0f, inclusive
extern float RAKNET_API frandomMT(void);

/// Randomizes a buffer
/// \note not threadSafe, use an instance of RakNetRandom if necessary per thread
extern void RAKNET_API fillBufferMT(void* buffer, unsigned int bytes);

namespace RakNet {

// Same thing as above functions, but not global
class RAKNET_API RakNetRandom {
public:
    RakNetRandom();
    ~RakNetRandom();
    void         SeedMT(unsigned int seed);
    unsigned int ReloadMT(void);
    unsigned int RandomMT(void);
    float        FrandomMT(void);
    void         FillBufferMT(void* buffer, unsigned int bytes);

protected:
    unsigned int  state[624 + 1];
    unsigned int* next;
    int           left;
};

} // namespace RakNet

#endif
