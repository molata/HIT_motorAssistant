/******************************************************************************
* DISCLAIMER

* This software is supplied by Renesas Technology Corp. and is only 
* intended for use with Renesas products. No other uses are authorized.

* This software is owned by Renesas Technology Corp. and is protected under 
* all applicable laws, including copyright laws.

* THIS SOFTWARE IS PROVIDED "AS IS" AND RENESAS MAKES NO WARRANTIES
* REGARDING THIS SOFTWARE, WHETHER EXPRESS, IMPLIED OR STATUTORY, 
* INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
* PARTICULAR PURPOSE AND NON-INFRINGEMENT.  ALL SUCH WARRANTIES ARE EXPRESSLY 
* DISCLAIMED.

* TO THE MAXIMUM EXTENT PERMITTED NOT PROHIBITED BY LAW, NEITHER RENESAS 
* TECHNOLOGY CORP. NOR ANY OF ITS AFFILIATED COMPANIES SHALL BE LIABLE 
* FOR ANY DIRECT, INDIRECT, SPECIAL, INCIDENTAL OR CONSEQUENTIAL DAMAGES 
* FOR ANY REASON RELATED TO THIS SOFTWARE, EVEN IF RENESAS OR ITS 
* AFFILIATES HAVE BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

* Renesas reserves the right, without notice, to make changes to this 
* software and to discontinue the availability of this software.  
* By using this software, you agree to the additional terms and 
* conditions found by accessing the following link:
* http://www.renesas.com/disclaimer
******************************************************************************
* Copyright (C) 2008. Renesas Technology Corp., All Rights Reserved.
*******************************************************************************	
* File Name    : Flash_API_RX600.c
* Version      : 1.30
* Description  : Flash programming for the RX600 Group
*
*
******************************************************************************
* History : MM.DD.YYYY Version Description
*         : 12.21.2009 1.00    First Release
*         : 01.13.2010 1.10    Made function names and variables RAPI compliant
*         : 02.11.2010 1.20    Fixed other RAPI issues and fixed I flag issue
*         : 04.29.2010 1.30    Added support for RX621/N Group. Moved most
*                              device specific data to header file.
*         : 05.26.2010 1.40    Added support for RX62T Group
******************************************************************************/


/******************************************************************************
Includes   <System Includes> , "Project Includes"
******************************************************************************/
/* Intrinsic functions of MCU */
#include <machine.h>
/* Function prototypes and device specific info needed for Flash API */
#include "Flash_API_RX600.h"
/* Device specific SFR definitions */
#if defined(RX610)
#include "iodefine_RX610.h"
#elif defined(RX62N)
#include "iodefine_RX62N.h"
#elif defined(RX621)
#include "iodefine_RX62N.h"
#elif defined(RX62T)
#include "iodefine_RX62N.h"
#else
#error "!!! You must specify your device in Flash_API_RX600.h first !!!"
#endif

/******************************************************************************
Typedef definitions
******************************************************************************/

/******************************************************************************
Macro definitions
******************************************************************************/
#define READ_MODE 0
#define ROM_PE_MODE 1
#define FLD_PE_MODE 2

/*  The number of ICLK ticks needed for 35us delay are calculated below */
#define		WAIT_tRESW		(35*ICLK_FREQUENCY)			        
/*  The number of ICLK ticks needed for 10us delay are calculated below */
#define		WAIT_t10USEC	(10*ICLK_FREQUENCY)		

/******************************************************************************
Imported global variables and functions (from other files)
******************************************************************************/

/******************************************************************************
Exported global variables and functions (to be accessed by other files)
******************************************************************************/

/******************************************************************************
Private global variables and functions
******************************************************************************/
/* Valid values are 'READ_MODE','ROM_PE_MODE' or 'FLD_PE_MODE' */
static uint8_t current_mode;	     
/* Stores whether interrupts were enabled when flash operation was called */
static uint8_t psw_i_bit;		     

uint8_t _Flash_Init(void);
uint8_t _Enter_PE_Mode( uint32_t flash_addr, uint16_t bytes );
void    _Exit_PE_Mode(void);
uint8_t _ROM_Write(uint32_t address, uint32_t data);
uint8_t _DataFlash_Write(uint32_t address, uint32_t data, uint8_t size);
void    _DataFlash_Status_Clr(void);
uint8_t _Notify_Peripheral_Clock(volatile uint8_t * flash_addr);
void    _Flash_Reset(void);
    
/* Signals whether FCU firmware has been transferred to the FCU RAM */
uint8_t fcu_transfer_complete = 0;    /* 0=No, 1=Yes */

/* This macro was defined in the Flash_API_RXxxx.h file and will 
   create an array of flash addresses called block_addresses.  It is
   used for getting the write addresses of flash blocks */
DEFINE_ADDR_LOOKUP;

/******************************************************************************
* Function Name: _Flash_Init
* Description  : Initializes the FCU peripheral block.
	NOTE: This function does not have to execute from in RAM.
* Arguments    : none
* Return Value : 
		0x00 = Operation Successful
        0x01 = Operation Failed
******************************************************************************/
uint8_t _Flash_Init(void)
{
	uint32_t 	* src;
	uint32_t 	* dst;
	uint16_t	i;
	
	/* Disable FCU interrupts */
	FLASH.FRDYIE.BIT.FRDYIE = 0;
	
    /* Disable FCU interrupts in FCU block */
	FLASH.FAEINT.BIT.ROMAEIE = 0;
	FLASH.FAEINT.BIT.CMDLKIE = 0;
	FLASH.FAEINT.BIT.DFLAEIE = 0;
	FLASH.FAEINT.BIT.DFLRPEIE = 0;
	FLASH.FAEINT.BIT.DFLWPEIE = 0;
	
    /* Disable FCU interrupts in ICU */
    
    /* Disable flash interface error (FIFERR) */
    IPR(FCU, FIFERR) = 0;
    IEN(FCU, FIFERR) = 0;
    /* Disable flash ready interrupt (FRDYI) */
    IPR(FCU, FRDYI) = 0;
    IEN(FCU, FRDYI) = 0;

	/*----------------------------------*/
	/* Transfer Firmware to the FCU RAM */
	/*----------------------------------*/
	/* To use FCU commands, the FCU firmware must be stored in the FCU RAM. */
	/* Before writing data to the FCU RAM, clear FENTRYR to H'0000 to stop the FCU. */
	if(FLASH.FENTRYR.WORD != 0x0000)
	{
		/* Disable the FCU form accepting commands */
		/* Clear both the FENTRY0(ROM) and FENTRYD(Data Flash) bits */
        FLASH.FENTRYR.WORD = 0xAA00;					// Clear FENTRY0 & FENTRYD to 0
    }
	
	/* Enable the FCU RAM */
	FLASH.FCURAME.WORD = 0xC401;
    

	/* Copies the FCU firmware to the FCU RAM.
	   Source: H'FEFFE000 to H'FF00000 (FCU firmware area)
	   Destination: H'007F8000 to H'007FA000 (FCU RAM area) */
	src = (uint32_t *)FCU_PRG_TOP;
	dst = (uint32_t *)FCU_RAM_TOP;
	for( i=0; i<(FCU_RAM_SIZE/4); i++)
	{
		*dst = *src;
		src++;
		dst++;
	}

	fcu_transfer_complete = 1;

	return 0;
}

/******************************************************************************
* Function Name: _DataFlash_Status_Clr
* Description  : Clear the status of the Data Flash operation.
	NOTE: This function does not have to execute from in RAM.
* Arguments    : none
* Return Value : none
******************************************************************************/
void _DataFlash_Status_Clr(void)
{
    volatile uint8_t *ptrb;

    ptrb = (uint8_t *)(DF_ADDRESS);

    /* Clear ILGLERR */
	if(FLASH.FSTATR0.BIT.ILGLERR == 1)	
    {
		if(FLASH.FASTAT.BYTE != 0x10)
        {
			FLASH.FASTAT.BYTE = 0x10;
		}
	}
    
    *ptrb = 0x50;
}

/******************************************************************************
* Function Name: _DataFlash_Write
* Description  : Write either 8-bytes of 128-bytes to Data Flash area.
	NOTE: This function does not have to execute from in RAM.
* Arguments    :
	address
		The address (in the Data Flash programming area) to write the data to
	data
		The address of the data to write
	size
		The size of the data to write. Must be set to either '8' or '128'
* Return Value : 
		0x00 = Operation Successful
        0x01 = Operation Failed
******************************************************************************/
uint8_t _DataFlash_Write(uint32_t address, uint32_t data, uint8_t size)
{
    int32_t wait_cnt;
    uint8_t n = 0;

    /* Size must be a supported DF programming size */
    if((size == 128) || (size == 8))
	{
        size = size >> 1;
        *(volatile uint8_t *)DF_ADDRESS = 0xE8;
        /* 0x40 for 128 bytes or 0x04 for 8 bytes */
        *(volatile uint8_t *)DF_ADDRESS = size;	 
		while(n++ < size)
		{
            *(volatile uint16_t *)address = *(uint16_t *)data;
			data += 2;
        }

    	/* Write the final FCU command for programming */
		*(volatile uint8_t *)(DF_ADDRESS) = 0xD0;


        /* Now we wait for FCU to indicate flash is done, or we timeout */
        wait_cnt = WAIT_MAX_DF_WRITE;

    	while(FLASH.FSTATR0.BIT.FRDY == 0)
        {	
    		wait_cnt--;
		
    		if(wait_cnt == 0)
            {            
    		    /* At this point we have passed the maximum time for writing
                   a block and should reset the FCU and report an error has
                   occurred */
    			_Flash_Reset();

    			return 1;
    		}
	
    	}

        /* Check for illegal command or programming errors */
    	if((FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.PRGERR  == 1)) 
        {        
    		return 1;
    	}                
    }
    else
	{
        /* Invalid 'size' argument */
        return(1);
    }
	return 0;
}

/******************************************************************************
* Function Name: R_FlashDataAreaBlankCheck
* Description  : 
	NOTE: This function does not have to execute from in RAM.
* Arguments    :
	address
		The address to check if is blank.
		If the parameter 'size'=='BLANK_CHECK_8_BYTE', this should be set to an
		8-byte address boundary.
		If the parameter 'size'=='BLANK_CHECK_ENTIRE_BLOCK', this should be set
		to a defined Data Block Number ('BLOCK_DB0', 'BLOCK_DB1', etc...)
	size
		This specifies if you are checking an 8-byte location, or an entire
		block. You must set this to either 'BLANK_CHECK_8_BYTE' or
		'BLANK_CHECK_ENTIRE_BLOCK'.
* Return Value : 
		0x00 = Blank
        0x01 = Not Blank
        0x02 = Operation Failed
******************************************************************************/
uint8_t R_FlashDataAreaBlankCheck(uint32_t address, uint8_t size)
{
    volatile uint8_t *ptrb;
	uint8_t result;

	current_mode = FLD_PE_MODE;
	/* Enter Program/Erase Normal Mode for Data Flash Operations */
	if( _Enter_PE_Mode( address, 0 ) != 0)
	{
		current_mode = READ_MODE;
		return 2;
	}

	/* Set  bit FRDMD (bit 4) in FMODR to 1 */
	FLASH.FMODR.BIT.FRDMD = 1;

    if(size == BLANK_CHECK_8_BYTE)
	{
        /* The beginning of this block that contains the 8-bytes */
		ptrb = (uint8_t *)(address & DF_MASK);	
		/* Check if the next 8 bytes are blank
		   Bits BCADR to the address of the 8byte location to check (only bits A3-A12)
		   Set bit BCSIZE in EEPBCCNT to 0. */
        FLASH.DFLBCCNT.WORD = (uint16_t)(address & (DF_BLOCK_SIZE-8));
    }
	else	/* size == BLANK_CHECK_ENTIRE_BLOCK */
	{
		ptrb = (uint8_t *)block_addresses[address];
		/* Check if the entire block is blank
		   Set bit BCSIZE in EEPBCCNT to 1.
		   Bits BCADR are not used, set to '0'. */
		FLASH.DFLBCCNT.WORD = 0x0001;
    }

    /* ---- Send the FCU command ---- */
    *ptrb = 0x71;
    *ptrb = 0xD0;

    /* Check 'FRDY' bit */
    while( FLASH.FSTATR0.BIT.FRDY == 0 )
	{
        /* ---- Wait till ready ---- */
    }

	/* Reset the FRDMD bit back to 0 */
	FLASH.FMODR.BIT.FRDMD = 0x00;

	/* Check if the 'ILGERR' was set during the command */
	if(FLASH.FSTATR0.BIT.ILGLERR == 1)	/* Check 'ILGERR' bit */
	{
		/* ERROR */
		_Exit_PE_Mode( );
		current_mode = READ_MODE;
		return(2);
    }

	/* (Read the 'BCST' bit (bit 0) in the 'DFLBCSTAT' register
       0=blank, 1=not blank */
	result = FLASH.DFLBCSTAT.BIT.BCST;	

	_Exit_PE_Mode( );
	current_mode = READ_MODE;
	return( result );
}


/******************************************************************************
* Function Name: R_FlashDataAreaAccess
* Description  : 
	This function is used to allow read and program permissions to the Data
	Flash areas.
	NOTE: This function does not have to execute from in RAM.
* Arguments    : 
	read_en_mask
		Bitmasked value. Bits 0-3 represents each Data Blocks 0-3 (respectively).
		  '0'=no Read access. '1'=Allows Read by CPU
	write_en_mask
		Bitmasked value. Bits 0-3 represents each Data Blocks 0-3 (respectively).
		  '0'=no Erase/Write access. '1'=Allows Erase/Write by FCU
* Return Value : none
******************************************************************************/
void R_FlashDataAreaAccess(uint16_t read_en_mask, uint16_t write_en_mask)
{
    #if   defined(RX610)
    
    /* Set Read access for the Data Flash blocks */
	FLASH.DFLRE.WORD = 0x2D00 | (read_en_mask & 0x00FF);

	/* Set Erase/Program access for the Data Flash blocks */
	FLASH.DFLWE.WORD = 0x1E00 | (write_en_mask & 0x00FF);
        
    #elif defined(RX621) || defined(RX62T) || defined(RX62N)
	
    /* Set Read access for the Data Flash blocks DB0-DB7 */
	FLASH.DFLRE0.WORD = 0x2D00 | (read_en_mask & 0x00FF);
    
    /* Set Read access for the Data Flash blocks DB8-DB15 */
	FLASH.DFLRE1.WORD = 0xD200 | ((read_en_mask >> 8) & 0x00FF);

	/* Set Erase/Program access for the Data Flash blocks DB0-DB7 */
	FLASH.DFLWE0.WORD = 0x1E00 |  (write_en_mask & 0x00FF);

    /* Set Erase/Program access for the Data Flash blocks DB8-DB15 */
	FLASH.DFLWE1.WORD = 0xE100 |  ((write_en_mask >> 8) & 0x00FF);
    
    #else    
        
    #error "!!! You must specify your device in Flash_API_RX6xx.h first !!!"
    
    #endif
}

#if ENABLE_ROM_PROGRAMMING
#pragma section FRAM
#endif

/******************************************************************************
* Function Name: _ROM_Write
* Description  : Write 256 bytes to ROM Area Flash.
	NOTE: This function MUST execute from in RAM.
* Arguments    : none
* Return Value : 
		0x00 = Operation Successful
        0x01 = Operation Failed
******************************************************************************/
uint8_t _ROM_Write(uint32_t address, uint32_t data)
{
	uint8_t i;
    int32_t wait_cnt;

	/* Write the FCU Program command */
    *(volatile uint8_t *)address = 0xE8;	
    *(volatile uint8_t *)address = 0x80;

    /* Write 256 bytes into flash, 16-bits at a time */
	for(i = 0; i < 128; i++)
	{
        *(volatile uint16_t *)address = *(uint16_t *)data;
		data += 2;
    }

    /* Write the final FCU command for programming */
	*(volatile uint8_t *)address = 0xD0;

    /* Now we wait for FCU to indicate flash is done, or we timeout */
    wait_cnt = WAIT_MAX_ROM_WRITE;

    while(FLASH.FSTATR0.BIT.FRDY == 0)
    {
    	wait_cnt--;

    	if(wait_cnt == 0)
        {    
    	    /* At this point we have passed the maximum time for writing
               a block and should reset the FCU and report an error has
               occurred */
    		_Flash_Reset();

    		return 1;
    	}
    }

    /* Check for illegal command or programming errors */
    if((FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.PRGERR  == 1)) 
    {        
    	return 1;
    }                
    
    return (0);
}


/******************************************************************************
* Function Name: _Enter_PE_Mode
* Description  : 
	NOTE: This function MUST execute from in RAM for 'ROM Area' programming, but
	      if you are ONLY doing Data Flash programming, this function can reside and
		  execute in Flash.
* Arguments    : 	bytes
		The number of bytes you are writing (if you are writing).  This argument
        is used to determine if you are crossing the 1MB boundary in ROM.  If you
        do cross this boundary then FENTRY0 and FENTRY1 both need to be set.
* Return Value : 
		0x00 = Operation Successful
        0x01 = Operation Failed
******************************************************************************/
uint8_t _Enter_PE_Mode( uint32_t flash_addr, uint16_t bytes )
{

	/* FENTRYR must be 0x0000 before bit FENTRY0 or FENTRYD can be set to 1 */
    FLASH.FENTRYR.WORD = 0xAA00;

    nop();
    nop();

	if( current_mode == ROM_PE_MODE) 
    {             
   		/* ROM */		
        /* See if interrupts were already disabled, so as not to enable them when leaving */
	    if((get_psw() & I_FLAG) == 0)
        {
            psw_i_bit = 0;
        } 
        else
        {
            psw_i_bit = 1;
        }
         
        /* Disable Interrupts because the interrupt vector table may not be accessible */
        set_psw(get_psw() & (~I_FLAG));
          
        if(flash_addr >= 0x00F00000)
        {
            /* Enter ROM PE mode for addresses 0xFFF00000 - 0xFFFFFFFF */
            FLASH.FENTRYR.WORD = 0xAA01;
        } 
        else
        {            
            /* Enter ROM PE mode for addresses 0xFFE00000 - 0xFFEFFFFF */
            FLASH.FENTRYR.WORD = 0xAA02;
        }
                
    } else if( current_mode == FLD_PE_MODE )
	{
		/* Data Flash */
        
        /* Set FENTRYD bit(Bit 7) and FKEY (B8-15 = 0xAA) */
		FLASH.FENTRYR.WORD = 0xAA80;	

		/*  First clear the status of the FCU before doing Data Flash programming.
    		This is to clear out any previous errors that may have occured.
    		For example, if you attempt to read the Data Flash area 
    		before you make it readable using R_FlashDataAreaAccess(). */
		_DataFlash_Status_Clr();
        
    } else
	{
		/* Invalid value of 'current_mode' */
		return 1;
	}

    /* Enable Write/Erase of ROM/Data Flash */
    FLASH.FWEPROR.BYTE = 0x01;


	/* Check FCU error */
	if((FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.ERSERR  == 1) || (FLASH.FSTATR0.BIT.PRGERR  == 1) || (FLASH.FSTATR1.BIT.FCUERR == 1)) 
	{
		/* Enable Interrupts (ROM PE Mode only) */
	    set_psw(get_psw() | (I_FLAG));

        /* ERROR */
		return 1;	 
	}

	/* Successful */
	return 0;
}

/******************************************************************************
* Function Name: _Exit_PE_Mode
* Description  : 
	NOTE: This function MUST execute from in RAM for 'ROM Area' programming, but
	      if you are ONLY doing Data Flash programming, this function can reside and
		  execute in Flash.
* Arguments    : none
* Return Value : none
******************************************************************************/
void _Exit_PE_Mode( void )
{
	int32_t		wait_cnt;
    volatile uint8_t *pAddr;
	 
	wait_cnt = WAIT_MAX_ERASE;

	/* Check FSTATR0.FRDY and execute FCU reset */
	while(FLASH.FSTATR0.BIT.FRDY == 0)
    {

		/* Count down max erase time */
        wait_cnt--;							

		if(wait_cnt == 0)
        {
			/* Execute FCU RESET */
            _Flash_Reset();			
			
			break;
		}
	}
	
	/* Check FSTATR0 and execute a status register clear command */
	if((FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.ERSERR  == 1) || (FLASH.FSTATR0.BIT.PRGERR  == 1)) 
    {
		
		/* Clear ILGLERR */
        if(FLASH.FSTATR0.BIT.ILGLERR == 1)
        {
			if(FLASH.FASTAT.BYTE != 0x10)
            {
				FLASH.FASTAT.BYTE = 0x10;
			}
		}
		
        /* Issue a status register clear command to clear all error bits */
		if( current_mode == ROM_PE_MODE )
		{
			/* ROM */
			pAddr = (uint8_t *)ROM_PE_ADDR;
			*pAddr = 0x50;
		}
		else // ( current_mode == FLD_PE_MODE )
		{
			/* Data Flash */
			pAddr = (uint8_t *)DF_ADDRESS;
			*pAddr = 0x50;
		}
	
	}

	/* Enter ROM Read mode */
    FLASH.FENTRYR.WORD = 0xAA00;				
    
    /* Flash write/erase disabled */
	FLASH.FWEPROR.BYTE = 0x02;


	/* Restore the imask value (ROM PE Mode only) */
	if( current_mode == ROM_PE_MODE )
	{
		/* Enable Interrupts if I flag was set initially (ROM PE Mode only) */
        if(psw_i_bit == 1)
        {
	        set_psw(get_psw() | (I_FLAG));
        }
	}

}


/******************************************************************************
* Function Name: R_FlashErase
* Description  : Erases an entire flash block.
	NOTE: This function MUST execute from in RAM for 'ROM Area' programming, but
	      if you are ONLY doing Data Flash programming, this function can reside and
		  execute in Flash.
* Arguments    : 
	block
		The block number to erase (BLOCK_0, BLOCK_1, etc...)
* Return Value : 
		0x00 = Operation Successful
        0x01 = Operation Failed
******************************************************************************/
uint8_t R_FlashErase (uint8_t block)
{
	volatile uint8_t *pAddr;
	uint8_t result = 0;
    int32_t wait_cnt;

    /* Erase Command Address */
	pAddr = (volatile uint8_t *)(block_addresses[ block ]);

	if( !fcu_transfer_complete )
	{
		result = _Flash_Init();	
		if( result != 0x00 )
		{
			return 1;
		}
	}

	/* Make sure we are not in the middle of another operation */
	if( current_mode != READ_MODE )
	{
        /* Currently in the middle of doing something else */
		return 1;	 
	}

   	/* Do we want to erase a Data Flash block or ROM block? */
	if( block >= BLOCK_DB0 )
	{
		current_mode = FLD_PE_MODE;
	}
	else
	{
		current_mode = ROM_PE_MODE;
	}
    
    /* Enter Program Erase Normal Mode for ROM/Data Flash Operations */
	if( _Enter_PE_Mode((uint32_t)pAddr, 0) != 0)        
	{
		current_mode = READ_MODE;
		return 1;
	}
    
    /* Inform FCU of peripheral clock speed */
    if( _Notify_Peripheral_Clock(pAddr) != 0)
    {
        current_mode = READ_MODE;
		return 1;
    }

	/**********************************/
	/* Cancel the ROM Protect feature */
	/* NOTE: If you are using block locking feature to protect your 
	   data, remove the following line */
	FLASH.FPROTR.WORD = 0x5501;

    /* Send the FCU Command */
    *pAddr = 0x20;
    *pAddr = 0xD0;

    /* Wait until the operation is complete, or timeout if max time
       for a max size block erase is taken */
	/* The operations is complete with the FRDY (bit 7) in register
	   FSTATR goes back high. */
	wait_cnt = WAIT_MAX_ERASE;

	while(FLASH.FSTATR0.BIT.FRDY == 0)
    {
	
		wait_cnt--;
		
		if(wait_cnt == 0){
		
			_Flash_Reset();

			return 1;
		}
	
	}

	/* Check if erase operation was successful by checking 
	   bit 'ERSERR' (bit5) and 'ILGLERR' (bit 6) of register 'FSTATR0' */
	/* Check FCU error */
	if((FLASH.FSTATR0.BIT.ILGLERR == 1) || (FLASH.FSTATR0.BIT.ERSERR  == 1)) 
    {
        result = 1;
    }
 
	/* Leave Program/Erase Mode */
 	_Exit_PE_Mode();

	current_mode = READ_MODE;

	return result;
}

/******************************************************************************
* Function Name: R_FlashWrite
* Description  : Writes bytes into flash.
	NOTE: This function MUST execute from in RAM for 'ROM Area' programming, but
	      if you are ONLY doing Data Flash programming, this function can reside and
		  execute in Flash.
* Arguments    : 
	flash_addr
		Flash address location to write to. This address must be on a 256 byte
		boundary.
	buffer_addr
		Address location of data buffer to write into flash.
	bytes
		The number of bytes to write. You must always pass a multiple of 256
		bytes for programming.
* Return Value : 
		0x00 = Operation Successful
        0x01 = Operation Failed
		0x02 = (ROM AREA PROGRAMMING) Number of bytes is not divisible by 256
		0x03 = (ROM AREA PROGRAMMING) Flash address not on a 256 byte boundary
		0x04 = (Data Flash AREA PROGRAMMING) Number of bytes is not divisible by 8
		0x05 = (Data Flash AREA PROGRAMMING) Flash address not on a 8 byte boundary
        0x06 = Invalid address
        0x07 = (ROM AREA PROGRAMMING) Cannot write over 1MB boundary at 0xFFF00000
******************************************************************************/
uint8_t R_FlashWrite (	uint32_t flash_addr,
							uint32_t buffer_addr,
							uint16_t bytes )
{
	uint8_t result = 0;
	uint8_t num_byte_to_write;
    
    /* Take off upper byte since for programming/erase addresses for ROM are same as read addresses
       except upper byte is masked off to 0's.  Data Flash addresses are not affected. */
    flash_addr &= 0x00FFFFFF;

	/* ROM area or Data Flash area - Check for DF first */  
	if( flash_addr >= DF_ADDRESS && flash_addr < DF_ADDRESS + DF_SIZE )
	{
		/* Check if the number of bytes were passed is a multiple of 8 */
		if( bytes & 0x07 )
		{
			return 4;	// ERROR!! You must always pass a multiple of 8.
		}
		/* Check for an address on a 8 byte page boundary */
		if( (uint16_t)flash_addr & 0x07)
		{
			return 5;	// ERROR!! You must always pass an flash address on a 8 byte boundary
		}
	}
    /* Check for ROM area */
	else if( flash_addr >= ROM_PE_ADDR && flash_addr < ROM_PE_ADDR + ROM_SIZE)
	{
		/* Check if the number of bytes were passed is a multiple of 256 */
		if( bytes & 0xFF )
		{
			return 2;	// ERROR!! You must always pass a multiple of 256.
		}

		/* Check for an address on a 256 byte page */
		if( (uint16_t)flash_addr & 0xFF)
		{
			return 3;	// ERROR!! You must always pass an flash address on a 256 byte boundary
		}
        
        /* Check for passing over 1MB boundary at 0xFFF00000 */
        if( (flash_addr < (0x00F00000)) && (flash_addr + bytes) > 0x00F00000)
        {
            return 7;   // ERROR!! You cannot write over 1MB boundary at 0xFFF00000.  You must split up the write.
        } 
	} else 
    {
        return 6;   // ERROR!! Invalid flash address
    }

    /* If FCU firmware has already been transferred to FCU RAM, no need to do it again */
	if( !fcu_transfer_complete )
	{
        /* If FCU firmware has not been transferred, then transfer from FCU ROM to FCU RAM */
		result = _Flash_Init();	
	}

	/* Make sure we are not in the middle of another operation */
	if( current_mode != READ_MODE )
	{
		return 1;	// Currently in the middle of doing something else
	}

	/* Do we want to program a DF area or ROM area? */
	if( flash_addr < block_addresses[ROM_NUM_BLOCKS-1] )
	{
		current_mode = FLD_PE_MODE;
	}
	else
	{
		current_mode = ROM_PE_MODE;

	}

	/* Enter Program Erase Normal Mode for ROM Operations */
	if( _Enter_PE_Mode( flash_addr, bytes ) != 0)
	{
		current_mode = READ_MODE;
		return 1;
	}

	/**********************************/
	/* Cancel the ROM Protect feature */
	/* NOTE: If you are using block locking feature to protect your 
	   data, remove the following line */
	FLASH.FPROTR.WORD = 0x5501;

	if( current_mode == FLD_PE_MODE )
	{
		/* Can write either 8 or 128 bytes at a time (128 bytes is faster though)
		   But, we can only write 128 bytes if we are on a 128 byte boundary. */
        while( bytes )
		{
			if( (bytes >= 128) && (((uint8_t)flash_addr & 0x7F) == 0x00))
			{
				num_byte_to_write = 128;
			}
			else
			{
				num_byte_to_write = 8;
			}

			/* Call the Programming function */
			result = _DataFlash_Write(flash_addr, buffer_addr,num_byte_to_write);

			/* Check for errors */
			if( result != 0x00 )
			{
				break;	// Stop programming 
			}

			/* Now do the next set of 128 or 8 bytes */
			flash_addr += num_byte_to_write;
			buffer_addr += num_byte_to_write;
			bytes -= num_byte_to_write;
		}

	}
	else /* current_mode == ROM_PE_MODE */
	{
		while( bytes )
		{
			/* Call the Programming function */
			result = _ROM_Write(flash_addr, buffer_addr);

			/* Check for errors */
			if( result != 0x00 )
			{
                /* Stop programming */
				break;	
			}

			/* Now do the next set of 256 bytes */
			flash_addr += 256;
			buffer_addr += 256;
			bytes -= 256;
		}
		
	}

	/* Leave Program/Erase Mode */
	_Exit_PE_Mode();

	current_mode = READ_MODE;

	return result;
}

/******************************************************************************
* Function Name: _Notify_Peripheral_Clock
* Description  : Notifies FCU or Peripheral Clock (PCLK)
	NOTE: This function MUST execute from in RAM for 'ROM Area' programming, but
	      if you are ONLY doing Data Flash programming, this function can reside and
		  execute in Flash.
* Arguments    : 
	flash_addr
		Flash address you will be erasing or writing to
* Return Value : 
		0x00 = Operation Successful
        0x01 = Operation Failed
******************************************************************************/
uint8_t _Notify_Peripheral_Clock(volatile uint8_t * flash_addr)
{
	int32_t wait_cnt;

	/* Notify Peripheral Clock(PCK) */
    /* Set frequency of PCK */
	FLASH.PCKAR.WORD = PCLK_FREQUENCY;    		

	/* Execute Peripheral Clock Notification Command */
    *flash_addr = 0xE9;			

	*flash_addr = 0x03;

	*(uint16_t far *)flash_addr = 0x0F0F;

	*(uint16_t far *)flash_addr = 0x0F0F;

	*(uint16_t far *)flash_addr = 0x0F0F;

	*flash_addr = 0xD0;


	/* Check FCU error */
	wait_cnt = WAIT_t10USEC;

    /* Check FRDY */
	while(FLASH.FSTATR0.BIT.FRDY == 0)
    {
		/* Count down 10 usec */
        wait_cnt--;								
		
		if(wait_cnt == 0)
        {
			/* Execute FCU RESET */
            _Flash_Reset();				        

			return 1;
		}
	}

    /* Check ILGLERR */
	if(FLASH.FSTATR0.BIT.ILGLERR == 1)
    {
        /* Error */
		return 1;
	}

    /* Success */
	return 0;
}

/******************************************************************************
* Function Name: _Flash_Reset
* Description  : Reset the FCU.
*	NOTE: This function MUST execute from in RAM for 'ROM Area' programming, but
*	      if you are ONLY doing Data Flash programming, this function can reside and
*		  execute in Flash.
* Arguments    : none
* Return Value : none
******************************************************************************/
void _Flash_Reset(void)
{
	int32_t		wait_cnt;

    /* Reset the FCU */
	FLASH.FRESETR.WORD = 0xCC01;

    /* Give FCU time to reset */
	wait_cnt = WAIT_tRESW;

	while(wait_cnt != 0)
    {
		wait_cnt--;
	}

    /* FCU is not reset anymore */
	FLASH.FRESETR.WORD = 0xCC00;

}





