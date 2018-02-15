/***************************************************
Import the following mbed library for I2C 
communication with the accelerometer
https://os.mbed.com/users/emilmont/code/MMA8451Q/
***************************************************/

#include "mbed.h"
#include "MMA8451Q.h"

/*MMA8451Q device address*/
#define MMA8451_I2C_ADDRESS (0x1d<<1)

/*I2C Pin selection*/
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
    printf("MMA8451 ID: %d\n", acc.getWhoAmI());
    
    while(1)
    {
    xyz_on_off.x_acc = abs(acc.getAccX());
    xyz_on_off.y_acc = abs(acc.getAccY());
    xyz_on_off.z_acc = abs(acc.getAccZ());
    wait(0.5f);//Wait before printing the value
    printf("\n\rX: %1.1f, Y: %1.1f, Z: %1.1f", xyz_on_off.x_acc, xyz_on_off.y_acc, xyz_on_off.z_acc);
    }
}
