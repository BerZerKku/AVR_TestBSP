/*
 * m29f800b.c
 *
 *  Created on: 05.08.2015
 *      Author: Shcheblykin
 */

/****M29F800, M29W800 *8Mb Flash Memory*8bit mode***********************
 Filename: m29f800b.c
 Description: Library routines for the M29F800 and M29W800 8Mbit
 (1024kx8) Flash Memories.
 8 bit drivers.
 Version: 1.00, Tested on M29W800s only!
 Date: 23/03/98
 Author: Brendan Watts, OTS & Alexandre Nairac
 Copyright (c) 1998 STMicroelectronics.
 This program is provided "AS IS" WITHOUT WARRANTY OF ANY KIND,EITHER
 EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTY
 OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE ENTIRE RISK
 AS TO THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE
 PROGRAM PROVE DEFECTIVE, YOU ASSUME THE COST OF ALL NECESSARY SERVICING,
 REPAIR OR CORRECTION.
 ********************************************************************************
 Version History.
 Ver. Date Comments
 0.00 04/03/98 Initial modifications with no testing
 1.00 23/03/98 Tested on M29W800s only
 ********************************************************************************
 This source file provides library C code for using the M29x800 devices.
 The following devices are supported in the code:
 M29F800T
 M29F800B
 M29W800T
 M29W800B
 This file is used to access the devices in 8-bit mode only.
 A separate file is available for users who wish to access the
 device in 16 bit mode (m29f800a.c).
 The following functions are available in this library:
 FlashReadReset() to reset the flash for normal memory access
 FlashAutoSelect() to get information about the device
 FlashBlockErase() to erase one or more blocks
 FlashChipErase() to erase the whole chip
 FlashProgram() to program a byte or an array
 FlashErrorStr() to return the error string of an error
 For further information consult the Data Sheet and the Application Note.
 The Application Note gives information about how to modify this code for
 a specific application.
 The hardware specific functions which need to be modified by the user are:
 FlashWrite() for writing a word to the flash
 FlashRead() for reading a word from the flash
 FlashPause() for timing short pauses (in micro seconds)
 A list of the error conditions is at the end of the code.
 There are no timeouts implemented in the loops in the code. At each point
 where an infinite loop is implemented a comment /# TimeOut! #/ has been
 placed. It is up to the user to implement these to avoid the code hanging
 instead of timing out.
 C does not include a method for disabling interrupts to keep time-critical
 sections of code from being interrupted. The user may wish to disable
 interrupts during parts of the code to avoid the FLASH_MPU_TO_SLOW error
 from occuring if an interrupt occurs at the wrong time. Where interrupts
 should be disabled and re-enabled there is a /# DSI! #/ or /# ENI! #/
 comment.
 *******************************************************************************/
#include <stdlib.h>
#include "m29f800b.h" /* Header file with global prototypes */
#define USE_M29F800T
/*******************************************************************************
 Constants
 *******************************************************************************/
#define COUNTS_PER_MICROSECOND (40)
#define MANUFACRURER_ST (0x20)
#define MANUFACTURER_MICRON (0x01)
#define MANUFACTURER MANUFACTURER_MICRON
#define BASE_ADDR ((volatile unsigned char*) 0x8000UL)
/* BASE_ADDR is the base address of the flash, see the functions FlashRead
 and FlashWrite(). Some applications which require a more complicated
 FlashRead() or FlashWrite() may not use BASE_ADDR */
#if defined(USE_M29F800T) | defined(USE_M29W800T)
static const unsigned long BlockOffset[] = {
		0x00000L, /* Start offset of block 0 */
		0x10000L, /* Start offset of block 1 */
		0x20000L, /* Start offset of block 2 */
		0x30000L, /* Start offset of block 3 */
		0x40000L, /* Start offset of block 4 */
		0x50000L, /* Start offset of block 5 */
		0x60000L, /* Start offset of block 6 */
		0x70000L, /* Start offset of block 7 */
		0x80000L, /* Start offset of block 8 */
		0x90000L, /* Start offset of block 9 */
		0xA0000L, /* Start offset of block 10 */
		0xB0000L, /* Start offset of block 11 */
		0xC0000L, /* Start offset of block 12 */
		0xD0000L, /* Start offset of block 13 */
		0xE0000L, /* Start offset of block 14 */
		0xF0000L, /* Start offset of block 15 */
		0xF8000L, /* Start offset of block 16 */
		0xFA000L, /* Start offset of block 17 */
		0xFC000L /* Start offset of block 18 */
};

static const unsigned int DeviceBlockType[] = {
		FLASH_MAIN_BLOCK, /* Block 0 */
		FLASH_MAIN_BLOCK, /* Block 1 */
		FLASH_MAIN_BLOCK, /* Block 2 */
		FLASH_MAIN_BLOCK, /* Block 3 */
		FLASH_MAIN_BLOCK, /* Block 4 */
		FLASH_MAIN_BLOCK, /* Block 5 */
		FLASH_MAIN_BLOCK, /* Block 6 */
		FLASH_MAIN_BLOCK, /* Block 7 */
		FLASH_MAIN_BLOCK, /* Block 8 */
		FLASH_MAIN_BLOCK, /* Block 9 */
		FLASH_MAIN_BLOCK, /* Block 10 */
		FLASH_MAIN_BLOCK, /* Block 11 */
		FLASH_MAIN_BLOCK, /* Block 12 */
		FLASH_MAIN_BLOCK, /* Block 13 */
		FLASH_MAIN_BLOCK, /* Block 14 */
		FLASH_MAIN_BLOCK, /* Block 15 */
		FLASH_MAIN_BLOCK, /* Block 16 */
		FLASH_MAIN_BLOCK, /* Block 17 */
		FLASH_BOOT_BLOCK  /* Block 18 */
};
#endif
#ifdef USE_M29F800T
	#if (MANUFACTURER == MANUFACTURER_MICRON)
	#define EXPECTED_DEVICE (0xD6) /* M29F800T */
	#endif

	#if (MANUFACTURER == MANUFACTURER_ST)
	#define EXPECTED_DEVICE (0x00EC) /* M29F800T */
	#endif
#endif
#ifdef USE_M29W800T
#define EXPECTED_DEVICE (0xD7) /* M29W800T */
#endif

#if defined(USE_M29F800B) | defined(USE_M29W800B)
static const unsigned long BlockOffset[] = {
	0x00000L, /* Start offset of block 0 */
	0x04000L, /* Start offset of block 1 */
	0x06000L, /* Start offset of block 2 */
	0x08000L, /* Start offset of block 3 */
	0x10000L, /* Start offset of block 4 */
	0x20000L, /* Start offset of block 5 */
	0x30000L, /* Start offset of block 6 */
	0x40000L, /* Start offset of block 7 */
	0x50000L, /* Start offset of block 8 */
	0x60000L, /* Start offset of block 9 */
	0x70000L, /* Start offset of block 10 */
	0x80000L, /* Start offset of block 11 */
	0x90000L, /* Start offset of block 12 */
	0xA0000L, /* Start offset of block 13 */
	0xB0000L, /* Start offset of block 14 */
	0xC0000L, /* Start offset of block 15 */
	0xD0000L, /* Start offset of block 16 */
	0xE0000L, /* Start offset of block 17 */
	0xF0000L /* Start offset of block 18 */
};

static const unsigned int DeviceBlockType[] = {
	FLASH_BOOT_BLOCK, /* Block 0 */
	FLASH_MAIN_BLOCK, /* Block 1 */
	FLASH_MAIN_BLOCK, /* Block 2 */
	FLASH_MAIN_BLOCK, /* Block 3 */
	FLASH_MAIN_BLOCK, /* Block 4 */
	FLASH_MAIN_BLOCK, /* Block 5 */
	FLASH_MAIN_BLOCK, /* Block 6 */
	FLASH_MAIN_BLOCK, /* Block 7 */
	FLASH_MAIN_BLOCK, /* Block 8 */
	FLASH_MAIN_BLOCK, /* Block 9 */
	FLASH_MAIN_BLOCK, /* Block 10 */
	FLASH_MAIN_BLOCK, /* Block 11 */
	FLASH_MAIN_BLOCK, /* Block 12 */
	FLASH_MAIN_BLOCK, /* Block 13 */
	FLASH_MAIN_BLOCK, /* Block 14 */
	FLASH_MAIN_BLOCK, /* Block 15 */
	FLASH_MAIN_BLOCK, /* Block 16 */
	FLASH_MAIN_BLOCK, /* Block 17 */
	FLASH_MAIN_BLOCK /* Block 18 */
};
#endif

#ifdef USE_M29F800B
#define EXPECTED_DEVICE (0x) /* M29F800B */
#endif
#ifdef USE_M29W800B
#define EXPECTED_DEVICE (0x5B) /* M29W800B */
#endif
#define NUM_BLOCKS (sizeof(BlockOffset)/sizeof(BlockOffset[0]))
#define FLASH_SIZE (0x100000L) /* 1024k x8 */
/*******************************************************************************
 Static Prototypes
 The following functions are only needed in this module.
 *******************************************************************************/
static unsigned char FlashWrite(unsigned long ulOff, unsigned char ucVal);
static void FlashPause(unsigned int uMicroSeconds);
static int FlashDataPoll(unsigned long ulOff, unsigned char ucVal);
static int FlashBlockFailedErase(unsigned char ucBlock);
/*******************************************************************************
 Function: unsigned char FlashWrite( unsigned long ulOff, unsigned char ucVal)
 Arguments: ulOff is byte offset into the flash to write to.
 ucVal is the value to be written
 Returns: ucVal
 Description: This function is used to write a value to the flash. On many
 microprocessor systems a macro can be used instead, increasing the speed of
 the flash routines. For example:
 #define FlashWrite( ulOff, ucVal ) ( BASE_ADDR[ulOff] = (unsigned char) ucVal )
 A function is used here instead to allow the user to expand it if necessary.
 The function is made to return ucVal so that it is compatible with the macro.
 Pseudo Code:
 Step 1: Write ucVal to the byte offset in the flash
 Step 2: return ucVal
 *******************************************************************************/
static unsigned char FlashWrite(unsigned long ulOff, unsigned char ucVal) {
	/* Step1, 2: Write uVal to the word offset in the flash and return it */
	BASE_ADDR [ulOff] = ucVal;
	return ucVal;
}
/*******************************************************************************
 Function: unsigned char FlashRead( unsigned long ulOff )
 Arguments: ulOff is byte offset into the flash to read from.
 Returns: The unsigned char at the byte offset
 Description: This function is used to read a byte from the flash. On many
 microprocessor systems a macro can be used instead, increasing the speed of
 the flash routines. For example:
 #define FlashRead( ulOff ) ( BASE_ADDR[ulOff] )
 A function is used here instead to allow the user to expand it if necessary.
 Pseudo Code:
 Step 1: Return the value at byte offset ulOff
 *******************************************************************************/
unsigned char FlashRead(unsigned long ulOff) {
	/* Step 1 Return the value at word offset ulOff */
	return BASE_ADDR [ulOff];
}
/*******************************************************************************
 Function: void FlashPause( unsigned int uMicroSeconds )
 Arguments: uMicroSeconds: length of the pause in microseconds
 Returns: none
 Description: This routine returns after uMicroSeconds have elapsed. It is used
 in several parts of the code to generate a pause required for correct
 operation of the flash part.
 The routine here works by counting. The user may already have a more suitable
 routine for timing which can be used.
 Pseudo Code:
 Step 1: Compute count size for pause.
 Step 2: Count to the required size.
 *****************************************************************/
static void FlashPause(unsigned int uMicroSeconds) {
	volatile unsigned long ulCountSize;
	/* Step 1: Compute the count size */
	ulCountSize = (unsigned long) uMicroSeconds * COUNTS_PER_MICROSECOND;
	/* Step 2: Count to the required size */
	while (ulCountSize > 0) /* Test to see if finished */
		ulCountSize--; /* and count down */
}
/*******************************************************************************
 Function: void FlashReadReset( void )
 Arguments: none
 Return Value: none
 Description: This function places the flash in the Read Array mode described
 in the Data Sheet. In this mode the flash can be read as normal memory.
 All of the other functions leave the flash in the Read Array mode so this is
 not strictly necessary. It is provided for completeness.
 Note: A wait of 10us is required if the command is called during a program or
 erase instruction. This is included here to guarantee operation. The
 functions in the data sheet call this function if they suspect an error
 during programming or erasing so that the 10us pause is included. Otherwise
 they use the single instruction technique for increased speed.
 Pseudo Code:
 Step 1: write command sequence (see Instructions Table of the Data Sheet)
 Step 2: wait 10us or 20us (depending on device)
 *******************************************************************************/
void FlashReadReset(void) {
	/* Step 1: write command sequence */
	FlashWrite(0x0AAAL, 0xAA); /* 1st Cycle */
	FlashWrite(0x0555L, 0x55); /* 2nd Cycle */
	FlashWrite(0x0AAAL, 0xF0); /* 3rd Cycle */
	/* Step 2: wait 10us */
	FlashPause(10);
}

/*******************************************************************************
 Function: int FlashAutoSelect( int iFunc )
 Arguments: iFunc should be set to either the Read Signature values or to the
 block number. The header file defines the values for reading the Signature.
 Note: the first block is Block 0
 Return Value: When iFunc is >= 0 the function returns FLASH_BLOCK_PROTECTED
 (01h) if the block is protected and FLASH_BLOCK_UNPROTECTED (00h) if it is
 unprotected. See the AUTO SELECT INSTRUCTION in the Data Sheet for further
 instructions.
 When iFunc is FLASH_READ_MANUFACTURER (-2) the function returns the
 manufacturer’s code. The Manufacturer code for ST is 20h.
 When iFunc is FLASH_READ_DEVICE_CODE (-1) the function returns the Device
 Code. The device codes for the parts are:
 M29F800T ECh
 M29F800B 58h
 M29W800T D7h
 M29W800B 5Bh
 When iFunc is invalid the function returns FLASH_BLOCK_INVALID (-5)
 Description: This function can be used to read the electronic signature of the
 device, the manufacturer code or the protection level of a block.
 Pseudo Code:
 Step 1: Send the Auto Select Instruction to the device
 Step 2: Read the required function from the device.
 Step 3: Return the device to Read Array mode.
 *******************************************************************************/
uint8_t FlashAutoSelect(int8_t iFunc) {
	int8_t iRetVal; /* Holds the return value */
//	/* Step 1: Send the Read Electronic Signature instruction */
	FlashWrite(0x0AAAL, 0xAA); /* 1st Cycle */
	FlashWrite(0x0555L, 0x55); /* 2nd Cycle */
	FlashWrite(0x0AAAL, 0x90); /* 3rd Cycle */
//	/* Step 2: Read the required function */
	if (iFunc == FLASH_READ_MANUFACTURER) {
		/* A0 = A1 = A6 = 0 */
		iRetVal = (uint8_t) FlashRead(0x0000L);
	} else if (iFunc == FLASH_READ_DEVICE_CODE) {
		/* A0 = 1, A1 = A6 = 0, remember A-1 */
		iRetVal = (uint8_t) FlashRead(0x0002L);
	} else if ((iFunc >= 0) && (iFunc < NUM_BLOCKS)) {
		/* A0 = A6 = 0, A1 = 1, remember A-1 */
		iRetVal = (uint8_t) FlashRead(BlockOffset[iFunc] + 0x0004L);
	} else {
		iRetVal = FLASH_BLOCK_INVALID;
	}
//	/* Step 3: Return to Read Array mode */
	FlashWrite(0x0000L, 0xF0); /* Use single instruction cycle method */
	return iRetVal;
}
/*******************************************************************************
 Function: int FlashBlockErase( unsigned char ucNumBlocks,
 unsigned char ucBlock[] )
 Arguments: ucNumBlocks holds the number of blocks in the array ucBlock
 ucBlock is an array containing the blocks to be erased.
 Return Value: The function returns the following conditions:
 FLASH_SUCCESS (-1)
 FLASH_POLL_FAIL (-2)
 FLASH_TOO_MANY_BLOCKS (-3)
 FLASH_MPU_TOO_SLOW (-4)
 FLASH_WRONG_TYPE (-8)
 Number of the first protected or invalid block
 The user’s array, ucBlock[] is used to report errors on the specified
 blocks. If a time-out occurs because the MPU is too slow then the blocks
 in ucBlocks which are not erased are overwritten with FLASH_BLOCK_NOT_ERASED
 (FFh) and the function returns FLASH_MPU_TOO_SLOW.
 If an error occurs during the erasing of the blocks the blocks in
 ucBlocks which have failed the erase are set to FLASH_BLOCK_ERASE_FAILURE
 (FEh) and the function returns FLASH_POLL_FAIL.
 If both errors occur then the function will set the ucBlock array for
 each type of error (i.e. either to FLASH_BLOCK_NOT_ERASED or to
 FLASH_BLOCK_ERASE_FAILURE). It will return FLASH_POLL_FAIL even though
 the FLASH_MPU_TOO_SLOW has also occured.
 Description: This function erases up to ucNumBlocks in the flash. The blocks
 can be listed in any order. The function does not return until the blocks are
 erased. If any blocks are protected or invalid none of the blocks are erased.
 During the Erase Cycle the Data Polling Flowchart of the Data Sheet is
 followed. The toggle bit, DQ6, is not used. For an erase cycle the data on DQ7
 will be ’0’ during the erase and ’1’ on completion.
 Pseudo Code:
 Step 1: Check for correct flash type
 Step 2: Check for protected or invalid blocks
 Step 3: Write Block Erase command
 Step 4: Check for time-out blocks
 Step 5: Wait for the timer bit to be set
 Step 6: Perform Data Polling until P/E.C. has completed
 Step 7: Return to Read Array mode
 *******************************************************************************/
int FlashBlockErase(unsigned char ucNumBlocks, unsigned char ucBlock[]) {
	unsigned char ucCurBlock; /* Range Variable to track current block */
	int iRetVal = FLASH_SUCCESS; /* Holds return value: optimistic initially! */
	unsigned char FirstRead, SecondRead; /* used to check toggle bit DQ2 */
	/* Step 1: Check for correct flash type */
	if (!(FlashAutoSelect(FLASH_READ_MANUFACTURER) == MANUFACTURER)
	|| !(FlashAutoSelect( FLASH_READ_DEVICE_CODE ) == EXPECTED_DEVICE )) {
		return FLASH_WRONG_TYPE;
	}
	/* Step 2: Check for protected or invalid blocks. */
	if (ucNumBlocks > NUM_BLOCKS) /* Check specified blocks <= NUM_BLOCKS */
		return FLASH_TOO_MANY_BLOCKS;
	for (ucCurBlock = 0; ucCurBlock < ucNumBlocks; ucCurBlock++) {
		/* Use FlashAutoSelect to find protected or invalid blocks */
		if (FlashAutoSelect(
				(int) ucBlock[ucCurBlock]) != FLASH_BLOCK_UNPROTECTED)
			return (int) ucBlock[ucCurBlock]; /* Return protected/invalid blocks */
	}
	/* Step 3: Write Block Erase command */
	FlashWrite(0x0AAAL, 0xAA);
	FlashWrite(0x0555L, 0x55);
	FlashWrite(0x0AAAL, 0x80);
	FlashWrite(0x0AAAL, 0xAA);
	FlashWrite(0x0555L, 0x55);
	/* DSI!: Time critical section. Additional blocks must be added every 80us */
	for (ucCurBlock = 0; ucCurBlock < ucNumBlocks; ucCurBlock++) {
		FlashWrite(BlockOffset[ucBlock[ucCurBlock]], 0x30);
		/* Check for Erase Timeout Period */
		if ((FlashRead(BlockOffset[ucBlock[0]]) & 0x08) == 0x08)
			break; /* Cannot set any more sectors due to timeout */
	}
	/* ENI! */
	/* Step 4: Check for time-out blocks */
	/* if timeout occured then check if last block is erasing or not */
	/* Use DQ2 of status register, toggle implies block is erasing */
	if (ucCurBlock < ucNumBlocks) {
		FirstRead = FlashRead(BlockOffset[ucBlock[ucCurBlock]]) & 0x04;
		SecondRead = FlashRead(BlockOffset[ucBlock[ucCurBlock]]) & 0x04;
		if (FirstRead != SecondRead) {
			ucCurBlock++; /* Point to the next block */
		}
		if (ucCurBlock < ucNumBlocks) {
			/* Indicate that some blocks have been timed out of the erase list */
			iRetVal = FLASH_MPU_TOO_SLOW;
		}
		/* Now specify all other blocks as not being erased */
		while (ucCurBlock < ucNumBlocks) {
			ucBlock[ucCurBlock++] = FLASH_BLOCK_NOT_ERASED;
		}
	}
	/* Step 5: Wait for the timer bit to be set */
	while (1) /* TimeOut!: If, for some reason, the hardware fails then this
	 loop may not exit. Use a timer function to implement a timeout
	 from the loop. */
	{
		if ((FlashRead(BlockOffset[ucBlock[0]]) & 0x08) == 0x08)
			break; /* Break when device starts the erase cycle */
	}
	/* Step 6: Perform data polling until P/E.C. completes, check for errors */
	if (FlashDataPoll(BlockOffset[ucBlock[0]], 0xFF) != FLASH_SUCCESS) {
		for (ucCurBlock = 0; ucCurBlock < ucNumBlocks; ucCurBlock++) {
			if (ucBlock[ucCurBlock] == FLASH_BLOCK_NOT_ERASED)
				break; /* The rest of the blocks have not been erased anyway */
			if (FlashBlockFailedErase(
					ucBlock[ucCurBlock]) == FLASH_BLOCK_FAILED_ERASE) {
				ucBlock[ucCurBlock] = FLASH_BLOCK_ERASE_FAILURE;
			}
		}
		iRetVal = FLASH_POLL_FAIL;
	}
	/* Step 7: Return to Read Array mode */
	FlashWrite(0x0000L, 0xF0); /* Use single instruction cycle method */

	return iRetVal;
}
/*******************************************************************************
 Function: int FlashChipErase( unsigned char *Results )
 Arguments: Results is a pointer to an array where the results will be stored.
 If Results == NULL then no results are stored.
 Otherwise the results are written to the array if an error occurs. The
 array is left unchanged if the function returns FLASH_SUCCESS.
 The errors written to the array are:
 FLASH_BLOCK_ERASED (FDh) if the block erased correctly
 FLASH_BLOCK_ERASE_FAILURE (FEh) if the block failed to erased
 Return Value: On success the function returns FLASH_SUCCESS (-1)
 If a block is protected then the function returns the number of the block.
 If the erase algorithms fails then the function returns FLASH_POLL_FAIL (-2)
 If the wrong type of flash is accessed then the function returns
 FLASH_WRONG_TYPE (-8)
 Description: The function can be used to erase the whole flash chip so long as
 no sectors are protected. If any sectors are protected then nothing is
 erased.
 Pseudo Code:
 Step 1: Check for correct flash type
 Step 2: Check that all sectors are unprotected
 Step 3: Send Chip Erase Command
 Step 4: Perform data polling until P/E.C. has completed.
 Step 5: Check for blocks erased correctly
 Step 6: Return to Read Array mode
 *******************************************************************************/
int FlashChipErase(unsigned char *Results) {
	unsigned char ucCurBlock; /* Used to track the current block in a range */
	int iRetVal; /* Holds the return value */
	/* Step 1: Check for correct flash type */
	if (!(FlashAutoSelect(FLASH_READ_MANUFACTURER) == MANUFACTURER)
	|| !(FlashAutoSelect( FLASH_READ_DEVICE_CODE ) == EXPECTED_DEVICE ) )return FLASH_WRONG_TYPE;
	/* Step 2: Check that all sectors are unprotected */
	for (ucCurBlock = 0; ucCurBlock < NUM_BLOCKS; ucCurBlock++) {
		if (FlashAutoSelect((int8_t) ucCurBlock) != FLASH_BLOCK_UNPROTECTED)
			return (int) ucCurBlock; /* Return the first protected block */
	}
	/* Step 3: Send Chip Erase Command */
	FlashWrite(0x0AAA, 0xAA);
	FlashWrite(0x0555, 0x55);
	FlashWrite(0x0AAA, 0x80);
	FlashWrite(0x0AAA, 0xAA);
	FlashWrite(0x0555, 0x55);
	FlashWrite(0x0AAA, 0x10);
	/* Step 4: Perform data polling until P/E.C. completed */
	iRetVal = FlashDataPoll(0x0000, 0xFF);/* Erasing writes 0xFF to flash */
	/* Step 5: Check for blocks erased correctly */
	if (iRetVal != FLASH_SUCCESS && Results != NULL ) {
		for (ucCurBlock = 0; ucCurBlock < NUM_BLOCKS; ucCurBlock++) {
			if (FlashBlockFailedErase(ucCurBlock) == FLASH_BLOCK_FAILED_ERASE) {
				Results[ucCurBlock] = FLASH_BLOCK_ERASE_FAILURE;
			} else
				Results[ucCurBlock] = FLASH_BLOCK_ERASED;
		}
	}
	/* Step 6: Return to Read Array mode */
	FlashWrite(0x0000, 0xF0); /* Use single instruction cycle method */
	return iRetVal;
}
/*******************************************************************************
 Function: int FlashProgram( unsigned long ulOff, size_t NumBytes,
 void *Array )
 Arguments: ulOff is the byte offset into the flash to be programmed
 NumBytes holds the number of bytes in the array.
 Array is a pointer to the array to be programmed.
 Return Value: On success the function returns FLASH_SUCCESS (-1).
 On failure the function returns FLASH_PROGRAM_FAIL (-6).
 If the address exceeds the address range of the Flash Device the function
 returns FLASH_ADDRESS_OUT_OF_RANGE (-7) and nothing is programmed.
 If the wrong type of flash is accessed then the function returns
 FLASH_WRONG_TYPE (-8).
 Description: This function is used to program an array into the flash. It does
 not erase the flash first and will fail if the block is not erased first.
 This function assumes that none of the flash blocks in the
 address range to be programmed are protected. The user must ensure that none
 of these blocks are protected prior to calling this function. If the user
 attempts to program a protected block, the behaviour of the function is
 unspecified.
 Pseudo Code:
 Step 1: Check that the flash is of the correct type
 Step 2: Check the offset range is valid.
 Step 3: While there is more to be programmed
 Step 4: Program the next byte
 Step 5: Perform data polling until P/E.C. has completed.
 Step 6: Update pointers
 Step 7: End of While Loop
 Step 8: Return to Read Array mode
 *******************************************************************************/
int FlashProgram(unsigned long ulOff, size_t NumBytes, void *Array) {
	unsigned char *ucArrayPointer;/* Use an unsigned char to access the array */
	unsigned long ulLastOff; /* Holds the last offset to be programmed */
	/* Step 1: Check for correct flash type */
	if (!(FlashAutoSelect(FLASH_READ_MANUFACTURER) == MANUFACTURER)
	|| !(FlashAutoSelect( FLASH_READ_DEVICE_CODE ) == EXPECTED_DEVICE ) ) {
		return FLASH_WRONG_TYPE;
	}
	/* Step 2: Check the address range is valid */
	ulLastOff = ulOff + NumBytes - 1;
	if (ulLastOff >= FLASH_SIZE)
		return FLASH_OFFSET_OUT_OF_RANGE;
	/* Step 3: While there is more to be programmed */
	ucArrayPointer = (unsigned char *) Array;
	while (ulOff <= ulLastOff) {
		/* Step 4: Program the next byte */
		FlashWrite(0x0AAAL, 0xAA); /* 1st cycle */
		FlashWrite(0x0555L, 0x55); /* 2nd cycle */
		FlashWrite(0x0AAAL, 0xA0); /* Program command */
		FlashWrite(ulOff, *ucArrayPointer); /* Program value */
		/* Step 5: Perform data polling until P/E.C. has completed. */
		/* See Data Polling Flowchart of the Data Sheet */
		if (FlashDataPoll(ulOff, *ucArrayPointer) == FLASH_POLL_FAIL) {
			FlashReadReset();
			return FLASH_PROGRAM_FAIL;
		}
		/* Step 6: Update pointers */
		ulOff++; /* next byte offset */
		ucArrayPointer++; /* next byte in array */
		/* Step 7: End while loop */
	}
	/* Step 8: Return to Read Array mode */
	FlashWrite(0x0000L, 0xF0); /* Use single instruction cycle method */
	return FLASH_SUCCESS;
}
/*******************************************************************************
 Function: static int FlashDataPoll( unsigned long ulOff,
 unsigned char ucVal )
 Arguments: ulOff should hold a valid offset to be polled. For programming
 this will be the offset of the byte being programmed. For erasing this can
 be any address in the block(s) being erased.
 ucVal should hold the value being programmed. A value of FFh should be used
 when erasing.
 Return Value: The function returns FLASH_SUCCESS if the P/E.C. is successful
 or FLASH_POLL_FAIL if there is a problem.
 Description: The function is used to monitor the P/E.C. during erase or
 program operations. It returns when the P/E.C. has completed. The Data Sheet
 gives a flow chart (Data Polling Flowchart) showing the operation of the
 function.
 Pseudo Code:
 Step 1: Read DQ5 and DQ7 (into a byte)
 Step 2: If DQ7 is the same as Value(bit 7) then return FLASH_SUCCESS
 Step 3: Else if DQ5 is zero then operation is not yet complete, goto 1
 Step 4: Else (DQ5 == 1), Read DQ7
 Step 5: If DQ7 is now the same as Value(bit 7) then return FLASH_SUCCESS
 Step 6: Else return FLASH_POLL_FAIL
 *******************************************************************************/
static int FlashDataPoll(unsigned long ulOff, unsigned char ucVal) {
	unsigned char uc; /* holds value read from valid offset */
	while (1) /* TimeOut!: If, for some reason, the hardware fails then this
	 loop may not exit. Use a timer function to implement a timeout
	 from the loop. */
	{
		/* Step 1: Read DQ5 and DQ7 (into a byte) */
		uc = FlashRead(ulOff); /* Read DQ5, DQ7 at valid addr */
		/* Step 2: If DQ7 is the same as Value(bit 7) then return FLASH_SUCCESS */
		if ((uc & 0x80) == (ucVal & 0x80)) /* DQ7 == DATA */
			return FLASH_SUCCESS;
		/* Step 3: Else if DQ5 is zero then operation is not yet complete */
		if ((uc & 0x20) == 0x00) /* DQ5 == 0 (1 for Erase Error) */
			continue;
		/* Step 4: Else (DQ5 == 1) */
		uc = FlashRead(ulOff); /* Read DQ7 at valid addr */
		/* Step 5: If DQ7 is now the same as Value(bit 7) then
		 return FLASH_SUCCESS */
		if ((uc & 0x80) == (ucVal & 0x80)) /* DQ7 == DATA */
			return FLASH_SUCCESS;
		/* Step 6: Else return FLASH_POLL_FAIL */
		else
			/* DQ7 here means fail */
			return FLASH_POLL_FAIL;
	}
}
/*******************************************************************************
 Function: int FlashBlockFailedErase( unsigned char ucBlock )
 Arguments: ucBlock specifies the block to be checked
 Return Value: FLASH_SUCCESS (-1) if the block erased successfully
 FLASH_BLOCK_FAILED_ERASE (-9) if the block failed to erase
 Description: This function can only be called after an erase operation which
 has failed the FlashDataPoll() function. It must be called before the reset
 is made.
 The function reads bit 2 of the Status Register to determine is the block
 has erased successfully or not. Successfully erased blocks should have DQ2
 set to 1 following the erase. Failed blocks will have DQ2 toggle.
 Pseudo Code:
 Step 1: Read DQ2 in the block twice
 Step 2: If they are both the same then return FLASH_SUCCESS
 Step 3: Else return FLASH_BLOCK_FAILED_ERASE
 *******************************************************************************/
static int FlashBlockFailedErase(unsigned char ucBlock) {
	unsigned char FirstRead, SecondRead; /* Two variables used for clarity,
	 Optimiser will probably not use any */
	/* Step 1: Read block twice */
	FirstRead = FlashRead(BlockOffset[ucBlock]) & 0x04;
	SecondRead = FlashRead(BlockOffset[ucBlock]) & 0x04;
	/* Step 2: If they are the same return FLASH_SUCCESS */
	if (FirstRead == SecondRead)
		return FLASH_SUCCESS;
	/* Step 3: Else return FLASH_BLOCK_FAILED_ERASE */
	return FLASH_BLOCK_FAILED_ERASE;
}
/*******************************************************************************
 Function: char *FlashErrorStr( int ErrNum );
 Arguments: ErrNum is the error number returned from another Flash Routine
 Return Value: A pointer to a string with the error message
 Description: This function is used to generate a text string describing the
 error from the flash. Call with the return value from another flash routine.
 Pseudo Code:
 Step 1: Check the error message range.
 Step 2: Return the correct string.
 *******************************************************************************/
char *FlashErrorStr( int ErrNum )
{
	static char *str[] = {
			"Flash Success",
			"Flash Poll Failure",
			"Flash Too Many Blocks",
			"MPU is too slow to erase all the blocks",
			"Flash Block selected is invalid",
			"Flash Program Failure",
			"Flash Address Out Of Range",
			"Flash is Wrong Type",
			"Flash Block Failed Erase",
			"Flash is Unprotected",
			"Flash is Protected"};
	/* Step 1: Check the error message range */
	ErrNum = -ErrNum; /* All errors are negative: make +ve;*/
	/* Step 1,2 Return the correct string */
	if (ErrNum < 1 || ErrNum > 11) /* Check the range */
		return "Unknown Error\n";
	else
		return str[ErrNum - 1];
}
/*******************************************************************************
List of Errors and Return values, Explanations and Help.
 ********************************************************************************
Return Name: FLASH_SUCCESS
Return Value: -1
Description: This value indicates that the flash command has executed
correctly.
 ********************************************************************************
Error Name: FLASH_POLL_FAIL
Return Value: -2
Description: The P/E.C. algorithm has not managed to complete the command
operation successfully. This may be because the device is damaged.
Solution: Try the command again. If it fail a second time then it is
likely that the device will need to be replaced.
********************************************************************************
Error Name: FLASH_TOO_MANY_BLOCKS
Return Value: -3
Description: The user has chosen to erase more blocks than the device has.
This may be because the array of blocks to erase contains the same block
more than once.
Solutions: Check that the program is trying to erase valid blocks. The device
will only have NUM_BLOCKS blocks (defined at the top of the file). Also check
that the same block has not been added twice or more to the array.
********************************************************************************
Error Name: FLASH_MPU_TOO_SLOW
Return Value: -4
Description: The MPU has not managed to write all of the selected blocks to the
device before the timeout period expired. See BLOCK ERASE (BE) INSTRUCTION
section of the Data Sheet for details.
Solutions: If this occurs occasionally then it may be because an interrupt is
occuring during between writing the blocks to be erased. Seach for "DSI!" in
the code and disable interrupts during the time critical sections.
If this command always occurs then it may be time for a faster
microprocessor, a better optimising C compiler or, worse still, learn
assembly. The immediate solution is to only erase one block at a time.
Disable the test (by #define’ing out the code) and always call the function
with one block at a time.
********************************************************************************
Error Name: FLASH_BLOCK_INVALID
Return Value: -5
Description: A request for an invalid block has been made. Valid blocks number
from 0 to NUM_BLOCKS-1.
Solution: Check that the block is in the valid range.
********************************************************************************
Error Name: FLASH_PROGRAM_FAIL
Return Value: -6
Description: The programmed value has not been programmed correctly.
Solutions: Make sure that the block containing the value was erased before
programming. Try erasing the sector and re-programming the value. If it fails
again then the device may need to be changed.
********************************************************************************
Error Name: FLASH_OFFSET_OUT_OF_RANGE
Return Value: -7
Description: The offset given is out of the range of the device.
Solution: Check the offset range is in the valid range.
********************************************************************************
Error Name: FLASH_WRONG_TYPE
Return Value: -8
Description: The source code has been used to access the wrong type of flash.
Solutions: Use a different flash chip with the target hardware or contact
STMicroelectronics for a different source code library.
********************************************************************************
Error Name: FLASH_BLOCK_FAILED_ERASE
Return Value: -9
Description: The previous erase to this block has not managed to successfully
erase the block.
Solution: Sadly the flash needs replacing.
********************************************************************************
Return Name: FLASH_UNPROTECTED
Return Value: -10
Description: The user has requested to unprotected a flash that is already
unprotected or the user has requested to re-protect a flash that has no
protected sectors. This is just a warning to the user that their operation
did not make any changes and was not necessary.
********************************************************************************
Return Name: FLASH_PROTECTED
Return Value: -11
Description: The user has requested to protect a flash that is already
protected. This is just a warning to the user that their operation did
not make any changes and was not necessary.
*******************************************************************************/
