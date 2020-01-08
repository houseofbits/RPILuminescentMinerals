#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <time.h>
#include "abelibs/ExpanderPi/ABE_ExpanderPi.h"
#include "abelibs/ServoPi/ABE_ServoPi.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;

ExpanderPi expi;
ServoPi pwm(0x40, 1);

#define PWM_RESOLUTION 4095

#define A_LEDPWM_PIN 1
#define B_LEDPWM_PIN 1
#define SENSOR_INPUT_PIN 2

//1) Exposition = A|B
//2) Wavelength = UVA|UVB|UVC
//3) Lamp position = TOP|BOTTOM|LEFT|RIGHT
#define A_UVA_TOP_PIN 1
#define A_UVA_BOTTOM_PIN 2
//...

/**
 * Aproximation For LED gamma correction
 * http://www.flong.com/texts/code/shapers_exp/
 * */
float exponentialEasing (float x, float a)
{
    float epsilon = 0.00001;
    float min_param_a = 0.0 + epsilon;
    float max_param_a = 1.0 - epsilon;
    a = max(min_param_a, min(max_param_a, a));

    if (a < 0.5){
        // emphasis
        a = 2.0*(a);
        float y = pow(x, a);
        return y;
    } else {
        // de-emphasis
        a = 2.0*(a-0.5);
        float y = pow(x, 1.0/(1-a));
        return y;
    }
}

/**
 * 
 * */
void ResetOutputs()
{
    

}

/**
 * 
 * */
void InitIO()
{
    try{

       // ExpanderPi expi;

        char io_direction = 0b00000001;

        // port - 0 = pins 1 to 8, port 1 = pins 9 to 16

        expi.io_set_port_direction(0, 0x00); // set the direction for bank 0 to be outputs
        expi.io_set_port_direction(1, 0xFF); // set bank 1 to be inputs
        expi.io_write_port(0, 0x00);

        expi.io_set_port_pullups(1, 0xFF);

        //expi.io_read_pin(9);

        //expi.io_write_pin(0, 1);
        
        //ServoPi pwm(0x40, 1);

        pwm.set_pwm_freq(300, 0x40);
        pwm.output_enable();
        //pwm.set_pwm(1, 0, 1000, 0x40);

        ResetOutputs();

	}
	catch (exception &e)
	{
		cout <<"Initialisation error: "<< e.what() << endl;
	}
}

/**
 * 
 * */
void FadeLEDIn(int pinId)
{
    for (int x = 1; x <= PWM_RESOLUTION; x = x + 1) {
        //pwm.set_pwm(pinId, 0, x, 0x40); // set the pwm width to x
    }
}

/**
 * 
 * */
void FadeLEDOut(int pinId)
{

    for (int x = 1; x <= PWM_RESOLUTION; x = x + 1) {
        //pwm.set_pwm(pinId, 0, x, 0x40); // set the pwm width to x
    }

}

/**
 * Wait until sensor is activated
 * */
bool WaitForSensorActivation()
{
    // while(true){   
        
    //     int in = expi.io_read_pin(SENSOR_INPUT_PIN);
        
    //     if (in) {
            
    //         return true;
    //     }

    //     if (inactivityTime > 5) {
    //         ResetOutputs();
    //         return false;
    //     }

    //     usleep(300000);
    // }
    return true;
}

/**
 *
 * */
void Delay(int s)
{   
    for(int i = 0; i < s; i++)
        usleep(1000000);
}

/**
 *
 * */
void IOOn(int pin){

}

/**
 *
 * */
void IOOff(int pin){

}

int main() 
{

    InitIO();

    cout << "Placeholder" << endl;

    time_t start,end;

    time (&start);

    usleep(2000000);

    time (&end);
    
    double system_secs = difftime (end,start);

    cout << " Runtime: " << system_secs << "s" << endl;

    /*
        while(true){

            if(!WaitForSensorActivation()) continue; //Start over from beginning

            FadeLEDIn(A_LEDPWM_PIN);
            FadeLEDIn(B_LEDPWM_PIN);
        
            Delay(5);

            IOOn(A_UVA_BOTTOM_PIN);

            FadeLEDOut(A_LEDPWM_PIN);
            FadeLEDOut(B_LEDPWM_PIN);  

            if(!WaitForSensorActivation()) continue;

            //Proceed to switch on UVA lamps

            IOOn(A_UVA_TOP_PIN);

            //...
        }


    */

    return(0);
}