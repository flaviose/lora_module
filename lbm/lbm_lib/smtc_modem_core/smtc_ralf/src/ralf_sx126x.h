/**
 * @file      ralf_sx126x.h
 *
 * @brief     Radio abstraction layer definition
 *
 * The Clear BSD License
 * Copyright Semtech Corporation 2021. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted (subject to the limitations in the disclaimer
 * below) provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Semtech corporation nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY
 * THIS LICENSE. THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND
 * CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT
 * NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL SEMTECH CORPORATION BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef RALF_SX126X_H__
#define RALF_SX126X_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
 * -----------------------------------------------------------------------------
 * --- DEPENDENCIES ------------------------------------------------------------
 */

#include <stdint.h>
#include <stdbool.h>

#include "ral_sx126x.h"
#include "ralf.h"

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC MACROS -----------------------------------------------------------
 */

#define RALF_DRV_SX126X_INSTANTIATE                                                         \
    {                                                                                       \
        .setup_gfsk = ralf_sx126x_setup_gfsk, .setup_lora = ralf_sx126x_setup_lora,         \
        .setup_flrc = ralf_sx126x_setup_flrc, .setup_lora_cad = ralf_sx126x_setup_lora_cad, \
    }

#define RALF_SX126X_INSTANTIATE( ctx )                                                 \
    {                                                                                  \
        .ral = RAL_SX126X_INSTANTIATE( ctx ), .ralf_drv = RALF_DRV_SX126X_INSTANTIATE, \
    }

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC CONSTANTS --------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC TYPES ------------------------------------------------------------
 */

/*
 * -----------------------------------------------------------------------------
 * --- PUBLIC FUNCTIONS PROTOTYPES ---------------------------------------------
 */

/**
 * @see ralf_setup_gfsk
 */
ral_status_t ralf_sx126x_setup_gfsk( const ralf_t* radio, const ralf_params_gfsk_t* params );

/**
 * @see ralf_setup_lora
 */
ral_status_t ralf_sx126x_setup_lora( const ralf_t* radio, const ralf_params_lora_t* params );

/**
 * @see ralf_setup_flrc
 */
ral_status_t ralf_sx126x_setup_flrc( const ralf_t* radio, const ralf_params_flrc_t* params );

/**
 * @see ralf_setup_lora_cad
 */
ral_status_t ralf_sx126x_setup_lora_cad( const ralf_t* radio, const ralf_params_lora_cad_t* params );

#ifdef __cplusplus
}
#endif

#endif  // RALF_SX126X_H__

/* --- EOF ------------------------------------------------------------------ */
