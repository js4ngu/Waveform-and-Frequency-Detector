#include "stm32f767xx.h"
#include "OK-STM767.h"
#include <math.h>
#define sampling_rate 36000
#define pi 3.141592

unsigned short result;
float temperature;

unsigned short voltage_origin;
unsigned short voltage_origin_max = 0;
unsigned short voltage_origin_pow;
unsigned short voltage_origin_sum = 0;
float rms;
unsigned short frq = 0;

float V_sin;
float V_tr;
float V_sw;
float err[3];
float err_min;
unsigned short wave;


float abs(float n) {
    if(n<0) return -n;
    else return n;
}

int main(void) {
    Initialize_MCU();				// initialize MCU and kit
    Delay_ms(50);					// wait for system stabilization
    Initialize_LCD();				// initialize text LCD module
    Initialize_TFT_LCD();				// initialize TFT-LCD module

    Beep();

    TFT_string(8,12,White,Black, "ADC software start test");
    TFT_string(8,16,White,Black,"ADC3 (LM35DZ) = +00.0 C");
    TFT_string(8, 20,White,Black,"ADC3 (J4)     = +00.0 V");

    GPIOA->MODER |= 0x0000F0FF;           //ADC12_IN6, ADC12_IN7 실행
    RCC -> APB2ENR |= 0x00000101;          // ADC1 실행, TIMER 실행

    ADC->CCR = 0x08020000;                  // ADCCLK = 54MHZ / 6  = 9 MHz , 온도센서 출력 Vsense 허용
    ADC1-> SMPR2 = 0x00180000;             //채널 6,7의 샘플링 시간 = A/C 컨버터 클록의 144주기

    ADC1-> CR1 = 0x00000000;                 //12bit 분해능 0~4095
    ADC1 -> CR2 = 0x10000001;                 // trigger신호의 상승엣지 검출,
    ADC1 -> SQR1 = 0x00000001;            // 전체 regular channel수 3개  *************2개 수정


    while(1) {
        ADC1->SQR3 = 0x00000003;			// channel 3 (LM35DZ)
        ADC1->CR2 |= 0x40000000;			// start conversion by software
        while(!(ADC1->SR & 0x00000002));		// wait for end of conversion
        result = ADC1->DR;

        TFT_xy(24,16);
        TFT_color(Cyan,Black);
        TFT_signed_float((float)result*330./4095., 2, 1);                  // display temperature

        for (int i = 0; i < sampling_rate; i++) {
            ADC1->SQR3 = 0x00000006;                   //channel 6 (J4)
            ADC1->CR2 |= 0x40000000;			// start conversion by software
            while(!(ADC1->SR & 0x00000002));		// wait for end of conversion

            voltage_origin = ADC1->DR;
            voltage_origin_pow = voltage_origin * voltage_origin; //rms value계산을 위해 제곱
            voltage_origin_sum = voltage_origin_sum + voltage_origin_pow; //rms value 계산을 위해 summation

            if (voltage_origin_max <= voltage_origin){
                voltage_origin_max = voltage_origin; //최대값 검출
            }

            if (voltage_origin == 0) frq++; // 주파수 검출 -> 0v 인가한다해도 0이 안뜰 수 있음, 이건 실제 gnd인가하여 피팅 필요함
        }

        rms = sqrt(voltage_origin_sum/sampling_rate);
        V_sin = voltage_origin_max / 1.414;
        V_tr = voltage_origin_max / 1.732;
        V_sw = voltage_origin_max;

        err[0] = abs(rms-V_sin);
        err[1] = abs(rms-V_tr);
        err[2] = abs(rms-V_sw);

        err_min = err[0];
        for (int i = 0; i < 3; i++) {
            if(err[i] <= err_min){
                err_min = err[i];
                wave = i;
            }
        }

        switch (wave) {
            case 0 :
                printf("sin : %f\n", err[0]);
                break;
            case 1 :
                printf("triangle : %f\n", err[1]);
                break;
            case 2 :
                printf("square : %f\n", err[2]);
                break;
            default:
                break;
        }
        LED_toggle();				// blink LED1
        Delay_ms(200);
    }
}