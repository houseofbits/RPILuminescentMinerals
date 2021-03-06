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
#define LED_GAMMA_EXPONENT 0.8
#define LED_FADE_DELAY_US 0
#define LED_FADE_STEP 5

#define A_LEDPWM_PIN 1
#define B_LEDPWM_PIN 2
#define SENSOR_INPUT_PIN 1

//Screens - facing from electronic box, A-left, B-right
//1) Screen side = A|B
//2) Wavelength = UVA|UVB|UVC
//3) Lamp position = TOP|BOTTOM|LEFT|RIGHT
#define A_UVC_TOP_PIN 6
#define A_UVB_TOP_PIN 4
#define A_UVA_TOP_PIN 3
#define B_UVB_TOP_PIN 14
#define B_UVC_TOP_PIN 15
#define B_UVA_TOP_PIN 16


#define A_UVB_TOPLEFT_PIN 9
#define A_UVB_BOTTOMLEFT_PIN 10
#define A_UVA_BOTTOM_PIN 7
#define A_UVB_BOTTOMRIGHT_PIN 5
#define A_UVB_TOPRIGHT_PIN 8

#define B_UVB_TOPLEFT_PIN 11
#define B_UVA_BOTTOM_PIN 12
#define B_UVB_BOTTOMRIGHT_PIN 2
#define B_UVB_TOPRIGHT_PIN 1

#define INACTIVITY_RESET_TIMEOUT 5    //Inactivity period (S) after which sequence starts over

#define LED_LIGHT_ON_PERIOD 30
#define UVA_LIGHT_ON_PERIOD 30
#define UVB_LIGHT_ON_PERIOD 30
#define UVC_LIGHT_ON_PERIOD 30
#define UVALL_LIGHT_ON_PERIOD 30

void Debug(std::string str)
{
//    cout<<str<<endl;
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
    bus1->write_port(0, 0xFF);
    bus1->write_port(1, 0xFF);

    if(pwm)pwm->set_pwm(1, 0, PWM_RESOLUTION, 0x40);
    if(pwm)pwm->set_pwm(2, 0, PWM_RESOLUTION, 0x40);
}

/**
 * 
 * */
void InitIO()
{
    try{

        bus1 = new IoPi(0x20);
        bus1->set_port_direction(0, 0x00);
        bus1->set_port_direction(1, 0x00);        
        bus1->write_port(0, 0xFF);
        bus1->write_port(1, 0xFF);

        bus2 = new IoPi(0x21);

        bus1->set_port_direction(0, 0x00);
        bus1->set_port_direction(1, 0x00);
        bus2->set_port_direction(0, 0xFF);
        bus2->set_pin_pullup(1,1);
        bus2->set_pin_pullup(2,1);

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
void FadeLEDOff(int pinId)
{
    float f = 0;
    for (int x = 1; x <= PWM_RESOLUTION; x = x + LED_FADE_STEP) {
        usleep(LED_FADE_DELAY_US);
        f = exponentialEasing((float)x/(float)PWM_RESOLUTION, LED_GAMMA_EXPONENT);
        f = f * (float)PWM_RESOLUTION;
        if(pwm)pwm->set_pwm(pinId, 0, (int)f, 0x40);
    }
}

/**
 * @param LED output pin number
 * */
void FadeLEDOn(int pinId)
{
    float f = 0;
    for (int x = 1; x <= PWM_RESOLUTION; x = x + LED_FADE_STEP) {
        usleep(LED_FADE_DELAY_US);
        f = exponentialEasing((float)x/(float)PWM_RESOLUTION, LED_GAMMA_EXPONENT);
        f = (1.0 - f) * (float)PWM_RESOLUTION;        
        if(pwm)pwm->set_pwm(pinId, 0, (int)f, 0x40);
    }
}

/**
 * @param LED output pin number
 * */
void SetLEDOn(int pinId)
{
    if(pwm)pwm->set_pwm(pinId, 0, 0, 0x40);
}

/**
 * @param LED output pin number
 * */
void SetLEDOff(int pinId)
{
    if(pwm)pwm->set_pwm(pinId, 0, PWM_RESOLUTION, 0x40);
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
        else return true;       

        if (in == 0) {            
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
    if(bus1)bus1->write_pin(pin, 0);
}

/**
 * @param output pin number
 * */
void IOOff(int pin){
    if(bus1)bus1->write_pin(pin, 1);
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

void UVAOn(){
    IOOn(A_UVA_BOTTOM_PIN);            
    IOOn(B_UVA_BOTTOM_PIN);

    Delay(1);

    IOOn(A_UVA_TOP_PIN);            
    IOOn(B_UVA_TOP_PIN);            
}
void UVAOff(){
    Debug("UVA off");

    IOOff(A_UVA_BOTTOM_PIN);            
    IOOff(B_UVA_BOTTOM_PIN);            

    Delay(1);

    IOOff(A_UVA_TOP_PIN);            
    IOOff(B_UVA_TOP_PIN);            
}
void UVBOn(){
    Debug("UVB on");

    IOOn(A_UVB_TOPLEFT_PIN);            
    IOOn(A_UVB_TOPRIGHT_PIN);
    IOOn(A_UVB_BOTTOMLEFT_PIN);            
    IOOn(A_UVB_BOTTOMRIGHT_PIN);
    IOOn(B_UVB_TOPLEFT_PIN);            
    IOOn(B_UVB_TOPRIGHT_PIN);
    IOOn(B_UVB_BOTTOMRIGHT_PIN);

    Delay(1);

    IOOn(A_UVB_TOP_PIN);            
    IOOn(B_UVB_TOP_PIN);                        
}
void UVBOff(){
    Debug("UVB off");

    IOOff(A_UVB_TOPLEFT_PIN);            
    IOOff(A_UVB_TOPRIGHT_PIN);
    IOOff(A_UVB_BOTTOMLEFT_PIN);            
    IOOff(A_UVB_BOTTOMRIGHT_PIN);
    IOOff(B_UVB_TOPLEFT_PIN);            
    IOOff(B_UVB_TOPRIGHT_PIN);
    IOOff(B_UVB_BOTTOMRIGHT_PIN);

    Delay(1);    

    IOOff(A_UVB_TOP_PIN);            
    IOOff(B_UVB_TOP_PIN);                        
}
void UVCOn(){
    Debug("UVC on");

    IOOn(A_UVC_TOP_PIN);     
    IOOn(B_UVC_TOP_PIN);                        
}
void UVCOff(){
    Debug("UVC off");

    IOOff(A_UVC_TOP_PIN);            
    IOOff(B_UVC_TOP_PIN);                        
}



int main() 
{
    InitIO();

    //devTestOutputs();   return 0;

    while(true){

        try{
            
            ////////////////////////////////////////////////////////
            // Fade in LED light
            if(!WaitForSensorActivation()) continue; //Start over from beginning

            Debug("LED fade in");

            SetLEDOff(A_LEDPWM_PIN);
            SetLEDOff(B_LEDPWM_PIN);

            FadeLEDOn(A_LEDPWM_PIN);
            FadeLEDOn(B_LEDPWM_PIN);

            Debug("LED delay");

            //LED light delay period
            Delay(LED_LIGHT_ON_PERIOD);

            Debug("LED fade off");

            //Fade off LED light
            FadeLEDOff(A_LEDPWM_PIN);
            FadeLEDOff(B_LEDPWM_PIN);  

            ///////////////////////////////////////////////////////
            //Switch on UVA light            
            if(!WaitForSensorActivation()) continue;

            Debug("UVA on");

            UVAOn();

            Debug("UVA delay");

            //UVA light delay period
            Delay(UVA_LIGHT_ON_PERIOD);

            UVAOff();

            ///////////////////////////////////////////////////////
            //Switch on UVB light            
            if(!WaitForSensorActivation()) continue;

            UVBOn();

            Debug("UVB delay");

            //UVB light delay period
            Delay(UVB_LIGHT_ON_PERIOD);

            UVBOff();

            ///////////////////////////////////////////////////////
            //Switch on UVC light            
            if(!WaitForSensorActivation()) continue;

            UVCOn();    

            Debug("UVC delay");

            //UVC light delay period
            Delay(UVC_LIGHT_ON_PERIOD);
            
            ///////////////////////////////////////////////////////
            //Switch on ALL UV light            

            UVAOn();
            UVBOn();
            UVCOn();

            Debug("UVALL delay");
                
            Delay(UVALL_LIGHT_ON_PERIOD);

            UVCOff();
            UVBOff();
            UVAOff();

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