// Copyright (C) 2016-2019 Semtech (International) AG. All rights reserved.
//
// This file is subject to the terms and conditions defined in file 'LICENSE',
// which is part of this source code package.

#ifndef _sha256_h_
#define _sha256_h_

#include <stdint.h>

void sha256( uint8_t* hash, const uint8_t* msg, uint32_t len );

void sha256i_init( void );

void sha256i_update( const uint8_t* msg, uint32_t len );

void sha256i_digest( uint8_t* hash );

#endif
