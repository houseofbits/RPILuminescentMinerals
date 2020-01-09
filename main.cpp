#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <time.h>
#include "abelibs/ExpanderPi/ABE_ExpanderPi.h"
#include "abelibs/ServoPi/ABE_ServoPi.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;

ExpanderPi* expi = 0;
ServoPi* pwm = 0;

#define PWM_RESOLUTION 4095
#define LED_GAMMA_EXPONENT 0.7
#define LED_FADE_DELAY_US 100

#define A_LEDPWM_PIN 1
#define B_LEDPWM_PIN 1
#define SENSOR_INPUT_PIN 2

//1) Screen side = A|B
//2) Wavelength = UVA|UVB|UVC
//3) Lamp position = TOP|BOTTOM|LEFT|RIGHT
#define A_UVA_TOP_PIN 1
#define A_UVB_TOP_PIN 1
#define A_UVC_TOP_PIN 1
#define A_UVA_BOTTOM_PIN 2
#define A_UVB_LEFT_PIN 2
#define A_UVB_RIGHT_PIN 2
#define B_UVA_TOP_PIN 1
#define B_UVB_TOP_PIN 1
#define B_UVC_TOP_PIN 1
#define B_UVA_BOTTOM_PIN 2
#define B_UVB_LEFT_PIN 2
#define B_UVB_RIGHT_PIN 2

#define INACTIVITY_RESET_TIMEOUT 5    //Inactivity period (S) after which sequence starts over

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
    expi->io_write_port(0, 0x00);
    expi->io_write_port(1, 0x00);
}

/**
 * 
 * */
void InitIO()
{
    try{

		//IoPi bus1(0x20);
		//IoPi bus2(0x21);

        expi = new ExpanderPi();
        pwm = new ServoPi(0x40, 1);

        //Pins 1 to 8 - output
        expi->io_set_port_direction(0, 0x00);

        // port - 0 = pins 1 to 8, port 1 = pins 9 to 16
        char port1_io_direction = 0b00000011;

        //Pins 9 - 14 - output
        //Pins 15, 16 - input
        expi->io_set_port_direction(1, 0xFF);

        expi->io_write_port(0, 0x00);

        expi->io_set_port_pullups(1, 0xFF);

        //expi->io_read_pin(9);

        //expi->io_write_pin(0, 1);

        pwm->set_pwm_freq(300, 0x40);
        pwm->output_enable();
        //pwm->set_pwm(1, 0, 1000, 0x40);

        ResetOutputs();

	}
	catch (exception &e)
	{
		cout <<"Initialisation error: "<< e.what() << endl;
	}
}

/**
 * @param LED output pin number
 * */
void FadeLEDIn(int pinId)
{
    float f = 0;
    for (int x = 1; x <= PWM_RESOLUTION; x = x + 1) {
        usleep(LED_FADE_DELAY_US);
        f = exponentialEasing((float)x/(float)PWM_RESOLUTION, LED_GAMMA_EXPONENT);
        f = f * (float)PWM_RESOLUTION;
        pwm->set_pwm(pinId, 0, (int)f, 0x40);
    }
}

/**
 * @param LED output pin number
 * */
void FadeLEDOut(int pinId)
{
    float f = 0;
    for (int x = 1; x <= PWM_RESOLUTION; x = x + 1) {
        usleep(LED_FADE_DELAY_US);
        f = exponentialEasing((float)x/(float)PWM_RESOLUTION, LED_GAMMA_EXPONENT);
        f = f * (float)PWM_RESOLUTION;        
        pwm->set_pwm(pinId, 0, (int)f, 0x40);
    }
}

/**
 * Wait until sensor is activated
 * @return true on sensor active, false on incativity timeout
 * */
bool WaitForSensorActivation()
{
    time_t start,end;
    double inactivity_time;

    time (&start);

    while(true){   
        
        int in = 1; //expi.io_read_pin(SENSOR_INPUT_PIN);
        
        if (in) {
            
            return true;
        }

        time (&end);
        
        inactivity_time = difftime (end,start);

        if (inactivity_time > INACTIVITY_RESET_TIMEOUT) {
            ResetOutputs();
            return false;
        }

        usleep(300000);
    }
    return true;
}

/**
 * @param delay time in seconds
 * */
void Delay(int s)
{   
    for(int i = 0; i < s; i++)
        usleep(1000000);
}

/**
 * @param output pin number
 * */
void IOOn(int pin){
    expi->io_write_pin(pin, 1);
}

/**
 * @param output pin number
 * */
void IOOff(int pin){
    expi->io_write_pin(pin, 0);
}

int main() 
{

    //  float a = 0.7f;   
    //  cout<<"0.1  "<<exponentialEasing(0.1,a)<<endl;     
    //  cout<<"0.2  "<<exponentialEasing(0.2,a)<<endl;     
    //  cout<<"0.35 "<<exponentialEasing(0.35,a)<<endl;          
    //  cout<<"0.5  "<<exponentialEasing(0.5,a)<<endl;
    //  cout<<"0.65 "<<exponentialEasing(0.65,a)<<endl;     
    //  cout<<"0.8  "<<exponentialEasing(0.8,a)<<endl;
    //  cout<<"0.9  "<<exponentialEasing(0.9,a)<<endl;     
    //  return 0;

    //InitIO();

    // cout << "Placeholder" << endl;

    // time_t start,end;

    // time (&start);

    // usleep(2000000);

    // time (&end);
    
    // double system_secs = difftime (end,start);

    // cout << " Runtime: " << system_secs << "s" << endl;

    while(true){

        try{

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
        catch (exception &e) {
            cout <<"Process error: "<< e.what() << endl;
            break;
        }            
    }

    delete pwm;
    delete expi; 

    return(0);
}