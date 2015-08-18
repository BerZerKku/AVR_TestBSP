/*
 * M29F800B.H
 *
 *  Created on: 05.08.2015
 *      Author: Shcheblykin
 */

#ifndef M29F800B_H_
#define M29F800B_H_

#include "stdint.h"

/****M29F800B.H*Header File for M29F800B.C**************************************
Filename: m29f800b.h
Description: Header file for m29f800b.c. Consult the C file for details
Copyright (c) 1997 STMicroelectronics.
This program is provided "AS IS" WITHOUT WARRANTY OF ANY KIND,EITHER
EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTY
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK
AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE
PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
REPAIR OR CORRECTION.
******************************************************************************
Commands for the various functions
******************************************************************************/
#define FLASH_READ_MANUFACTURER (-2)
#define FLASH_READ_DEVICE_CODE (-1)
#define FLASH_UNPROTECT (0)
#define FLASH_PROTECT (1)
#define FLASH_MAIN_BLOCK (0)
#define FLASH_BOOT_BLOCK (1)

/*******************************************************************************
Error Conditions and return values.
See end of C file for explanations and help
*******************************************************************************/
#define FLASH_BLOCK_PROTECTED (0x01)
#define FLASH_BLOCK_UNPROTECTED (0x00)
#define FLASH_BLOCK_NOT_ERASED (0xFF)
#define FLASH_BLOCK_ERASE_FAILURE (0xFE)
#define FLASH_BLOCK_ERASED (0xFD)
#define FLASH_SUCCESS (-1)
#define FLASH_POLL_FAIL (-2)
#define FLASH_TOO_MANY_BLOCKS (-3)
#define FLASH_MPU_TOO_SLOW (-4)
#define FLASH_BLOCK_INVALID (-5)
#define FLASH_PROGRAM_FAIL (-6)
#define FLASH_OFFSET_OUT_OF_RANGE (-7)
#define FLASH_WRONG_TYPE (-8)
#define FLASH_BLOCK_FAILED_ERASE (-9)
#define FLASH_UNPROTECTED (-10)
#define FLASH_PROTECTED (-11)

/*******************************************************************************
Function Prototypes
*******************************************************************************/
extern unsigned char FlashRead( unsigned long ulOff );
extern void FlashReadReset( void );
extern uint8_t FlashAutoSelect(int8_t iFunc);
extern int FlashBlockErase( unsigned char ucNumBlocks, unsigned char ucBlock[]);
extern int FlashChipErase( unsigned char *Results );
extern int FlashProgram( unsigned long Off, size_t NumBytes, void *Array );
extern int FlashUnprotect( int Unprotect, int BlockType);
extern char *FlashErrorStr( int ErrNum );

#endif /* M29F800B_H_ */
