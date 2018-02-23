/**----------------------------------------------------------------------------
 
   \file main.cpp

--                                                                           --
--              ECEN 5803 Mastering Embedded System Architecture             --
--                  Project 1                                       --
--                Microcontroller Firmware                                   --
--                      main.cpp                                             --
--                                                                           --
-------------------------------------------------------------------------------
--
--  Designed for:  University of Colorado at Boulder
--               
--                
--  Designed by:  Tristan, Subhradeep, Omkar
--  
-- 
-- Version: 2.1
-- Target Microcontroller: MKL25Z128VLK4
-- Tools used:  ARM mbed compiler
--              ARM mbed 
--              FRDM KL25Z
-- 
-- Functional Description:  Main file that displays and calculates DMIPS
--                          
--                          
--                          
--                           
--
--      Copyright (c) 2017, 2018 Tim Scherr  All rights reserved.
--
--
-- 
-- 
--  


*/
/**
*@function main()
*
*@description 
*             
*             1. Calculates the DMIPS for the MCU
*@parameter void
*
*@return void
*
*@date 22th February 2018
*/ 



#include "mbed.h"
#include "dhry.h"
 
Timer timer;

Serial pc(USBTX, USBRX);  //serial channel over HDK USB interface
 
int main() {
    double btime, dps;
    unsigned long iterations;
    
    pc.baud(9600);
    pc.printf("DMIPS Calculation Program \r\n");
    pc.printf("Please Wait for a minute... \r\n");
    
    timer.start();
        do {
            Proc0();
            Proc0();
            iterations += LOOPS;
            btime = timer.read();
        } while (btime <= 60.000);
        dps = (double)iterations / btime;
        printf("Dhrystone time for %ld passes = %.3f sec\r\n", iterations, btime);
        printf("benchmark is at %0.f dhrystones/second\r\n", dps);
        printf("DMIPS is: %f\n\r", (dps/1757.0));
        printf("End of the Program");
        wait(1.0);
    
}
