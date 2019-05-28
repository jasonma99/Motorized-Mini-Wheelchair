/* DESCRIPTION
 * Sample code showing keypad's response to pushing buttons 1 and 2
 * Include pressedKey on debugger's "Expressions" to see the hexaKeys' value when you alternate between the two keys
 * Did not include button debouncer in this (releasing the button does not set pressedKey back to Value 0 '\x00')
 */


#include <msp430.h>
#include "driverlib.h"
#include "Board.h"
#include "msp430fr4133.h"
#include "HAL_FR4133LP_LCD.h"
#include "HAL_FR4133LP_Learn_Board.h"
//#include "distanceSensor.c"
//#include "RTC.h" // for real-time clock applications

char hexaKeys[4][3] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

void setRowsHigh();
void setRowsLow();
void Key();
char pressedKey;
int speed;

void main (void)
{
    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
/*    setTime( 0x12, 0, 0, 0);                // initialize time to 12:00:00 AM
    P1DIR |= 0x01;                          // Set P1.0 to output direction
    CCR0 = 32768-1;
    TACTL = TASSEL_1+MC_1;                  // ACLK, upmode
    CCTL0 |= CCIE;                          // enable CCRO interrupt
    _EINT();
    while( 1 )
    {
        LPM3; // enter LPM3, clock will be updated
        P1OUT ^= 0x01; // do any other needed items in loop
    }*/


    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    glow();

    Init_LCD();                                 //Initialize LCD

    speed = 0;
    LCD_Display_Buttons(1); // SPEED
    LCD_Display_digit(pos6, speed); // 0

    P1DIR |= 0x01;                          // Set P1.0 to output direction
    P4DIR |= 0x01;                          // Set P2.0 to output direction

    P1OUT = 0x01; // turn on red LED P1.0
    P4OUT = 0x00; // turn off green LED P4.0

    // ROWS ARE OUTPUTS
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN4);                  // Row 1: Output direction
    GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN6);                  // Row 2: Output direction
    setRowsLow();

    // COLUMNS ARE ISR TRIGGERS

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);     // Column 1: Input direction
    GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN5, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN5);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN5);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN5);

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);     // Column 2: Input direction
    GPIO_selectInterruptEdge(GPIO_PORT_P1, GPIO_PIN3, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P1, GPIO_PIN3);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN3);
    GPIO_enableInterrupt(GPIO_PORT_P1, GPIO_PIN3);

    _EINT();        // Start interrupt

//    while(1) { // poll the ultrasonic sensor
//
//    }
    PMM_unlockLPM5();           // Need this for LED to turn on- in case of "abnormal off state"
    __bis_SR_register(LPM4_bits + GIE);     // Need this for interrupts or else "abnormal termination"
    __no_operation();           //For debugger
}

void toggleLEDs(){
    P1OUT ^= 0x01; // toggle red LED P1.0
    P4OUT ^= 0x01; // toggle green LED P4.0
}

void setRowsHigh(){
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4); // Row 1- HIGH
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN6); // Row 2- HIGH
}

void setRowsLow(){
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4); // Row 1- HIGH
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN6); // Row 2- HIGH
}

void Key()
{
        setRowsLow();
        if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN5) == GPIO_INPUT_PIN_LOW){     // Column 1 to GND
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4); // Row 1- HIGH
            if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN5) == GPIO_INPUT_PIN_HIGH) { // Column 1 to HIGH
//                LCD_Clear();
//                LCD_Display_digit(pos1, 1);
                LCD_Clear();
                if (speed < 9) {
                    speed++;
                }
                LCD_Display_digit(pos6, speed);
                LCD_Display_Buttons(1);
                if (speed == 1) {
                    toggleLEDs();
                }
            } else {
                GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN6); // Row 2- HIGH
                if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN5) == GPIO_INPUT_PIN_HIGH) { // Column 1 to HIGH
                    LCD_Clear();
                    LCD_Display_letter(pos1, 11); // L
                    LCD_Display_letter(pos2, 4); // E
                    LCD_Display_letter(pos3, 5); // F
                    LCD_Display_letter(pos4, 19); // T
                    speed = 0;
                }
            }

        } else if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN3) == GPIO_INPUT_PIN_LOW) {     // Column 2 to GND
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4); // Row 1- HIGH
            if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN3) == GPIO_INPUT_PIN_HIGH) { // Column 2 to HIGH
                LCD_Clear();
                LCD_Display_letter(pos1, 17); // R
                LCD_Display_letter(pos2, 8); // I
                LCD_Display_letter(pos3, 6); // G
                LCD_Display_letter(pos4, 7); // H
                LCD_Display_letter(pos5, 19); // T
                speed = 0;
            } else {
                GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN6); // Row 2- HIGH
                if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN3) == GPIO_INPUT_PIN_HIGH) { // Column 2 to HIGH
//                    LCD_Clear();
//                    LCD_Display_digit(pos1, 5);
                    LCD_Clear();
                    if (speed > -1) {
                        speed--;
                    }
                    if (speed >= 0) {
                        LCD_Display_digit(pos6, speed);
                    } else if (speed == -1){
                        LCD_Display_letter(pos6, 17); // R
                    }
                    if (speed == 0) {
                        toggleLEDs();
                    }
                    LCD_Display_Buttons(1);
                }
            }

        }
        setRowsLow();
}

#pragma vector = PORT1_VECTOR       // Using PORT1_VECTOR interrupt because P1.4 and P1.5 are in port 1
__interrupt void PORT1_ISR(void)
{
    Key();

    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN3);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN4);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN5);
}

// Timer A0 interrupt service routine
#pragma vector=TIMERA0_VECTOR
__interrupt void Timer_A (void)
{
    incrementSeconds();
    LPM3_EXIT;
}
