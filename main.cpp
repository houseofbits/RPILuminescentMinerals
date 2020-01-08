#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <time.h>
#include "abelibs/ExpanderPi/ABE_ExpanderPi.h"
#include "abelibs/ServoPi/ABE_ServoPi.h"

using namespace std;
using namespace ABElectronics_CPP_Libraries;

int main() {

    try{

        ExpanderPi expi;

        ServoPi pwm(0x40, 1);

        pwm.set_pwm_freq(300, 0x40);
        pwm.output_enable();

	}
	catch (exception &e)
	{
		cout << e.what() << endl;
	}

    cout << "Placeholder" << endl;

    time_t start,end;

    time (&start);

    usleep(2000000);

    time (&end);
    
    double system_secs = difftime (end,start);

    cout << " Runtime: " << system_secs << "s" << endl;

    return(0);
}