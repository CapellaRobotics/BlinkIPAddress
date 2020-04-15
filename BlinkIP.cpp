/************************************************************************************
 * MIT License
 *
 * Copyright (c) 2020 CapellaRobotics
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ************************************************************************************/









// To compile: g++ BlinkIP.cpp -o BlinkIP
// To run: ./BlinkIP

#include <stdio.h>
#include <fstream>
#include <unistd.h>

//IP address includes
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>

using namespace std;



void setupGPIO() {

	// First enable the GPIO (General Purpose Input/Output) Lines
	char fileName[64];
	sprintf(fileName, "/sys/class/gpio/export"); // open file to write
	ofstream gpioSetup(fileName);

	const char pinNumber_1[] = "53";
	gpioSetup.write(pinNumber_1, 2); // write gpio number
	gpioSetup.flush(); // flush to send

	const char pinNumber_2[] = "54";
	gpioSetup.write(pinNumber_2, 2); // write gpio number
	gpioSetup.flush(); // flush to send

	const char pinNumber_3[] = "55";
	gpioSetup.write(pinNumber_3, 2); // write gpio number
	gpioSetup.flush(); // flush to send

	const char pinNumber_4[] = "56";
	gpioSetup.write(pinNumber_4, 2); // write gpio number
	gpioSetup.flush(); // flush to send

	gpioSetup.close();



	// change them all to outputs
	for(int i = 0; i < 4; i++) {
		sprintf(fileName, "/sys/class/gpio/gpio%d/direction", 53 + i); // open file to write (gpio53 -> gpio56)
		ofstream gpioDirection(fileName);
		const char output[] = "out";
		gpioDirection.write(output, 3);
		gpioDirection.close();
	}
}


void setGPIO(unsigned int pin, int value) { // sets the individual led to the value specified
	char fileName[64];
	sprintf(fileName, "/sys/class/gpio/gpio%d/value", pin); // open file to write
	ofstream gpioFile(fileName);

	if(value != 0) { // turn off
		const char on = '1';
		gpioFile.write(&on, 1);
	}
	else { // turn off
		const char off = '0';
		gpioFile.write(&off, 1);
	}

	gpioFile.close();
}



void setLeds_Value(int val) { // set the leds based on the binary digits of the value provided

	setGPIO(53, (val >> 0) & 1);
	setGPIO(54, (val >> 1) & 1);
	setGPIO(55, (val >> 2) & 1);
	setGPIO(56, (val >> 3) & 1);

}

void displayVal(int val) { // blinks the value provided

	setLeds_Value(val); // set to value
	usleep(850 * 1000);
	setLeds_Value(0); // turn off
	usleep(150 * 1000);

}











bool stopIP = 0;

int main(int argc, char * argv[]) {
	printf("Starting BlinkIP\n");


	setupGPIO();



	struct ifaddrs * ifAddrStruct=NULL;
	struct ifaddrs * ifa=NULL;
	void * tmpAddrPtr=NULL;
	char addressBuffer[INET_ADDRSTRLEN];

	bool foundEth0 = 0;

	int dir = 0;

	while(!foundEth0) {

		getifaddrs(&ifAddrStruct);

		for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
			if (!ifa->ifa_addr) {
				continue;
			}

			if (ifa->ifa_addr->sa_family == AF_INET) { // check it is IP4
				// is a valid IP4 Address
				tmpAddrPtr=&((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;

				inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
				printf("Connection: %s -> IP Address: %s\n", ifa->ifa_name, addressBuffer); // display connection name and its IP Address

				if(strcmp(ifa->ifa_name, "eth0") == 0) { // check if it is our desired Connection (Eth0)
					foundEth0 = 1;
					break;
				}
			}
		}

		if (ifAddrStruct != NULL)
			freeifaddrs(ifAddrStruct);


		if(!foundEth0) {
			// We could not find our connection

			if(dir == 0) { //scroll from the right to the left turning leds on
				int val = 0;

				for(int i = 0; i < 4; i++) {
					// turn on one led at a time
					val |= 1 << i; // val = 0b0001 -> 0b0011 -> 0b0111 -> 0b1111
					setLeds_Value(val);
					usleep(250 * 1000);
				}
				dir = 1;
			}
			else if(dir == 1) { //scroll from the left to the right turning leds off
				int val = 0xF;
				for(int i = 3; i >= 0; i--) {
					//turn off on led at a time
					val &= ~(1 << i); // val = 0b0111 -> 0b0011 -> 0b0001 -> 0b0000
					setLeds_Value(val);
					usleep(250 * 1000);
				}
				dir = 0;
			}

		}
	}

	if(foundEth0) {
		while(1) {

			// Signal start of ip address
			for(int i = 0; i < 3; i++) { // Blink all leds 3 times
				setLeds_Value(0xF); // all on
				usleep(200 * 1000);
				setLeds_Value(0); // all off
				usleep(200 * 1000);
			}

			int val = 0;
			for(int i = 0; i < 4; i++) {   // scroll from the right to the left turning leds on
				val |= 1 << i;
				setLeds_Value(val);
				usleep(250 * 1000);
			}
			setLeds_Value(0); // turn all back off
			usleep(250 * 1000);


			// now start displaying ip address
			int index = 0;
			while(addressBuffer[index] != 0) {
				if(addressBuffer[index] == '.')
					displayVal(0xF); // all on for a .

				else if(addressBuffer[index] >= '0' && addressBuffer[index] <= '9') // verify it is a valid number
					displayVal((int) (addressBuffer[index] - '0'));   // turn character '0' (which equals 48 in ASCII), into decimal value 0 (example: '2' - '0'  -->  50 - 48 = 2)

				index++;
			}
		}
	}


	setLeds_Value(0); // turn all off
}

