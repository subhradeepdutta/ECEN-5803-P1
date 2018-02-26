/*Include files*/
#include "MKL25Z4.h"
#include "system_MKL25Z4.h"
#include "mbed.h"
#include "core_cm0plus.h"

#define MSB_16BIT (1 << 15)

Serial pc(USBTX, USBRX);

/*Intialize pins to read analog input*/
AnalogIn analog0(PTB0);//VREFL
AnalogIn analog1(PTB1);//J10_4
AnalogIn analog2(PTB2);//Temperature

/*Variables to store ADC values*/
unsigned int ADC_val_channel_0, ADC_val_channel_1; 
volatile unsigned int ADC_val_channel_2;

/**************************************
Description: This function performs
						 calibration for the ADC
						 and returns a value 
						 indicating if the process
						 was successful

Input: N/A

Output: The function returns 0 if the
			  calibration was successful else
				it returns 1
**************************************/

int ADC_Calibrate()
{
	uint16_t calibration = 0;
	
	/*Maximum hardware averaging for better calibration results*/
	ADC0->SC3 |= (ADC_SC3_AVGE_MASK | ADC_SC3_AVGS_MASK);
	
	/*Select software trigger for intiating conversion*/
	ADC0->SC2 &= ~(ADC_SC2_ADTRG_MASK);
	
	/*Starts the calibration sequence*/
	ADC0->SC3 |= ADC_SC3_CAL_MASK;
	
	/*Wait until COCO is set to indicate the calibration is complete*/
	while( ADC0->SC1[0] & ADC_SC1_COCO_MASK) ;
	
	/*Check the status of calibration by reading CALF mask*/
	int calibration_failure = (ADC0->SC3 & ADC_SC3_CALF_MASK) >> ADC_SC3_CALF_SHIFT;
	
	/*Sum the plus side calibration registers*/
	calibration += ADC0->CLP0;
  calibration += ADC0->CLP1;
  calibration += ADC0->CLP2;
  calibration += ADC0->CLP3;
  calibration += ADC0->CLP4;
  calibration += ADC0->CLPS;
	/*Divide the calibration variable by 2*/
	calibration = calibration/2;
	
	/*Set the MSB fpr the calibration variable*/
	calibration |= (MSB_16BIT);
	
	/*Store the values in plus side gain calibration register*/
	ADC0->PG = calibration;

	/*Update the minus side calibration registers*/
  calibration = 0;
  calibration += ADC0->CLM0;
  calibration += ADC0->CLM1;
  calibration += ADC0->CLM2;
  calibration += ADC0->CLM3;
  calibration += ADC0->CLM4;
  calibration += ADC0->CLMS;
	/*Divide the calibration variable by 2*/
  calibration = calibration/2;
	
	/*Set the MSB fpr the calibration variable*/
  calibration |= (MSB_16BIT);  

	/*Store the values in minus side gain calibration register*/
  ADC0->MG = calibration;

	/*Clear SC*/
  ADC0->SC3 &= ~(ADC_SC3_CAL_MASK);

  return (calibration_failure);
}


/**************************************
Description: This function intializes
						 ADC and checks if
						 calibration was successful

Input: N/A

Output: The function returns 0 if the
			  ADC initialization process
				was completed successfully
**************************************/
int ADC_Init()
{
		/*Provide clock for ADC0*/
    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;
	
		/*Enable clock to Port B*/
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK;
	
		/*PTB0 = ADC0_SE8*/
		PORTB->PCR[0]=PORT_PCR_MUX(0);
	
		/*PTB2 = ADC0SE12*/
		PORTB->PCR[2]= PORT_PCR_MUX(0);
	
		/*Enable single ended conversions*/
		ADC0->SC1[0] &= ~(ADC_SC1_DIFF_MASK);
	
		ADC0->CFG1 &= ~(ADC_CFG1_ADLPC_MASK);//No low power mode
	
		ADC0->CFG1 |= ADC_CFG1_ADLSMP_MASK | //Long sample time, clock div = 1
									ADC_CFG1_MODE(3);		//16 bit conversion, input bus clk 
	
		/*Default long sample time*/
    ADC0->CFG2 |= ADC_CFG2_ADLSTS(0);
		
	  ADC0->SC3  |= (ADC_SC3_ADCO_MASK | //Continuous Conversion for channel 2
									 ADC_SC3_AVGE_MASK); //Hardware average failed
		
		/* 4 samples averaged*/
		ADC0->SC3  &= ~(ADC_SC3_AVGS_MASK);
		
		int calibration_status = ADC_Calibrate();
		
		return (calibration_status);
}


int ADC_Read(int channel)
{
	/*Set the channel selection value by configuring the value of ADCH in SC1[0]*/
	if(channel == 0)
	{
		ADC0->SC1[0] &= ~(ADC_SC1_ADCH_MASK); 
		ADC0->SC1[0] |=ADC_SC1_ADCH(30);
	}
	else if(channel == 1)
	{
		ADC0->SC1[0] &= ~(ADC_SC1_ADCH_MASK); 
		ADC0->SC1[0] |=ADC_SC1_ADCH(9);
	}
	else if(channel == 2)
	{
		ADC0->SC1[0] &= ~(ADC_SC1_ADCH_MASK); 
		ADC0->SC1[0] |=ADC_SC1_ADCH(26);
	}
	
	/*Block until conversion is complete*/
	while((ADC0->SC1[0] & ADC_SC1_COCO_MASK)==0);
	
	/*COCO is cleared automatically after the respective data register is read*/
	return (ADC0->R[0]);
}


int main()
{
	volatile unsigned int temperature;
	unsigned int m;
    while(1)
    {     
			/*Check if calibration was successful*/
			if(ADC_Init() == 0)
			{
				ADC_val_channel_0 = ADC_Read(0);
				ADC_val_channel_1 = ADC_Read(1);
				ADC_val_channel_2 = ADC_Read(2);
			}
			/*Convert ADC value to celsius*/
			if((ADC_val_channel_2 - 7160)>0)
				m = 1646;
			else
				m = 1769;
			temperature = 25 - ((ADC_val_channel_2 - 716)/m);
			/* Print ADC values to Terminal */
			pc.printf("%d, %d, %d,  %d\n\r", ADC_val_channel_0, ADC_val_channel_1, ADC_val_channel_2, temperature);
        }
}
