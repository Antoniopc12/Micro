/*
 * Ejemplo de configuración de un pulso PWM con el generador PWM de la TIVA
 *
 * IMPORTANTE: En este ejemplo no se usa ni la frecuencia (se usa 6.25KHz en vez de 50Hz) ni el rango de ciclos de trabajos
 * (el ciclo va de 0 a 100 respecto al periodo, mientras que en un servo iría de 1ms a 2ms) válidos para trabajar con los
 * servos del microbot. Los estudiantes tendrán que realizar las adaptaciones adecuadas.
 *
 * OJO! El valor a cargar en PWMGenPeriodSet y  PWMPulseWidthSet DEBE caber en 16 bits! (<65535), aunque la variable sea uint32_t
 */

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/pwm.h"
#include "driverlib/pin_map.h"
#include "driverlib/gpio.h"

#define PWM_BASE_FREQ 100

int main(void)
{
    uint32_t val_load, pwm_clk;
    static uint32_t valor;
    uint16_t duty = 1;
    uint8_t up=1, down=0;
    uint16_t i = 0;

    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); // Reloj del sistema a 40MHz
    SysCtlPWMClockSet(SYSCTL_PWMDIV_64);    // Establece reloj del sistema (40MHz/64=625KHz)

    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM0); //Habilita modulo PWM
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);    // Habilita puerto salida para señal PWM (ver en documentacion que pin se corresponde a cada módulo PWM)

    GPIOPinTypePWM(GPIO_PORTB_BASE, GPIO_PIN_6);    // PB6 como salida PWM 
    GPIOPinConfigure(GPIO_PB6_M0PWM0);              // del módulo PWM0


    pwm_clk = SysCtlClockGet() / 64;  // 64, ya que es el factor de división del reloj de PWM respecto al reloj del sistema
    val_load = (pwm_clk / PWM_BASE_FREQ) - 1;   //Valor del contador para la freq. pwm. En este caso --> 625Khz/100 = 6.25KHz
    PWMGenConfigure(PWM0_BASE, PWM_GEN_0, PWM_GEN_MODE_DOWN);   // Módulo PWM contara hacia abajo
    PWMGenPeriodSet(PWM0_BASE, PWM_GEN_0, val_load);    // Carga la cuenta que establece la frecuencia de la señal PWM

    PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, duty*val_load/100);  // Establece el periodo (en este caso, un porcentaje del valor máximo)
    PWMOutputState(PWM0_BASE, PWM_OUT_0_BIT, true); // Habilita la salida de la señal
    PWMGenEnable(PWM0_BASE, PWM_GEN_0); //Habilita/pone en marcha el generador PWM

    while(1)
    {
        valor=duty*val_load/100;
        PWMPulseWidthSet(PWM0_BASE, PWM_OUT_0, valor);
        if (duty == 100)
        {
            down = 1;
            up = 0;
        }
        if (duty == 1)
        {
            up = 1;
            down = 0;
        }
        if (up)
            duty += 1;
        if (down)
            duty -= 1;
        for(i = 0; i< 40000; i++);
    }
}
