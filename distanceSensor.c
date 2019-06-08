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
