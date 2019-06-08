/*
 * distanceSensor.c
 *
 *  Created on: May 28, 2019
 *      Author: Michael Leung, mh2leung
*    Resources: HC-SR04 Demo for Arduino
 */

#include <msp430.h>
#include "driverlib.h"
#include "Board.h"
#include "msp430fr4133.h"
//#include "HAL_FR4133LP_LCD.h"
//#include "HAL_FR4133LP_Learn_Board.h"
//#include "RTC.h" // for real-time clock applications

#define MAX_DIST 23200
#define SPEED_OF_SOUND 340

void toggleLED();
void glow(void);
void poll();

int ledOn;
//TI_second = 0;

void glow (void)
{
    // P2.5 trigger
    // P1.3 echo
    // P5.0 LED/motor
    ledOn = 0x0;
    GPIO_setAsOutputPin(GPIO_PORT_P2, GPIO_PIN5);
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);

    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN0);
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN0);

    while(1) { // tight polling loop, should be replaced by periodic polling interrupt in keypadButton.c
        poll();
    }

}

void toggleLED() {
    ledOn ^= 0x1;
    if (ledOn > 0) {
        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN0);
    } else {
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0);
    }
}

void poll() {
    unsigned long sendTime;
    unsigned long receiveTime;
    unsigned long pulseWidth;
//    float distance_cm;

    GPIO_setOutputHighOnPin(GPIO_PORT_P2, GPIO_PIN5);
//    delay EXACTLY 10 microseconds via TimerA or RTC.h library
//    __delay_cycles(160); // 10ms = 1/(16MHz processor)*150 cycles
    GPIO_setOutputLowOnPin(GPIO_PORT_P2, GPIO_PIN5);

    while (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN3) == GPIO_INPUT_PIN_LOW);

    //    sendTime = TI_second;
        sendTime = 0;
    while (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN3) == GPIO_INPUT_PIN_HIGH){
//        incrementSeconds();
        sendTime++;
    };
//    receiveTime = TI_second;

//    pulseWidth = sendTime - receiveTime; // this is BCD, but it needs to be in decimal for the comparison
    pulseWidth = sendTime;
//    distance_cm = pulseWidth * SPEED_OF_SOUND / 2;

    if (pulseWidth < MAX_DIST/1600) { // magic number
//        toggleLED();
        GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN0);
    } else {
        GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN0);
    }
}
