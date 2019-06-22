#include <msp430.h>
#include "driverlib.h"
#include "Board.h"
#include "msp430fr4133.h"
#include "HAL_FR4133LP_LCD.h"
#include "HAL_FR4133LP_Learn_Board.h"
#include "stdbool.h"

void glow();
void setRowsHigh();
void setRowsLow();
void Key();
void setMotorsA_B_forward();
void setMotorsA_forward_B_backward();
void setMotorsA_backward_B_forward();
void setMotorsA_B_backward();
void setPWM();
void Brake();

char pressedKey;
int speed;
int highPeriod;
Timer_A_initCompareModeParam initComp2Param = {0};


void main (void)
{

    WDTCTL = WDTPW | WDTHOLD;               // Stop watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                   // Disable the GPIO power-on default high-impedance mode
                                            // to activate previously configured port settings

    Init_LCD();                                 //Initialize LCD

    speed = 0;
    highPeriod = 0;
    LCD_Display_Buttons(1); // SPEED
    LCD_Display_digit(pos6, speed); // 0

    P1DIR |= 0x01;                          // Set P1.0 to output direction
    P4DIR |= 0x01;                          // Set P2.0 to output direction

    P1OUT |= 0x01; // turn on red LED P1.0
    P4OUT &= 0x00; // turn off green LED P4.0

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

    GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P2, GPIO_PIN7, GPIO_PRIMARY_MODULE_FUNCTION);     // Column 2: Input direction
    GPIO_selectInterruptEdge(GPIO_PORT_P2, GPIO_PIN7, GPIO_HIGH_TO_LOW_TRANSITION);
    GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P2, GPIO_PIN7);
    GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN7);
    GPIO_enableInterrupt(GPIO_PORT_P2, GPIO_PIN7);

    // Set PWM PIN
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P8, GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);
    // Set Motor PINS to GPIO
    GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN0);                  // Motor A Input 1
    GPIO_setAsOutputPin(GPIO_PORT_P5, GPIO_PIN1);                  // Motor A Input 2
    GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN2);                  // Motor B Input 1
    GPIO_setAsOutputPin(GPIO_PORT_P8, GPIO_PIN1);                  // Motor B Input 2
    Brake();
    PMM_unlockLPM5(); // PWM related

    _EINT();        // Start interrupt

    //Start timer
    Timer_A_initUpModeParam param = {0};
    param.clockSource = TIMER_A_CLOCKSOURCE_SMCLK;
    param.clockSourceDivider = TIMER_A_CLOCKSOURCE_DIVIDER_1;
    param.timerPeriod = completePeriod;
    param.timerInterruptEnable_TAIE = TIMER_A_TAIE_INTERRUPT_DISABLE;
    param.captureCompareInterruptEnable_CCR0_CCIE = TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE;
    param.timerClear = TIMER_A_DO_CLEAR;
    param.startTimer = true;
    Timer_A_initUpMode(TIMER_A1_BASE, &param);


    //Initialize compare mode to generate PWM
    initComp2Param.compareRegister = TIMER_A_CAPTURECOMPARE_REGISTER_2;
    initComp2Param.compareInterruptEnable = TIMER_A_CAPTURECOMPARE_INTERRUPT_DISABLE;
    initComp2Param.compareOutputMode = TIMER_A_OUTPUTMODE_RESET_SET;


    glow(); // tight-poll the ultrasonic sensor

    PMM_unlockLPM5();           // Need this for LED to turn on- in case of "abnormal off state"
    __bis_SR_register(LPM4_bits + GIE);     // Need this for interrupts or else "abnormal termination"
    __no_operation();           //For debugger
}

void toggle_direction_LEDs(){
    P1OUT ^= 0x01; // toggle red LED P1.0
    P4OUT ^= 0x01; // toggle green LED P4.0
}

void setRowsHigh(){
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4); // Row 1- HIGH
    GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN6); // Row 2- HIGH
}

void setRowsLow(){
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN4); // Row 1- LOW
    GPIO_setOutputLowOnPin(GPIO_PORT_P1, GPIO_PIN6); // Row 2- LOW
}

void Brake(){
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0);
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1);
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN2);
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN1);
    // Initialize speed to 0 m/s
    setPWM(highPeriod);
}
void goBackwards(){
    setPWM(100);
    // set Motors A and B backwards
    // Motor A - Clockwise
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN0); // Motor A Input 1 - High
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1); // Motor A Input 2 - Low
    // Motor B - Counter-Clockwise
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN2); // Motor B Input 1 - Low
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN1); // Motor B Input 2 - High
}

void turnRight(){
    // set Motors A backward and Motor B forward
    // Motor A - Clockwise
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN0); // Motor A Input 1 - High
    GPIO_setOutputLowOnPin(GPIO_PORT_P5, GPIO_PIN1); // Motor A Input 2 - Low
    // Motor B - Clockwise
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN2); // Motor B Input 1 - High
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN1); // Motor B Input 2 - Low
}

void turnLeft(){
    // set Motor A forward and Motor B backward
    // Motor A - Counter-Clockwise
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0); // Motor A Input 1 - Low
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN1); // Motor A Input 2 - High
    // Motor B - Counter-Clockwise
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN2); // Motor B Input 1 - Low
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN1); // Motor B Input 2 - High
}

void goForward(){
    // set Motors A and B forward
    // Motor A - Counter-Clockwise
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN0); // Motor A Input 1 - Low
    GPIO_setOutputHighOnPin(GPIO_PORT_P5, GPIO_PIN1); // Motor A Input 2 - High
    // Motor B - Clockwise
    GPIO_setOutputHighOnPin(GPIO_PORT_P8, GPIO_PIN2); // Motor B Input 1 - High
    GPIO_setOutputLowOnPin(GPIO_PORT_P8, GPIO_PIN1); // Motor B Input 2 - Low
}

void setPWM(){
    initComp2Param.compareValue = highPeriod;
    Timer_A_initCompareMode(TIMER_A1_BASE, &initComp2Param);
    _delay_cycles(2000);
}
void Key()
{
        if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN5) == GPIO_INPUT_PIN_LOW){     // Column 1 to GND
            GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4); // Row 1- HIGH
            if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN5) == GPIO_INPUT_PIN_HIGH) { // Column 1 to HIGH
                LCD_Clear();
                if (speed < 9) {
                    speed++;
                    highPeriod = 100 * speed;
                    goForward();
                }
                LCD_Display_digit(pos6, speed);
                LCD_Display_Buttons(1);
                if (speed == 1) {
                    toggle_direction_LEDs();
                }
            } else {
                GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN6); // Row 2- HIGH
                if (GPIO_getInputPinValue(GPIO_PORT_P1, GPIO_PIN5) == GPIO_INPUT_PIN_HIGH) { // Column 1 to HIGH
                    LCD_Clear();
                    LCD_Display_letter(pos1, 11); // L
                    LCD_Display_letter(pos2, 4); // E
                    LCD_Display_letter(pos3, 5); // F
                    LCD_Display_letter(pos4, 19); // T
                    P1OUT |= 0x01; // turn on red LED P1.0
                    P4OUT &= 0x00; // turn off green LED P4.0
                    turnLeft();
                }
            }
        } else {
            if (GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) == GPIO_INPUT_PIN_LOW) {     // Column 2 to GND
                GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN4); // Row 1- HIGH
                if (GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) == GPIO_INPUT_PIN_HIGH) { // Column 2 to HIGH
                    LCD_Clear();
                    LCD_Display_letter(pos1, 17); // R
                    LCD_Display_letter(pos2, 8); // I
                    LCD_Display_letter(pos3, 6); // G
                    LCD_Display_letter(pos4, 7); // H
                    LCD_Display_letter(pos5, 19); // T
                    P1OUT |= 0x01; // turn on red LED P1.0
                    P4OUT &= 0x00; // turn off green LED P4.0
                    turnRight();
                } else {
                        GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN6); // Row 2- HIGH
                        if (GPIO_getInputPinValue(GPIO_PORT_P2, GPIO_PIN7) == GPIO_INPUT_PIN_HIGH) { // Column 2 to HIGH
                            LCD_Clear();
                            if (speed > -1) {
                                speed--;
                                highPeriod = 100 * speed;
                                if(highPeriod <= 0){
                                    highPeriod = 0;
                                }
                                goForward();
                            }
                            if (speed >= 0) {
                                LCD_Display_digit(pos6, speed);
                            } else if (speed == -1){
                                LCD_Display_letter(pos6, 17); // R
                                goBackwards();
                            }
                            if (speed == 0) {
                                toggle_direction_LEDs();
                            }
                            LCD_Display_Buttons(1);                    }
                }
            }
        }
        highPeriod = 100 * speed;
        setRowsLow();
}

#pragma vector = PORT1_VECTOR       // Using PORT1_VECTOR interrupt because P1.5 is in port 1
__interrupt void PORT1_ISR(void)
{
    Key();
    setPWM();
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN5);
    GPIO_clearInterrupt(GPIO_PORT_P1, GPIO_PIN6);
}

#pragma vector = PORT2_VECTOR       // Using PORT1_VECTOR interrupt because P2.7 is in port 2
__interrupt void PORT2_ISR(void)
{
    Key();
    setPWM();
    GPIO_clearInterrupt(GPIO_PORT_P2, GPIO_PIN7);
}
