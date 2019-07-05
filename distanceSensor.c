///*
// * distanceSensor.c
// *
// *  Created on: May 28, 2019
// *      Author: Michael Leung, mh2leung
//*    Resources: Code ported from HC-SR04 Demo for Arduino found here https://www.instructables.com/id/Simple-Arduino-and-HC-SR04-Example/
// */
//
//#include <msp430.h>
//#include "driverlib.h"
//#include "Board.h"
//#include "msp430fr4133.h"
//
//#define MAX_DIST 23200
//#define SPEED_OF_SOUND 340
//
//void glow(void);
//void poll();
//
//void glow (void)
//{
//    // P2.5 trigger
//    // P1.3 echo
//    // P5.0 LED/motor
//    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN5);
//    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);
//
//    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
//
//    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN0);
//    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN0);
//
//    while(1) { // tight polling loop, should be replaced by periodic polling interrupt in keypadButton.c
//        poll();
//    }
//
//}
//
//void poll() {
//    unsigned long timeElapsed;
//    unsigned long pulseWidth;
//
//    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
////    delay EXACTLY 10 microseconds via TimerA or RTC.h library
////    __delay_cycles(160); // 10ms = 1/(16MHz processor)*150 cycles
//    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);
//
//    // poll P1.3 until we read a low
//    while (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN3) == GPIO_INPUT_PIN_LOW);
//    timeElapsed = 0;
//    // poll P1.3 until we read a high
//    while (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN3) == GPIO_INPUT_PIN_HIGH){
//        timeElapsed++;
//    };
//    pulseWidth = timeElapsed;
//
//    if (pulseWidth < MAX_DIST/1600) { // magic number
//        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN0);
//    } else {
//        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0);
//    }
//}

#include <msp430.h>
#include "driverlib.h"
#include "Board.h"
#include "msp430fr4133.h"

#define MAX_DIST 23200
#define SPEED_OF_SOUND 340

int direction_state;

void glow(void);
void poll();
void Forward();
void Left();
void Right();
void Backward();

void glow (void)
{
    // P2.5 trigger
    // P1.3 echo
    // P5.0 LED/motor
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN5);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0);

    while(1) { // tight polling loop, should be replaced by periodic polling interrupt in keypadButton.c
        poll();
    }

}

void poll() {
    unsigned long timeElapsed;
    unsigned long pulseWidth;

    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
//    delay EXACTLY 10 microseconds via TimerA or RTC.h library
//    __delay_cycles(160); // 10ms = 1/(16MHz processor)*150 cycles
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);

    // poll P1.3 until we read a low
    while (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN3) == GPIO_INPUT_PIN_LOW);
    timeElapsed = 0;
    // poll P1.3 until we read a high
    while (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN3) == GPIO_INPUT_PIN_HIGH){
        timeElapsed++;
    };
    pulseWidth = timeElapsed;

    if (pulseWidth < MAX_DIST/1600) { // magic number
        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN0);
        // Kill the motors
        GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0);
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1);
        GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN2);
        GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN1);
    }
    else {
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0);
        switch(direction_state){
            case 1:
                Forward();
                break;
            case 2:
                Right();
                break;
            case 3:
                Left();
                break;
            case 4:
                Backward();
                break;
            default:
                break;
        }
    }
}

void Forward(){
    direction_state = 1;
    // set Motors A and B forward
    // Motor A - Counter-Clockwise
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0); // Motor A Input 1 - Low
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN1); // Motor A Input 2 - High
    // Motor B - Clockwise
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN2); // Motor B Input 1 - High
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN1); // Motor B Input 2 - Low
}


void Right(){
    direction_state = 2;
    // set Motors A backward and Motor B forward
    // Motor A - Clockwise
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN0); // Motor A Input 1 - High
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1); // Motor A Input 2 - Low
    // Motor B - Clockwise
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN2); // Motor B Input 1 - High
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN1); // Motor B Input 2 - Low
}

void Left(){
    direction_state = 3;
    // set Motor A forward and Motor B backward
    // Motor A - Counter-Clockwise
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0); // Motor A Input 1 - Low
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN1); // Motor A Input 2 - High
    // Motor B - Counter-Clockwise
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN2); // Motor B Input 1 - Low
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN1); // Motor B Input 2 - High
}

void Backward(){
    direction_state = 4;
    // set Motors A and B backwards
    // Motor A - Clockwise
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN0); // Motor A Input 1 - High
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1); // Motor A Input 2 - Low
    // Motor B - Counter-Clockwise
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN2); // Motor B Input 1 - Low
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN1); // Motor B Input 2 - High
}
