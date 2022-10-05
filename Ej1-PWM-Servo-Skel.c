/* ************************************************************************** */
/* UNIVERSIDAD DE MALAGA               DEPARTAMENTO DE TECNOLOGIA ELECTRONICA */
/* http://www.uma.es                                    http://www.dte.uma.es */
/* ========================================================================== */
/* PROGRAMA :  PWM-Servo                                                      */
/* VERSION  : 1.0                                                             */
/* TARGET   : Kit  TIVA Launchpad IDE CCSv7                                   */
/* DESCRIPCION : Este programas genera dos salidas PWM a través de los        */
/* terminales PF2 y PF3 usando el Timer adecuado en modo PWM, o un modulo PWM */
/*  Al pulsar los botones de la placa, deberña aumentar/reducir el ciclo de   */
/*  trabajo, provocando un aumento/reducción de la velocidad e incluso cambio */
/*  sentido                                                                   */
/* RECURSOS :                                                                 */
/* AUTOR    : Ignacio Herrero Reder (iherrero@uma.es)                         */
/* FECHA    : 08/10/17                                                        */
/* COMENTARIOS  : 1 tabulador = 8 espacios                                    */
                
/* **************************************************************************	*/
#include <stdint.h>
#include <stdbool.h>
// Librerias que se incluyen tipicamente para configuracion de perifericos y pinout
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h" 
// Libreria de control del sistema
#include "driverlib/sysctl.h"
// Incluir librerias de periférico y otros que se vaya a usar para control PWM y gestión
// de botones  (TODO)

/* **************************************************************************	*/
/* (ver cabecera en el código del programa)
 */
#include "driverlib/gpio.h" // NHR: Libreria GPIO
#include "driverlib/pwm.h"  // NHR: Libreria PWM
#include "drivers/buttons.h" // NHR
#include "driverlib/interrupt.h" // NHR
#include "inc/hw_ints.h" // NHR
//#include "main.h"
/* **************************************************************************	*/
#include <stdint.h>
#include <stdbool.h>
// Librerias que se incluyen tipicamente para configuracion de perifericos y pinout
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/pin_map.h" 
// Libreria de control del sistema
#include "driverlib/sysctl.h"
// Incluir librerias de periférico y otros que se vaya a usar para control PWM y gestión
// de botones  (TODO)
#define DUTY_MAX 100
#define DUTY_MIN 60
#define PERIOD_PWM 50	// TODO: Ciclos de reloj para conseguir una señal periódica de 50Hz (según reloj de periférico usado)
#define COUNT_1MS XXXX   // TODO: Ciclos para amplitud de pulso de 1ms (max velocidad en un sentido)
#define STOPCOUNT 1520  // TODO: Ciclos para amplitud de pulso de parada (1.52ms)
#define COUNT_2MS ZZZZ*/   // TODO: Ciclos para amplitud de pulso de 2ms (max velocidad en el otro sentido)
#define NUM_STEPS 50    // Pasos para cambiar entre el pulso de 2ms al de 1ms
#define CYCLE_ INCREMENTS (abs(COUNT_1MS-COUNT_2MS))/NUM_STEPS  // Variacion de amplitud tras pulsacion
extern void GPIOFIntHandler(void);
uint32_t ui32Period, ui32DutyCycle;
uint16_t duty1 = 90,duty2 = 70;
//uint16_t duty = 8;
int main(void){


    //uint32_t ui32Period, ui32DutyCycle;
    int x=0;
    uint32_t val_load, pwm_clk;
    static uint32_t valor1,valor2;
    //static uint32_t valor;

    uint8_t up=1, down=0;
    uint16_t i = 0;
    // Elegir reloj adecuado para los valores de ciclos sean de tamaño soportable
    SysCtlClockSet(SYSCTL_SYSDIV_5|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|SYSCTL_OSC_MAIN);
    SysCtlPWMClockSet(SYSCTL_PWMDIV_64);
  // Configura pulsadores placa TIVA (int. por flanco de bajada)
    ButtonsInit();
    GPIOIntTypeSet(GPIO_PORTF_BASE, ALL_BUTTONS,GPIO_RISING_EDGE);
    GPIOIntEnable(GPIO_PORTF_BASE,ALL_BUTTONS);
    IntEnable(INT_GPIOF);
  // Configuracion  ondas PWM: frecuencia 50Hz, anchura inicial= valor STOPCOUNT, 1540us para salida por PF2, y COUNT_1MS (o COUNT_2MS ) para salida por PF3(puedes ponerlo inicialmente a  PERIOD_PWM/10)
  	// Opcion 1: Usar un Timer en modo PWM (ojo! Los timers PWM solo soportan cuentas 
      //  de 16 bits, a menos que uséis un prescaler/timer extension)
  	// Opcion 2: Usar un módulo PWM(no dado en Sist. Empotrados pero mas sencillo)


    SysCtlPeripheralEnable(SYSCTL_PERIPH_PWM1); //Habilita modulo PWM
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);    // Habilita puerto salida para se�al PWM (ver en documentacion que pin se corresponde a cada m�dulo PWM)
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_3);
    GPIOPinTypePWM(GPIO_PORTF_BASE, GPIO_PIN_2);
    GPIOPinConfigure(GPIO_PF3_M1PWM7);
    GPIOPinConfigure(GPIO_PF2_M1PWM6);


    pwm_clk = SysCtlClockGet() / 64;
    val_load = (pwm_clk/ PERIOD_PWM) - 1;

    PWMGenConfigure(PWM1_BASE, PWM_GEN_3, PWM_GEN_MODE_DOWN);   // M�dulo PWM contara hacia abajo
    PWMGenPeriodSet(PWM1_BASE, PWM_GEN_3, val_load);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, duty1*val_load/100);
    PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, duty2*val_load/100);// Establece el periodo (en este caso, un porcentaje del valor m�ximo)
    PWMOutputState(PWM1_BASE, PWM_OUT_7_BIT, true);
    PWMOutputState(PWM1_BASE, PWM_OUT_6_BIT, true);// Habilita la salida de la se�al
    PWMGenEnable(PWM1_BASE, PWM_GEN_3); //Habilita/pone en marcha el generador PWM
    // Opcion 1: Usar un Wide Timer (32bits) en modo PWM (estos timers soportan
      //  cuentas de 32 bits, pero tendréis que sacar las señales de control pwm por
      //  otros pines distintos de PF2 y PF3)
    while(1)
        {
            valor1=duty1*val_load/1000;
            valor2=duty2*val_load/1000;
            //valor=duty*val_load/100;
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_7, valor1);
            PWMPulseWidthSet(PWM1_BASE, PWM_OUT_6, valor2);
            /*if (duty == 100)
            {
                down = 1;
                up = 0;
            }
            if (duty == 50)
            {
                up = 1;
                down = 0;
            }
            if (up)
                duty += 1;
            if (down)
                duty -= 1;*/
            for(i = 0; i< 40000; i++);
        }

  // Codigo principal, (poner en bucle infinito o bajo consumo)
}
void GPIOFIntHandler(void)
{
    int32_t i32Status = GPIOIntStatus(GPIO_PORTF_BASE,ALL_BUTTONS);
    // Boton Izquierdo: reduce ciclo de trabajo en CYCLE_INCREMENTS para el servo conectado a PF4, hasta llegar a MINCOUNT
    if(((i32Status & LEFT_BUTTON) == LEFT_BUTTON)){

        if (duty1<DUTY_MAX)
        duty1++;


    }
    // Boton Derecho: aumenta ciclo de trabajo en CYCLE_INCREMENTS para el servo conectado a PF4, hasta llegar a MAXCOUNT
    if(((i32Status & RIGHT_BUTTON) == RIGHT_BUTTON)){

        if (duty1>DUTY_MIN)
        duty1--;


    }
    GPIOIntClear(GPIO_PORTF_BASE,ALL_BUTTONS);  //limpiamos flags
}
// Rutinas de interrupción de pulsadores
// Boton Izquierdo: modifica  ciclo de trabajo en CYCLE_INCREMENTS para el servo conectado a PF2, hasta llegar a  COUNT_1MS
// Boton Derecho: modifica  ciclo de trabajo en CYCLE_INCREMENTS para el servo conectado a PF2, hasta llegar a COUNT_2MS
