/***************************************************************************//**
 * @brief
 * @file    anasens.c
 * @author  Eric Costa Hall
 * @date    26/10/2021
 * @details
 * @section license
 *          FAE Sistemas de Medição. All rights reserved.
 ******************************************************************************/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "general_settings.h"
#include "anasens.h"
#include "IQmathLib.h"
#include "adc12_b.h"
#include "ref_a.h"

#define BATT_V_SENSE_PORT                       GPIO_PORT_P2
#define BATT_V_SENSE_PIN                        GPIO_PIN3
#define BATT_V_ENABLE_PIN                       GPIO_PIN2

#if DEBUG_ANASENS
#include "serial.h"
#include "systick.h"
#include <stdio.h>
#endif

/*******************************************************************************
 * Defines
 ******************************************************************************/
#define ANASENSE_ADC12IFGR0_0_MASK                  0x0001
#define ANASENSE_ADC12IFGR0_1_MASK                  0x0002

#define ANASENSE_LIGHT_SENSE_MASK                   ANASENSE_ADC12IFGR0_0_MASK
#define ANASENSE_VCC_SENSE_MASK                     ANASENSE_ADC12IFGR0_1_MASK


#define ANASENSE_LIGHT_SENSE_MEM_MAP                ADC12_B_MEMORY_0
#define ANASENSE_VCC_SENSE_MEM_MAP                  ADC12_B_MEMORY_1

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static uint32_t getLightSenseVoltage(void);
static uint32_t getVccSenseVoltage(void);
static uint32_t convertAdcToIQ19V(uint16_t ui_input);
static void disableAdc(void);
static void enableAdc(void);

/*******************************************************************************
 * Global variables
 ******************************************************************************/
static uint32_t giq19_opticalInputVoltage;
static uint32_t giq19_batteryInputValue;

/*******************************************************************************
 * Public functions
 ******************************************************************************/
/**
 * @brief
 *      Initialize module.
 */
void anasens_init(void)
{

    ADC12_B_initParam initParam = {0};
    ADC12_B_configureMemoryParam configureMemoryParam = {0};



    //If ref generator busy, WAIT
    while(Ref_A_isRefGenBusy(REF_A_BASE));

    //Select internal ref = 2.5V
    Ref_A_setReferenceVoltage(REF_A_BASE, REF_A_VREF2_5V);

    //Initialize the ADC12B Module
    /*
     * Base address of ADC12B Module
     * Use internal ADC12B bit as sample/hold signal to start conversion
     * USE MODOSC 5MHZ Digital Oscillator as clock source
     * Use default clock divider/pre-divider of 1
     * Not use internal channel
     */
    initParam.sampleHoldSignalSourceSelect = ADC12_B_SAMPLEHOLDSOURCE_SC;
    initParam.clockSourceSelect = ADC12_B_CLOCKSOURCE_ADC12OSC;
    initParam.clockSourceDivider = ADC12_B_CLOCKDIVIDER_1;
    initParam.clockSourcePredivider = ADC12_B_CLOCKPREDIVIDER__1;
    initParam.internalChannelMap = ADC12_B_NOINTCH;

    // Init ADC12_B configuration
    ADC12_B_init(ADC12_B_BASE, &initParam);

    /* Enables ADC12_B */
    ADC12_B_enable(ADC12_B_BASE);
    /*
     * Base address of ADC12B Module
     * For memory buffers 0-7 sample/hold for 4 clock cycles (47us-64us using MODOSC)
     * For memory buffers 8-15 sample/hold for 64 clock cycles (5.9us - 8us using MODOSC)
     * Disable Multiple Sampling
     */
    ADC12_B_setupSamplingTimer(ADC12_B_BASE, ADC12_B_CYCLEHOLD_4_CYCLES,
            ADC12_B_CYCLEHOLD_4_CYCLES, ADC12_B_MULTIPLESAMPLESENABLE);

    //Configure Memory Buffer
    /*
     * Base address of the ADC12B Module
     * Configure memory buffer 0
     * Map input A0 (Light Sensor) to memory buffer 0
     * Vref+ = IntBuffer
     * Vref- = AVss
     */
    configureMemoryParam.memoryBufferControlIndex = ANASENSE_LIGHT_SENSE_MEM_MAP;
    configureMemoryParam.inputSourceSelect = ADC12_B_INPUT_A0;
    configureMemoryParam.refVoltageSourceSelect =
                                            ADC12_B_VREFPOS_INTBUF_VREFNEG_VSS;
    configureMemoryParam.endOfSequence = ADC12_B_ENDOFSEQUENCE;
    configureMemoryParam.windowComparatorSelect =
                                            ADC12_B_WINDOW_COMPARATOR_DISABLE;
    configureMemoryParam.differentialModeSelect =
                                            ADC12_B_DIFFERENTIAL_MODE_DISABLE;
    ADC12_B_configureMemory(ADC12_B_BASE, &configureMemoryParam);

    /*
     * Base address of the ADC12B Module
     * Configure memory buffer 8
     * input A15 (external voltage divider) to memory buffer 8
     * Vref+ = IntBuffer
     * Vref- = AVss
     */
    configureMemoryParam.memoryBufferControlIndex = ANASENSE_VCC_SENSE_MEM_MAP;
    configureMemoryParam.inputSourceSelect = ADC12_B_INPUT_A15;
    configureMemoryParam.refVoltageSourceSelect =
                                            ADC12_B_VREFPOS_INTBUF_VREFNEG_VSS;
    configureMemoryParam.endOfSequence = ADC12_B_ENDOFSEQUENCE;
    configureMemoryParam.windowComparatorSelect =
                                            ADC12_B_WINDOW_COMPARATOR_DISABLE;
    configureMemoryParam.differentialModeSelect =
                                            ADC12_B_DIFFERENTIAL_MODE_DISABLE;
    ADC12_B_configureMemory(ADC12_B_BASE, &configureMemoryParam);


    // Disable ADC peripheral
    disableAdc();

    giq19_opticalInputVoltage = 0;
    giq19_batteryInputValue = 0;
}

/**
 * @brief
 *      Start an ADC conversion.
 */
void anasens_startConversion(void)
{
    // Enable ADC and wait for stabilization
    enableAdc();

    /*
     * Base address of ADC12B Module
     * For memory buffers 0-7 sample/hold for 4 clock cycles (47us-64us using MODOSC)
     * For memory buffers 8-15 sample/hold for 64 clock cycles (5.9us - 8us using MODOSC)
     * Disable Multiple Sampling
     */
    ADC12_B_setupSamplingTimer(ADC12_B_BASE, ADC12_B_CYCLEHOLD_4_CYCLES,
                               ADC12_B_CYCLEHOLD_4_CYCLES, ADC12_B_MULTIPLESAMPLESDISABLE);

    /*
     * Base address of ADC12B Module
     * Start the conversion into memory buffer 0
     * Use the single-channel, single-conversion mode
     */
    ADC12_B_startConversion(ADC12_B_BASE, ADC12_B_START_AT_ADC12MEM0,
                            ADC12_B_SINGLECHANNEL);


}

/**
 * @brief
 *      Update enabled channels.
 */
void anasens_update(void)
{
    while(!adc_light_meas_wait());

    ADC12_B_setupSamplingTimer(ADC12_B_BASE, ADC12_B_CYCLEHOLD_64_CYCLES,
                               ADC12_B_CYCLEHOLD_64_CYCLES, ADC12_B_MULTIPLESAMPLESDISABLE);

    ADC12_B_startConversion(ADC12_B_BASE, ADC12_B_START_AT_ADC12MEM1,
                            ADC12_B_SINGLECHANNEL);

    // Get converted voltage value
    giq19_opticalInputVoltage = getLightSenseVoltage();

    while(!adc_vcc_meas_wait());

    // Convert ADC reading to voltage
    giq19_batteryInputValue = getVccSenseVoltage();


#if DEBUG_ANASENS
#include <stdio.h>

    uint16_t adc_mv = ADC12_B_getResults(ADC12_B_BASE,
                     ANASENSE_LIGHT_SENSE_MEM_MAP);
    printf("(ANASENS): Optical Input: %uV\n", adc_mv);

    adc_mv = ADC12_B_getResults(ADC12_B_BASE,
                        ANASENSE_VCC_SENSE_MEM_MAP);
    printf("(ANASENS): Battery Input: %uV\n", giq19_batteryInputValue);

#endif

    // Disable ADC
    disableAdc();

}
/**
 * @brief
 *      Get optical channel voltage measurement.
 *
 * @return
 *      Returns optical channel voltage measurement.
 */
uint32_t anasens_getOpticalInputVoltage(void)
{
    return giq19_opticalInputVoltage;
}

/**
 * @brief
 *      Get battery channel voltage measurement in milivolts.
 *
 * @return
 *      Returns battery channel voltage measurement.
 */
uint32_t anasens_getBatteryInputVoltage(void)
{
    return giq19_batteryInputValue;
}

static uint32_t getLightSenseVoltage(void)
{
    uint16_t adc_mv = ADC12_B_getResults(ADC12_B_BASE,
            ANASENSE_LIGHT_SENSE_MEM_MAP);

    return convertAdcToIQ19V(adc_mv);
}

static uint32_t getVccSenseVoltage(void)
{
    uint16_t adc_mv = ADC12_B_getResults(ADC12_B_BASE,
            ANASENSE_VCC_SENSE_MEM_MAP);

    return adc_mv;
}

bool adc_light_meas_wait(void)
{
    return (ADC12IFGR0 & ANASENSE_LIGHT_SENSE_MASK);
}

bool adc_vcc_meas_wait(void)
{
    return (ADC12IFGR0 & ANASENSE_VCC_SENSE_MASK);
}

static uint32_t convertAdcToIQ19V(uint16_t ui_input)
{
    uint32_t iq19_voltage = _IQ19(ui_input);

    return iq19_voltage;
}

static void enableAdc(void)
{
    //Enable ADC12 module; takes 100ns to stabilize
    ADC12_B_enable(ADC12_B_BASE);

    //Turn on Reference Voltage and wait
    Ref_A_enableReferenceVoltage(REF_A_BASE);
    while(Ref_A_isVariableReferenceVoltageOutputReady(REF_A_BASE) != REF_A_READY);
}

static void disableAdc(void)
{
    //Turn off Reference Voltage
    Ref_A_disableReferenceVoltage(REF_A_BASE);

    // Disable ADC12 module
    ADC12_B_disable(ADC12_B_BASE);
}

