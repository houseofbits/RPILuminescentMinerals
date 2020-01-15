#include <unistd.h>
#include <stdio.h>
#include <stdexcept>
#include <math.h>
#include <iostream>
#include <time.h>
#include "abelibs/IOPi/ABE_IoPi.h"
#include "abelibs/ServoPi/ABE_ServoPi.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;

IoPi* bus1 = 0;
IoPi* bus2 = 0;
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
#define A_UVB_TOP_PIN 2
#define A_UVC_TOP_PIN 3
#define A_UVA_BOTTOM_PIN 4
#define A_UVB_LEFT_PIN 5
#define A_UVB_RIGHT_PIN 6
#define B_UVA_TOP_PIN 7
#define B_UVB_TOP_PIN 8
#define B_UVC_TOP_PIN 9
#define B_UVA_BOTTOM_PIN 10
#define B_UVB_LEFT_PIN 11
#define B_UVB_RIGHT_PIN 12

#define INACTIVITY_RESET_TIMEOUT 5    //Inactivity period (S) after which sequence starts over

/**
 * @param delay time in seconds
 * */
void Delay(int s)
{   
    for(int i = 0; i < s; i++)
        usleep(1000000);
}

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
    bus1->write_port(0, 0x00);
    bus2->write_port(1, 0x00);
}

/**
 * 
 * */
void InitIO()
{
    try{

        bus1 = new IoPi(0x20);
        bus2 = new IoPi(0x21);      

        bus1->set_port_direction(0, 0x00);
        bus1->set_port_direction(1, 0x00);
        bus2->set_port_direction(0, 0xFF);

        pwm = new ServoPi(0x40, 1);
        pwm->set_pwm_freq(300, 0x40);
        pwm->set_pwm(1, 0, 300, 0x40);
        pwm->output_enable();

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
        if(pwm)pwm->set_pwm(pinId, 0, (int)f, 0x40);
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
        if(pwm)pwm->set_pwm(pinId, 0, (int)f, 0x40);
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
        
        int in = 1; 
        
        if(bus2)in = bus2->read_pin(SENSOR_INPUT_PIN);
        
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
 * @param output pin number
 * */
void IOOn(int pin){
    if(bus1)bus1->write_pin(pin, 1);
}

/**
 * @param output pin number
 * */
void IOOff(int pin){
    if(bus1)bus1->write_pin(pin, 0);
}

/**
 * 
 * */
void devTestOutputs(){

    while(true){

        try{
            int option = 0;    

            cout<<"Option"<<endl;
            cout<<" 0 - Port 0 output"<<endl;
            cout<<" 1 - Port 1 input"<<endl;
            cout<<" 2 - PWM output (VALUE 0 - "<<PWM_RESOLUTION<<")"<<endl;                
            cout<<" 3 - Quit"<<endl;                
            cin>>option;

            if(option == 3)return;

            if(option == 0){
                int pin = 0, value = 0;
                cout<<"Set pin: ";
                cin>>pin;
                cout<<"Value: ";
                cin>>value;

                if(pin > 16 || value > 1)continue;

                if(value > 0)IOOn(pin);
                if(value == 0)IOOff(pin);
            }
            if(option == 1){
                int pin = 0, value = 0;
                cout<<"Read pin: ";
                cin>>pin;

                if(bus2)value = bus2->read_pin(pin);
                cout<<"Value = "<<value<<endl;
                cin;
            }
            if(option == 2){
                int pin = 0, value = 0;
                cout<<"Set pwm pin: ";
                cin>>pin;
                cout<<"Value: ";
                cin>>value;

                if(pin > 16)continue;

                if(pwm)pwm->set_pwm(pin, 0, (int)value, 0x40);
            }
        }
        catch (exception &e) {
            cout <<"Process error: "<< e.what() << endl;
            continue;
        } 
    }
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

    InitIO();

    devTestOutputs();

    return 0;

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

            IOOn(A_UVA_BOTTOM_PIN);

            IOOn(A_UVA_TOP_PIN);

            //...
        }
        catch (exception &e) {
            cout <<"Process error: "<< e.what() << endl;
            break;
        }            
    }

    if(pwm)delete pwm;
    if(bus2)delete bus2;     
    if(bus1)delete bus1; 

    return(0);
}