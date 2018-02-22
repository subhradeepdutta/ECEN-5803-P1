/**----------------------------------------------------------------------------
             \file Monitor.cpp
--                                                                           --
--              ECEN 5003 Mastering Embedded System Architecture             --
--                  Project 1 Module 3                                       --
--                Microcontroller Firmware                                   --
--                      Monitor.cpp                                          --
--                                                                           --
-------------------------------------------------------------------------------
--
--  Designed for:  University of Colorado at Boulder
--               
--                
--  Designed by:  Tim Scherr
--  Revised by:  Tristan Lennertz, Subhradeep Dutta, & Omkar Prabhu 
-- 
-- Version: 2.0
-- Date of current revision:  2016-02 
-- Target Microcontroller: Freescale MKL25ZVMT4 
-- Tools used:  ARM mbed compiler
--              ARM mbed SDK
--              Freescale FRDM-KL25Z Freedom Board
--               
-- 
   Functional Description: See below 
--
--      Copyright (c) 2015 Tim Scherr All rights reserved.
--
*/              

#include <stdio.h>
#include <stdlib.h>

#include "shared.h"

/* Function to print ARM registers */
void printRegs();

/* Function for collecting hex value from terminal */
uint32_t collectHex(); 

/* Function to print a section of memory */
void print_mem(uint8_t * start, uint32_t length);

/* Forward declaration of register access functions */
uint32_t read_gpr_0();
uint32_t read_gpr_1();
uint32_t read_gpr_2();
uint32_t read_gpr_3();
uint32_t read_gpr_4();
uint32_t read_gpr_5();
uint32_t read_gpr_6();
uint32_t read_gpr_7();
uint32_t read_gpr_8();
uint32_t read_gpr_9();
uint32_t read_gpr_10();
uint32_t read_gpr_11();
uint32_t read_gpr_12();
uint32_t read_sp();
uint32_t read_lr();
uint32_t read_pc();

/* Custom integer-to-ascii function for ease of printing */
uint8_t my_itoa(int32_t data, uint8_t * ptr, uint32_t base);

/* Custom ascii-to-integer function for ease of input */
int32_t my_atoi(uint8_t * ptr, uint32_t base); 

/*******************************************************************************
* Set Display Mode Function
* Function determines the correct display mode.  The 3 display modes operate as 
*   follows:
*
*  NORMAL MODE       Outputs only mode and state information changes   
*                     and calculated outputs
*
*  QUIET MODE        No Outputs
*
*  DEBUG MODE        Outputs mode and state information, error counts,
*                    register displays, sensor states, and calculated output
*
*
* There is deliberate delay in switching between modes to allow the RS-232 cable 
* to be plugged into the header without causing problems. 
*******************************************************************************/
void set_display_mode(void)   
{
  UART_direct_msg_put("\r\nSelect Mode");
  UART_direct_msg_put("\r\n Hit NOR - Normal");
  UART_direct_msg_put("\r\n Hit QUI - Quiet");
  UART_direct_msg_put("\r\n Hit DEB - Debug" );
  UART_direct_msg_put("\r\n Hit V - Version#\r\n");
  UART_direct_msg_put("\r\nSelect:  ");
  
}

//*****************************************************************************/
/// \fn void chk_UART_msg(void) 
///
//*****************************************************************************/
void chk_UART_msg(void)    
{
   UCHAR j;
   while( UART_input() )      // becomes true only when a byte has been received
   {                                    // skip if no characters pending
      j = UART_get();                 // get next character

      if( j == '\r' )          // on a enter (return) key press
      {                // complete message (all messages end in carriage return)
        UART_direct_msg_put("\r\n");  
				UART_msg_process();
      }
      else 
      {
         if ((j != 0x02) )         // if not ^B
         {                             // if not command, then   
            UART_put(j);              // echo the character   
         }
         else
         {
           ;
         }
         
         if( j == '\b' ) 
         {                             // backspace editor
            if( msg_buf_idx != 0) 
            {                       // if not 1st character then destructive 
               UART_msg_put(" \b");// backspace
               msg_buf_idx--;
            }
         }
         else if( msg_buf_idx >= MSG_BUF_SIZE )  
         {                                // check message length too large
            UART_msg_put("\r\nToo Long!");
            msg_buf_idx = 0;
         }
         else if ((display_mode == QUIET) && (msg_buf[0] != 0x02) && 
                  (msg_buf[0] != 'D') && (msg_buf[0] != 'N') && 
                  (msg_buf[0] != 'V') && (msg_buf[0] != 'R') &&
									(msg_buf[0] != 'S') && (msg_buf[0] != 'M') &&
				          (msg_buf[0] != 'd') && (msg_buf[0] != 'n') && 
                  (msg_buf[0] != 'v') && (msg_buf[0] != 'r') &&
									(msg_buf[0] != 's') && (msg_buf[0] != 'm') &&
				 					(msg_buf[0] != 'p') && (msg_buf[0] != 'P') &&
                  (msg_buf_idx != 0))
         {                          // if first character is bad in Quiet mode
            msg_buf_idx = 0;        // then start over
         }
         else {                        // not complete message, store character
 
            msg_buf[msg_buf_idx] = j;
            msg_buf_idx++;
            if (msg_buf_idx > 2)
            {
               UART_msg_process();
            }
         }
      }
   }
}

//*****************************************************************************/
///  \fn void UART_msg_process(void) 
///UART Input Message Processing
//*****************************************************************************/
void UART_msg_process(void)
{
  UCHAR chr, err = 0;

   if((chr = msg_buf[0]) <= 0x60) // Upper Case
   {      
      switch( chr ) 
      {
         case 'D':
            if((msg_buf[1] == 'E') && (msg_buf[2] == 'B') && (msg_buf_idx == 3)) 
            {
               display_mode = DEBUG;
               UART_msg_put("\r\nMode=DEBUG\n");
               display_timer = 0;
            }
            else
               err = 1;
            break;

         case 'N':
            if((msg_buf[1] == 'O') && (msg_buf[2] == 'R') && (msg_buf_idx == 3)) 
            {
               display_mode = NORMAL;
               UART_msg_put("\r\nMode=NORMAL\n");
               display_timer = 0;
            }
            else
               err = 1;
            break;

         case 'Q':
            if((msg_buf[1] == 'U') && (msg_buf[2] == 'I') && (msg_buf_idx == 3)) 
            {
               display_mode = QUIET;
               UART_msg_put("\r\nMode=QUIET\n");
               display_timer = 0;
            }
            else
               err = 1;
            break;

         case 'V':
            display_mode = VERSION;
            UART_msg_put("\r\n");
            UART_msg_put( CODE_VERSION ); 
            UART_msg_put("\r\nSelect  ");
            display_timer = 0;
            break;
				 
				 case 'R':
					 /* Only display registers if in debug mode */
						if (display_mode != DEBUG) {
							err = 2;
						}
						else
						{
							printRegs();
							display_timer = 0;
						}
            break;
						
				 case 'S':
					 /* Only display top of stack if in debug mode */
						if (display_mode != DEBUG) {
							err = 2;
						}
						else
						{
							UART_direct_msg_put("\r\n*** Top 16 words of Stack ***\r\n"); 
							print_mem((uint8_t *) read_sp(), 16); 
							display_timer = 0;
						}
						break;
						
				 case 'M':
					 /* Only do a memory display if in debug mode */
						if (display_mode != DEBUG) {
							err = 2;
						}
						else
						{
							UART_direct_msg_put("\r\nInput memory location in hex: ");
							uint32_t result = collectHex(); 
							
							if (result == 0)
								UART_direct_msg_put("\r\nInvalid input.\r\n");
							else
								print_mem((uint8_t *) result, 32); 
							
							
							display_timer = 0;
						}
						break;
						
				 case 'P':
						pause_flag = !pause_flag; 
						break; 
						
                
         default:
            err = 1;
						break; 
      }
   }
   else 	// Lower Case
   {
      switch( chr ) 
      {
         case 'd':
            if((msg_buf[1] == 'e') && (msg_buf[2] == 'b') && (msg_buf_idx == 3)) 
            {
               display_mode = DEBUG;
               UART_msg_put("\r\nMode=DEBUG\n");
               display_timer = 0;
            }
            else
               err = 1;
            break;

         case 'n':
            if((msg_buf[1] == 'o') && (msg_buf[2] == 'r') && (msg_buf_idx == 3)) 
            {
               display_mode = NORMAL;
               UART_msg_put("\r\nMode=NORMAL\n");
               display_timer = 0;
            }
            else
               err = 1;
            break;

         case 'q':
            if((msg_buf[1] == 'u') && (msg_buf[2] == 'i') && (msg_buf_idx == 3)) 
            {
               display_mode = QUIET;
               UART_msg_put("\r\nMode=QUIET\n");
               display_timer = 0;
            }
            else
               err = 1;
            break;

         case 'v':
            display_mode = VERSION;
            UART_msg_put("\r\n");
            UART_msg_put( CODE_VERSION ); 
            UART_msg_put("\r\nSelect  ");
            display_timer = 0;
            break;
				 
				 case 'r':
					 /* Only display registers if in debug mode */
						if (display_mode != DEBUG) {
							err = 2;
						}
						else
						{
							printRegs();
							display_timer = 0;
						}
            break;
						
				 case 's':
					 /* Only display top of stack if in debug mode */
						if (display_mode != DEBUG) {
							err = 2;
						}
						else
						{
							UART_direct_msg_put("\r\n*** Top 16 words of Stack ***\r\n"); 
							print_mem((uint8_t *) read_sp(), 16); 
							display_timer = 0;
						}
            break;
					
				 case 'm':
						/* Only do a memory display if in debug mode */
						if (display_mode != DEBUG) {
							err = 2;
						}
						else
						{
							UART_direct_msg_put("\r\nInput memory location in hex: ");
							uint32_t result = collectHex(); 
							
							if (result == 0)
								UART_direct_msg_put("\r\nInvalid input.\r\n");
							else
								print_mem((uint8_t *) result, 32); 
							
							
							display_timer = 0;
						}
						break;
						
				 case 'p':
						pause_flag = !pause_flag; 
						break; 
                
         default:
            err = 1;
						break;
      }
   }

   if( err == 1 )
   {
      UART_msg_put("\n\rError!");
   }   
   else if( err == 2 )
   {
      UART_direct_msg_put("\n\rNot in DEBUG Mode!");
   }   
   
	 msg_buf_idx = 0;          // put index to start of buffer for next message
}


//*****************************************************************************
///   \fn   is_hex
/// Function takes 
///  @param a single ASCII character and returns 
///  @return 1 if hex digit, 0 otherwise.
///    
//*****************************************************************************
UCHAR is_hex(UCHAR c)
{
   if( (((c |= 0x20) >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f'))  )
      return 1;
   return 0;
}

/*******************************************************************************
*   \fn  DEBUG and DIAGNOSTIC Mode UART Operation
*******************************************************************************/
void monitor(void)
{
	/**********************************/
	/*     Spew outputs               */
	/**********************************/
	char tempBuff[TX_BUF_SIZE]; 
	
	switch(display_mode)
	{
		case(QUIET):
			{
				UART_msg_put("\r\n ");
				display_flag = 0;
			}  
				break;
			
		case(VERSION):
			{
				display_flag = 0;
			}  
			break;         
		
		case(NORMAL):
			{
				if (display_flag == 1 && !pause_flag)
				{
					UART_msg_put("\r\nNORMAL ");
					
					UART_msg_put(" Flow: ");
					// ECEN 5803 add code as indicated
					//  add flow data output here, use UART_hex_put or similar for 
					// numbers. DONT NEED FOR MODULE 3. 
					
					UART_msg_put(" Temp: ");
					//  add flow data output here, use UART_hex_put or similar for 
					// numbers. DONT NEED FOR MODULE 3. 
					
					UART_msg_put(" Freq: ");
					UART_msg_put("\r\n");
					//  add flow data output here, use UART_hex_put or similar for 
					// numbers. DONT NEED FOR MODULE 3. 
					
					display_flag = 0;
				}
			}  
			break;
		
		case(DEBUG):
			{
				if (display_flag == 1 && !pause_flag)
				{
					UART_msg_put("\r\nDEBUG ");
					
					UART_msg_put(" Flow: ");
					// ECEN 5803 add code as indicated               
					//  add flow data output here, use UART_hex_put or similar for 
					// numbers. DONT NEED FOR MODULE 3. 
					
					UART_msg_put(" Temp: ");
					//  add flow data output here, use UART_hex_put or similar for 
					// numbers. DONT NEED FOR MODULE 3. 
					
					UART_msg_put(" Freq: ");
					UART_msg_put("\r\n");
					//  add flow data output here, use UART_hex_put or similar for 
					// numbers. DONT NEED FOR MODULE 3. 

					/* Display ARM Regs */
					printRegs();

					/* Display Error Count From UART */
					UART_direct_msg_put("\r\nUART Transmission Error Count:\t"); 
					my_itoa(error_count, (uint8_t *)tempBuff, 10);
					UART_direct_msg_put(tempBuff); 
					UART_direct_msg_put("\r\n"); 

					// clear flag from timer0    
					display_flag = 0;
				}   
			}  
			break;

		default:
			{
				UART_msg_put("Mode Error");
			}
			break;
	}
}  

void printRegs()
{
	uint32_t temp_reg; 
	UART_direct_msg_put("\r\n***Register values***");
	
	UART_direct_msg_put("\r\nr0:\t");
	temp_reg = read_gpr_0(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\nr1:\t");
	temp_reg = read_gpr_1(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\nr2:\t");
	temp_reg = read_gpr_2(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\nr3:\t");
	temp_reg = read_gpr_3(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\nr4:\t");
	temp_reg = read_gpr_4(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\nr5:\t");
	temp_reg = read_gpr_5(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\nr6:\t");
	temp_reg = read_gpr_6(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\nr7:\t");
	temp_reg = read_gpr_7(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\nr8:\t");
	temp_reg = read_gpr_8(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\nr9:\t");
	temp_reg = read_gpr_9(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\nr10:\t");
	temp_reg = read_gpr_10(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\nr11:\t");
	temp_reg = read_gpr_11(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\nr12:\t");
	temp_reg = read_gpr_12(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\nsp:\t");
	temp_reg = read_sp(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\nlr:\t");
	temp_reg = read_lr(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	
	UART_direct_msg_put("\r\npc:\t");
	temp_reg = read_pc(); 
	UART_direct_hex_put(temp_reg >> 24);
	UART_direct_hex_put((temp_reg >> 16) & 0xFF);
	UART_direct_hex_put((temp_reg >> 8) & 0xFF);
	UART_direct_hex_put(temp_reg &0xFF);
	UART_direct_msg_put("\r\n");
}

__asm uint32_t read_gpr_0()
{
	BX    lr
}

__asm uint32_t read_gpr_1()
{
	MOVS	r0, r1
	BX    lr
}

__asm uint32_t read_gpr_2()
{
	MOVS	r0, r2
	BX    lr
}

__asm uint32_t read_gpr_3()
{
	MOVS	r0, r3
	BX    lr
}

__asm uint32_t read_gpr_4()
{
	MOVS	r0, r4
	BX    lr
}

__asm uint32_t read_gpr_5()
{
	MOVS	r0, r5
	BX    lr
}

__asm uint32_t read_gpr_6()
{
	MOVS	r0, r6
	BX    lr
}

__asm uint32_t read_gpr_7()
{
	MOVS	r0, r7
	BX    lr
}

__asm uint32_t read_gpr_8()
{
	MOVS	r0, #0
	ADD	 	r0, r0, r8
	BX    lr
}

__asm uint32_t read_gpr_9()
{
	MOVS	r0, #0
	ADD	 	r0, r0, r9
	BX    lr
}

__asm uint32_t read_gpr_10()
{
	MOVS	r0, #0
	ADD	 	r0, r0, r10
	BX    lr
}

__asm uint32_t read_gpr_11()
{
	MOVS	r0, #0
	ADD	 	r0, r0, r11
	BX    lr
}

__asm uint32_t read_gpr_12()
{
	MOVS	r0, #0
	ADD	 	r0, r0, r12
	BX    lr
}

__asm uint32_t read_sp()
{
	MOVS	r0, #0
	ADD	 	r0, r0, sp
	BX    lr
}

__asm uint32_t read_lr()
{
	MOVS	r0, #0
	ADD	 	r0, r0, lr
	BX    lr
}

__asm uint32_t read_pc()
{
	MOVS	r0, #0
	ADD	 	r0, r0, pc
	BX    lr
}

/* Helper function declarations */
int64_t toPower(uint32_t base, uint8_t exponent);
uint8_t digitLookup(int32_t val);
int32_t multipleLookup(uint8_t digit);

uint8_t my_itoa(int32_t data, uint8_t * ptr, uint32_t base)
{
  /* Intermediate string buffer and associated values for building conversion */
  uint8_t strBuff[TX_BUF_SIZE];
  uint8_t * strPtr = strBuff; 
  uint8_t strLen = 0; 
  
  /* Ensure the base is within the supported range */
  if (base >= 2 && base <= 16)
  {
    /* One-time check for negative */ 
    if (data < 0)
    {
      /* Insert negative before digits in string, work with data as
       * a positive for the remaining parts of the algorithm
       */
      *strPtr++ = '-'; 
      strLen++; 
      data *= -1;
    }
    
    /* Zero is breaking corner case for algorithm, deal with it specially */ 
    if (data == 0)
    {
      *strPtr++ = '0';
      strLen++; 
    }
    else
    {
      uint8_t valMagnitude = 0;
      
      /* determine the highest power of the base that the data 
       * contains a multiple of. 
       */
      while ((data / toPower(base, valMagnitude)) > 0)
        valMagnitude++; 
      
      /* Deaccumulate the data from the highest power multiple to the 
       * lowest. Each power's multiple is a digit in the string.
       */
      while (valMagnitude-- > 0)
      {
        int32_t currMultiple, currPower;
        
        /* Multiple calculation and deaccumulation */
        currPower = (uint32_t) toPower(base, valMagnitude);
        currMultiple = data / currPower; 
        data -= currMultiple * currPower;
        
        /* Multiple digit lookup and write to conversion string */
        *strPtr++ = digitLookup(currMultiple);
        strLen++; 
      }
    }
  }
  
  /* Copy the converted string to the receiving buffer, if possible. If the
   * receiving ptr is NULL, then conversion fails and returns a converted string
   * length of zero.
   */
  if (ptr != NULL && strLen > 0)
  {
    size_t ndx; 
    
    /* Copy intermediate conversion string to receiving string buffer */
    for (ndx = 0; ndx < strLen; ndx++)
    {
      *ptr++ = *(strBuff + ndx); 
    }
    
    /* Last step is to insert NULL */
    *ptr = '\0';
    strLen++; 
  }
  else
  {
    strLen = 0; 
  }
  
  return strLen; 
}

int32_t my_atoi(uint8_t * ptr, uint32_t base)
{
  int32_t iAccum = 0; 
  uint8_t negFlag = 0;
  
  if (ptr != NULL)
  {
    uint8_t numDigits, * tmpPtr; 
    
    /* One-time negative check. Need to raise flag and progress ptr if so */
    if (*ptr == '-')
    {
      negFlag = 1; 
      ptr++; 
    }
    
    /* Determine the number of digits in the string */ 
    tmpPtr = ptr; 
    numDigits = 0; 
    while (*tmpPtr++)
      numDigits++; 
    
    /* Accumulate the multiples of each magnitude each digit represents.
     * Digits descend from highest magnitude to lowest, and are the multiple
     * multiple of that particular magnitude in the number
     */
    while (numDigits-- > 0)
    {
      int32_t currMultiple, currPower; 
      
      /* Lookup the integer representation of the current digit and ensure it
       * conforms with passed base 
       */
      currMultiple = multipleLookup(*ptr++);
      if (currMultiple >= base || currMultiple < 0)
      {
        /* Illegal digit for base. Return 0 result as error indicator */
        iAccum = 0;
        break; 
      }
      
      /* Calculate current power and add its multiple to the running total */
      currPower = (int32_t) toPower(base, numDigits); 
      iAccum += currMultiple * currPower; 
    }
  }
  
  /* Apply negative to accumulated value if applicable */
  iAccum = negFlag ? iAccum * -1 : iAccum; 
  
  return iAccum; 
}

/**
 * @brief Calculates the (signed) value of base raised to the exponent power. 
 * 64-bit return value used because the itoa algorithm probes above the 32-bit
 * boundary, and would cause a floating point overflow without a 64-bit value. 
 * @param base      The base value to be raised to a certain power. 
 * @param exponent  The exponent for the base value. 
 * @return          The calculated value of base raised to the exponent.
 */ 
int64_t toPower(uint32_t base, uint8_t exponent)
{
  int64_t accum = 1; 
  size_t i; 
  
  for (i = 0; i < exponent; i++)
  {
    accum *= base; 
  }
  
  return accum; 
}

/**
 * @brief Returns ascii digit representation of passed value. 
 * Values between 0-15 are supported. Any others will return a null character. 
 * @param val Value of the desired digit.  
 * @return    Digit representing the passed value. 
 */ 
uint8_t digitLookup(int32_t val)
{
  uint8_t retVal = '\0'; 
  
  switch (val)
  {
    case 0:
      retVal = '0'; 
      break; 
    
    case 1:
      retVal = '1';
      break; 
      
    case 2:
      retVal = '2'; 
      break; 
      
    case 3:
      retVal = '3';
      break;
      
    case 4:
      retVal = '4';
      break; 
      
    case 5:
      retVal = '5';
      break; 
      
    case 6:
      retVal = '6';
      break; 
      
    case 7:
      retVal = '7';
      break; 
      
    case 8:
      retVal = '8';
      break; 
      
    case 9:
      retVal = '9';
      break; 
      
    case 10:
      retVal = 'A'; 
      break; 
      
    case 11:
      retVal = 'B';
      break; 
      
    case 12:
      retVal = 'C'; 
      break; 
      
    case 13:
      retVal = 'D'; 
      break; 
      
    case 14:
      retVal = 'E';
      break; 
      
    case 15:
      retVal = 'F'; 
      break; 
      
    default:
      break; 
  }
  
  return retVal; 
}

/**
 * @brief Returns integer value represented by a passed char digit.
 * Digits 0-F are supported. Any others will return a negative value to
 * indicate an error. 
 * @param digit Char digit to convert to integer. 
 * @return      Integer value represented by the passed char digit. 
 */ 
int32_t multipleLookup(uint8_t digit)
{
  int32_t retVal = -1; 
  
  switch (digit)
  {
    case '0':
      retVal = 0; 
      break; 
    
    case '1':
      retVal = 1;
      break; 
      
    case '2':
      retVal = 2; 
      break; 
      
    case '3':
      retVal = 3;
      break;
      
    case '4':
      retVal = 4;
      break; 
      
    case '5':
      retVal = 5;
      break; 
      
    case '6':
      retVal = 6;
      break; 
      
    case '7':
      retVal = 7;
      break; 
      
    case '8':
      retVal = 8;
      break; 
      
    case '9':
      retVal = 9;
      break; 
      
    case 'A':
    case 'a':
      retVal = 10; 
      break; 
      
    case 'B':
    case 'b':
      retVal = 11;
      break; 
      
    case 'C':
    case 'c':
      retVal = 12; 
      break; 
      
    case 'D':
    case 'd':
      retVal = 13; 
      break; 
      
    case 'E':
    case 'e':
      retVal = 14;
      break; 
      
    case 'F':
    case 'f':
      retVal = 15; 
      break; 
      
    default:
      break; 
  }
  
  return retVal; 
}

void print_mem(uint8_t * start, uint32_t length)
{
  uint32_t i, j, temp_val; 
 
  i = 0; 
  while (i < length) 
  {
		temp_val = (uint32_t)start; 
		
    /* Row start index print */  
		UART_direct_hex_put(temp_val >> 24);
		UART_direct_hex_put((temp_val >> 16) & 0xFF);
		UART_direct_hex_put((temp_val >> 8) & 0xFF);
		UART_direct_hex_put(temp_val & 0xFF);
    UART_direct_msg_put(":");
    
    /* Print DBG_ROW_WIDTH bytes, or the remaining number of bytes */
    for (j = 0; j < 4 && i < length; j++, i++)
    {
			UART_direct_msg_put(" ");
			UART_direct_hex_put(*start >> 24);
			UART_direct_hex_put((*start >> 16) & 0xFF);
			UART_direct_hex_put((*start >> 8) & 0xFF);
			UART_direct_hex_put(*start & 0xFF);
			
			start++;
    }
    
    /* End row */
    UART_direct_msg_put("\r\n");
  }
}

uint32_t collectHex()
{
	char tempBuff[TX_BUF_SIZE], echoBuff[2]; 
	char * buffPtr = tempBuff; 
	
	/* Fill buffer with user input */
	do
	{
		while(!UART_input())
			serial();
		
		*buffPtr = UART_get(); 
		
		/* echo back*/
		echoBuff[0] = *buffPtr; 
		echoBuff[1] = '\0'; 
		UART_direct_msg_put(echoBuff); 
	} while (*buffPtr++ != '\r' && buffPtr < (tempBuff + TX_BUF_SIZE - 1)); 
	*(buffPtr - 1) = '\0'; 
	
	return my_atoi((uint8_t *) tempBuff, 16); 
}
