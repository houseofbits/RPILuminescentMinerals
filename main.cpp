#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <time.h>

using namespace std;

int main() {

    cout << "Placeholder" << endl;

    time_t start,end;

    time (&start);

    usleep(2000000);

    time (&end);
    
    double system_secs = difftime (end,start);

    cout << " Runtime: " << system_secs << "s" << endl;

    return(0);
}