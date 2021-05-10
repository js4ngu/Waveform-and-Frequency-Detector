#include <stdio.h>
#include <math.h>
#define sampling_rate 63000
#define pi 3.141592

float voltage_origin;
float voltage_origin_max = 0;
float voltage_origin_pow;
float voltage_origin_sum = 0;
float rms;
int frq = 0;

float V_sin;
float V_tr;
float V_sw;
float err[3];
float err_min;
int wave;

float dc_sig(float amp){
    return amp;
}

float sin_sig(float amp, int frq, int i){
    return amp * sin(2*pi*frq*i/62950);
}


float triangle_sig(float amp, float frq, int adc) {
    float one_vibration_adc = 62950 / frq;
    int vibration_count = floor(adc / one_vibration_adc);
    float one_vibration_progress = adc - vibration_count * one_vibration_adc;
    float half_vibration_adc = one_vibration_adc / 2;

    if (one_vibration_progress < half_vibration_adc) {
        return amp * one_vibration_progress / half_vibration_adc;
    }
    else {
        return amp * (1 - (one_vibration_progress - half_vibration_adc) / half_vibration_adc);
    }
}


int main(void) {
    for (int i = 0; i < sampling_rate; i++) {
        //voltage_origin = dc_sig(5);
        //voltage_origin = sin_sig(3.3,1000,i);
        voltage_origin = triangle_sig(3.3, 100, i);

        voltage_origin_pow = voltage_origin * voltage_origin; //rms value계산을 위해 제곱
        voltage_origin_sum = voltage_origin_sum + voltage_origin_pow; //rms value 계산을 위해 summation

        if (voltage_origin_max <= voltage_origin){
            voltage_origin_max = voltage_origin; //최대값 검출
        }

        if (voltage_origin <= 0.01) frq++; // 주파수 검출 -> 0v 인가한다해도 0이 안뜰 수 있음, 이건 실제 gnd인가하여 피팅 필요함
    }

    rms = sqrt(voltage_origin_sum/sampling_rate);

    V_sin = voltage_origin_max / 1.414;
    V_tr = voltage_origin_max / 1.732;
    V_sw = voltage_origin_max;

    err[0] = fabs(rms-V_sin);
    err[1] = fabs(rms-V_tr);
    err[2] = fabs(rms-V_sw);

    err_min = err[0];
    for (int i = 0; i < 3; i++) {
        if(err[i] <= err_min){
            err_min = err[i];
            wave = i;
        }
    }

    printf("-----\n");
    printf("result\n\n");
    printf("voltage_origin_max : %f\n", voltage_origin_max);
    printf("frq : %d\n", frq/2);
    printf("RMS : %f\n\n", rms);

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
}