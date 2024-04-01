#include <ch32v00x.h>
#include <ch32v00x_adc.h>
#include <ch32v00x_rcc.h>
#include "vbat.h"

__attribute__((constructor))
static void vbat_init(void) {
    RCC_ADCCLKConfig(RCC_PCLK2_Div128);
    ADC_DeInit(ADC1);

    ADC_InitTypeDef adc = {
        .ADC_Mode               = ADC_Mode_Independent,
        .ADC_ScanConvMode       = ENABLE,
        .ADC_ContinuousConvMode = ENABLE,
        .ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None,
        .ADC_DataAlign          = ADC_DataAlign_Right,
        .ADC_NbrOfChannel       = 1,
    };
    ADC_Init(ADC1, &adc);

    ADC_RegularChannelConfig(ADC1, ADC_Channel_Vrefint, 1, ADC_SampleTime_241Cycles);
    ADC_Calibration_Vol(ADC1, ADC_CALVOL_50PERCENT);
    ADC_Cmd(ADC1, ENABLE);

    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1));
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1));

    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
}

uint16_t vbat_read(void) {
    return 1200 * 1024 / ADC_GetConversionValue(ADC1);
}
