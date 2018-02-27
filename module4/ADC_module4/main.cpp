/*Include files*/
#include "MKL25Z4.h"
#include "system_MKL25Z4.h"
#include "mbed.h"
#include "core_cm0plus.h"
#include "stdlib.h"

#define MSB_16BIT (1 << 15)
#define TEMP_SENSOR_CHANNEL_INPUT (26)
#define VREFSL_CHANNEL_INPUT (30)
#define J10_4_CHANNEL_INPUT (9)
#define V_TEMP25 (716)


Serial pc(USBTX, USBRX);

/*Initialize pins to read analog input*/
AnalogIn analog0(PTB0);//VREFL
AnalogIn analog1(PTB1);//J10_4
AnalogIn analog2(PTB2);//Temperature

/*Variables to store ADC values*/
float ADC_val_channel_0, ADC_val_channel_1, ADC_val_channel_2; 
 
/****************************************
Description: This function performs
			 calibration for the ADC
			 and returns a value 
			 indicating if the process
			 was successful

Input: N/A

Output: The function returns 0 if the
		calibration was successful else
		it returns 1
****************************************/

int ADC_Calibrate()
{
	uint16_t calibration = 0;
	
	/*Maximum hardware averaging for better calibration results*/
	ADC0->SC3 |= (ADC_SC3_AVGE_MASK | ADC_SC3_AVGS_MASK);
	
	/*Select software trigger for initiating conversion*/
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
	
	/*Set the MSB for the calibration variable*/
	calibration |= (MSB_16BIT);
	
	/*Store the values in plus side gain calibration register*/
	ADC0->PG = calibration;

	/*Reset the value for minus side*/
	calibration = 0;
	/*Update the minus side calibration registers*/
	calibration += ADC0->CLM0;
	calibration += ADC0->CLM1;
	calibration += ADC0->CLM2;
	calibration += ADC0->CLM3;
	calibration += ADC0->CLM4;
	calibration += ADC0->CLMS;
	/*Divide the calibration variable by 2*/
	calibration = calibration/2;
	
	/*Set the MSB for the calibration variable*/
	calibration |= (MSB_16BIT);  

	/*Store the values in minus side gain calibration register*/
	ADC0->MG = calibration;

	/*Clear SC*/
	ADC0->SC3 &= ~(ADC_SC3_CAL_MASK);

	return (calibration_failure);
}


/****************************************
Description: This function initializes the
			 ADC and checks if the 
			 calibration was successful

Input: N/A

Output: The function returns 0 if the
		ADC initialization process
		was completed successfully else
		returns 1 for failure
****************************************/
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
								 ADC_SC3_AVGE_MASK); //Hardware averaging enabled
	
	/* 4 samples averaged*/
	ADC0->SC3  &= ~(ADC_SC3_AVGS_MASK);
	
	int calibration_status = ADC_Calibrate();
	
	return (calibration_status);
}

/****************************************
Description: This function accepts the 
			 channel number as input and
			 performs a read from the ADC
			 for the corresponding channel
			 by setting the channel values

Input: ADC channel whose value is to be
	   read

Output: The function returns the value of
		the conversion by reading it from
		the corresponding data register or
		returns 0 if invalid channel
		number has been passed
****************************************/


uint16_t ADC_Read(int channel)
{
	/*Set the channel selection value by configuring the value of ADCH in SC1[0]*/
	if(channel == 0)
	{
		ADC0->SC1[0] &= ~(ADC_SC1_ADCH_MASK); 
		ADC0->SC1[0] |= ADC_SC1_ADCH(VREFSL_CHANNEL_INPUT);
	}
	else if(channel == 1)
	{
		ADC0->SC1[0] &= ~(ADC_SC1_ADCH_MASK); 
		ADC0->SC1[0] |= ADC_SC1_ADCH(J10_4_CHANNEL_INPUT);
	}
	else if(channel == 2)
	{
		ADC0->SC1[0] &= ~(ADC_SC1_ADCH_MASK); 
		ADC0->SC1[0] |= ADC_SC1_ADCH(TEMP_SENSOR_CHANNEL_INPUT);
	}
	else
	{
		return (0x0000);
	}
	
	/*Block until conversion is complete*/
	while((ADC0->SC1[0] & ADC_SC1_COCO_MASK)==0);
	
	/*COCO is cleared automatically after the respective data register is read*/
	return (ADC0->R[0]);
}


int main()
{
	volatile unsigned int temperature;
	float temp_val=0;
	float m;
	while(1)
    {     
		/*Check if calibration was successful*/
		if(!(ADC_Init()))
		{
			temp_val = (ADC_Read(0)/65535);
			ADC_val_channel_0 = (temp_val);
			/*Virtual value so conversion is not needed*/
			ADC_val_channel_1 = ADC_Read(1);
			temp_val = (ADC_Read(2)/65535);
			ADC_val_channel_2 = (temp_val);
		}
		/*Convert ADC value to Celsius based on formula */
		if ((ADC_val_channel_2 - 0.716)>0) 
             m = 1.646;
        else m = 1.769;
        temperature =  25 - (abs(ADC_val_channel_2 - 0.716) / m) ;
		
		
		/* Print ADC values to Terminal */
		pc.printf("%d, %d, %d,  %d\n\r", ADC_val_channel_0, ADC_val_channel_1, ADC_val_channel_2, temperature);
    }
}
