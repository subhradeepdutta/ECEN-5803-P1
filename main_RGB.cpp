#include "mbed.h"
#include "MMA8451Q.h"


//PwmOut g(LED_GREEN);
#define MMA8451_I2C_ADDRESS (0x1d<<1)
#define PWM_OFF 100
#define PERIOD  0.001f
PinName const SDA = PTE25;
PinName const SCL = PTE24;

typedef struct
{
    float x_acc;    //Acceleration along x axis
    float y_acc;    //Acceleration along y axis
    float z_acc;    //Acceleration along z axis
} xyz_data;

xyz_data xyz_on_off;

int main()
{ 
MMA8451Q acc(SDA, SCL, MMA8451_I2C_ADDRESS);
/*I2C Pin selection*/


float Touch_Sen_In = 0.95f; //Touch Sensor Input
float PWM_r = 1.05f;
float PWM_g = 1.05f;
float PWM_b = 1.05f;
PwmOut r(LED_RED);
PwmOut g(LED_GREEN);
PwmOut b(LED_BLUE);
r.period(PERIOD);
g.period(PERIOD);
b.period(PERIOD);


    while (true) {
          xyz_on_off.x_acc = abs(acc.getAccX());
          xyz_on_off.y_acc = abs(acc.getAccY());
          xyz_on_off.z_acc = abs(acc.getAccZ());
          if(xyz_on_off.x_acc <= NULL)    // Please put suitable threshold and change all signs to greater than equal to after testing LEDs once
          {
          PWM_r =PWM_OFF;
          }
          if(xyz_on_off.x_acc <= NULL)    // Please put suitable threshold and change all signs to greater than equal to after testing LEDs once
          {
          PWM_g =PWM_OFF;
          }
          if(xyz_on_off.x_acc <= NULL)    // Please put suitable threshold and change all signs to greater than equal to after testing LEDs once
          {
          PWM_b =PWM_OFF;
          }
          
          r= PWM_r*Touch_Sen_In;
          g= PWM_g*Touch_Sen_In;
          b= PWM_b*Touch_Sen_In;   
          printf("\n\rX: %1.1f, Y: %1.1f, Z: %1.1f", xyz_on_off.x_acc, xyz_on_off.y_acc, xyz_on_off.z_acc);                                     
        
    }
}
