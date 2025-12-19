/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdlib.h>
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define UART_TRANSMISSION_INTERVAL_MS 1000   // UART送信周期[ms]
#define MAX_SAMPLES_PER_INTERVAL      100    // 平均化に使う最大サンプル数
#define VREF_MV                       3300
#define ADC_MAX_VALUE                 4095

#define BUTTON_DEBOUNCE_MS            200

// フィードバック抵抗[Ω]（実機に合わせて変更）
#define R_FEEDBACK_OHM                88000.0f  // 88kΩ

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;
__IO uint32_t BspButtonState = BUTTON_RELEASED;

ADC_HandleTypeDef hadc1;
ADC_HandleTypeDef hadc2;

DAC_HandleTypeDef hdac1;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart5;

/* USER CODE BEGIN PV */

// UART & 平均化関連
static uint32_t last_uart_time      = 0;
static uint32_t adc_count_for_uart  = 0;
static float    current_sum_for_uart = 0.0f;
static char     uart_output[64];

// DAC電圧レベル (0, 5, 10, 15, 20, 30 mV)
// code = round(mV * 4095 / 3300)
const uint32_t dac_voltage_levels[] = {
    0,   // 0 mV
    6,   // 5 mV
    12,  // 10 mV
    19,  // 15 mV
    25,  // 20 mV
    37   // 30 mV
};
const uint8_t num_voltage_levels = sizeof(dac_voltage_levels) / sizeof(dac_voltage_levels[0]);

volatile uint8_t  current_voltage_index = 0;
volatile uint32_t dac_value             = 0;
static  uint32_t  adc_value             = 0;

// ボタン関連
volatile uint8_t button_request_change = 0;   // 割り込みからのイベントフラグ
static  uint32_t last_button_time      = 0;
static  uint16_t button_press_count    = 0;
static  uint8_t  measurement_paused    = 0;   // 0: 測定中, 1: 一時停止中

// ADS1299 周辺（必要ならそのまま残す）
extern SPI_HandleTypeDef hspi2;
#define ADS_SPI_HANDLE &hspi2

#define ADS_CS_PORT     ADC_CS_GPIO_Port
#define ADS_CS_PIN      ADC_CS_Pin
#define ADS_RESET_PORT  ADC_RESET_GPIO_Port
#define ADS_RESET_PIN   ADC_RESET_Pin
#define ADS_DRDY_PORT   ADC_DRDY_GPIO_Port
#define ADS_DRDY_PIN    ADC_DRDY_Pin

extern UART_HandleTypeDef huart5;
#define ADS_UART_HANDLE &huart5

// ADS1299 コマンド
#define CMD_WAKEUP    0x02
#define CMD_STANDBY   0x04
#define CMD_RESET     0x06
#define CMD_START     0x08
#define CMD_STOP      0x0A
#define CMD_RDATAC    0x10
#define CMD_SDATAC    0x11
#define CMD_RDATA     0x12
#define CMD_RREG      0x20
#define CMD_WREG      0x40

// ADS1299 レジスタアドレス
#define REG_ID        0x00
#define REG_CONFIG1   0x01
#define REG_CONFIG2   0x02
#define REG_CONFIG3   0x03
#define REG_LOFF      0x04
#define REG_CH1SET    0x05
#define REG_CH2SET    0x06
#define REG_CH3SET    0x07
#define REG_CH4SET    0x08
#define REG_CH5SET    0x09
#define REG_CH6SET    0x0A
#define REG_CH7SET    0x0B
#define REG_CH8SET    0x0C
#define REG_BIAS_SENSP 0x0D
#define REG_BIAS_SENSN 0x0E
#define REG_LOFF_SENSP 0x0F
#define REG_LOFF_SENSN 0x10
#define REG_LOFF_FLIP  0x11
#define REG_LOFF_STATP 0x12
#define REG_LOFF_STATN 0x13
#define REG_GPIO       0x14
#define REG_MISC1      0x15
#define REG_MISC2      0x16
#define REG_CONFIG4    0x17

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_UART5_Init(void);
static void MX_TIM3_Init(void);
static void MX_SPI2_Init(void);
static void MX_DAC1_Init(void);
static void MX_ADC1_Init(void);
static void MX_ADC2_Init(void);
/* USER CODE BEGIN PFP */

static void ads_send_command(uint8_t cmd);
static void ads_write_reg(uint8_t addr, uint8_t data);
static uint8_t ads_read_reg(uint8_t addr);
static void ads_init(void);
static int32_t ads_read_ch1_data(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/* USER CODE END 0 */

int main(void)
{
  HAL_Init();
  SystemClock_Config();

  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_UART5_Init();
  MX_TIM3_Init();
  MX_SPI2_Init();
  MX_DAC1_Init();
  MX_ADC1_Init();
  MX_ADC2_Init();

  /* USER ボタン & COM 初期化 */
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* DACスタート */
  if (HAL_DAC_Start(&hdac1, DAC_CHANNEL_1) != HAL_OK)
  {
    printf("DAC Start Error!\r\n");
    Error_Handler();
  }

  /* ADCキャリブレーション */
  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK)
  {
    printf("ADC Calibration Error!\r\n");
    Error_Handler();
  }

  /* 初期DAC値 (0mV) */
  current_voltage_index = 0;
  dac_value = dac_voltage_levels[current_voltage_index];
  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_value);

  printf("System ready. Initial DAC index=%d (0 mV)\r\n", current_voltage_index);

  /* Infinite loop */
  while (1)
  {
    /* ========= ボタン処理 =========
       1回押す   : 測定一時停止
       もう一回  : 測定再開 & DACレベル変更
    ================================= */
    if (button_request_change == 1)
    {
      uint32_t now = HAL_GetTick();

      if ((now - last_button_time) > BUTTON_DEBOUNCE_MS)
      {
        last_button_time = now;
        button_request_change = 0;

        button_press_count++;

        if (measurement_paused == 0)
        {
          measurement_paused = 1;
          printf("Measurement PAUSED (press count: %u)\r\n", button_press_count);
        }
        else
        {
          measurement_paused = 0;

          current_voltage_index = (current_voltage_index + 1) % num_voltage_levels;
          dac_value = dac_voltage_levels[current_voltage_index];

          uint32_t dac_mv = (dac_value * VREF_MV) / ADC_MAX_VALUE;
          printf("Measurement RESUMED. New DAC index=%d, approx %lu mV (press count: %u)\r\n",
                 current_voltage_index, dac_mv, button_press_count);
        }
      }
      else
      {
        // デバウンス期間中なら捨てる
        button_request_change = 0;
      }

      BspButtonState = BUTTON_RELEASED;
    }

    /* DAC出力（常に反映） */
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_value);

    /* 測定一時停止中なら ADC や UART はやらず待機 */
    if (measurement_paused)
    {
      HAL_Delay(50);
      continue;
    }

    /* ========= ADC測定 (平均化) ========= */
    HAL_Delay(5);  // DAC安定用

    uint32_t adc_sum        = 0;
    uint8_t  valid_samples  = 0;
    const uint8_t num_samples = 16;

    for (uint8_t i = 0; i < num_samples; i++)
    {
      if (HAL_ADC_Start(&hadc1) != HAL_OK)
      {
        printf("ADC Start Error!\r\n");
        continue;
      }

      if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
      {
        uint32_t reading = HAL_ADC_GetValue(&hadc1);
        adc_sum += reading;
        valid_samples++;
      }
      HAL_ADC_Stop(&hadc1);
      HAL_Delay(2);
    }

    if (valid_samples > 0)
    {
      adc_value = adc_sum / valid_samples;
    }

    /* ========= 電圧・電流計算 ========= */
    uint32_t adc_voltage_mv = (adc_value * VREF_MV) / ADC_MAX_VALUE;
    float    adc_voltage_v  = adc_voltage_mv / 1000.0f;

    // ※ ここが「V_dac - V_adc」から「V_adc」のみに変更されているポイント
    float current_ua = adc_voltage_v / R_FEEDBACK_OHM * 1e6f;

    /* ========= UART用平均化 ========= */
    if (adc_count_for_uart < MAX_SAMPLES_PER_INTERVAL)
    {
      current_sum_for_uart += current_ua;
      adc_count_for_uart++;
    }

    uint32_t now = HAL_GetTick();

    if ((now >= last_uart_time &&
         (now - last_uart_time) >= UART_TRANSMISSION_INTERVAL_MS) ||
        (now < last_uart_time &&
         (now + (0xFFFFFFFFu - last_uart_time)) >= UART_TRANSMISSION_INTERVAL_MS))
    {
      float avg_current_ua = (adc_count_for_uart > 0)
                             ? (current_sum_for_uart / adc_count_for_uart)
                             : current_ua;

      uint32_t dac_voltage_mv = (dac_value * VREF_MV) / ADC_MAX_VALUE;
      float    dac_voltage_v  = dac_voltage_mv / 1000.0f;

      snprintf(uart_output, sizeof(uart_output),
               "%.4f , %.3f\r\n", dac_voltage_v, avg_current_ua);
      printf("%s", uart_output);

      current_sum_for_uart = 0.0f;
      adc_count_for_uart   = 0;
      last_uart_time       = now;
    }

    HAL_Delay(50);
  }
}

/* ===================== 以下は初期化関数 ===================== */

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  if (HAL_RCCEx_EpodBoosterClkConfig(RCC_EPODBOOSTER_SOURCE_MSIS, RCC_EPODBOOSTER_DIV1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_PWREx_EnableEpodBooster() != HAL_OK)
  {
    Error_Handler();
  }

  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_MSIS;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.LSIDiv = RCC_LSI_DIV1;
  RCC_OscInitStruct.MSISState = RCC_MSI_ON;
  RCC_OscInitStruct.MSISSource = RCC_MSI_RC0;
  RCC_OscInitStruct.MSISDiv = RCC_MSI_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_PCLK3;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSIS;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }

  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_SYSCLK, RCC_MCODIV_64);
}

static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.GainCompensation = 0;
  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc1.Init.LowPowerAutoWait = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc1.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc1.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc1.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  // 電流計算用に接続しているピンに対応するチャネルにすること
  sConfig.Channel = ADC_CHANNEL_1;  // 例: PA0 の場合
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_ADC2_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc2.Instance = ADC2;
  hadc2.Init.Resolution = ADC_RESOLUTION_12B;
  hadc2.Init.GainCompensation = 0;
  hadc2.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc2.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc2.Init.LowPowerAutoWait = DISABLE;
  hadc2.Init.ContinuousConvMode = DISABLE;
  hadc2.Init.NbrOfConversion = 1;
  hadc2.Init.DiscontinuousConvMode = DISABLE;
  hadc2.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc2.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc2.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc2.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc2.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc2.Init.OversamplingMode = DISABLE;
  if (HAL_ADC_Init(&hadc2) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_DAC1_Init(void)
{
  DAC_ChannelConfTypeDef sConfig = {0};

  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  // 最低限の設定：トリガ無し、バッファ有効、外部ピンに出力
  sConfig.DAC_Trigger              = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer         = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_EXTERNAL;
  sConfig.DAC_SampleAndHold        = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_UserTrimming         = DAC_TRIMMING_FACTORY;

  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_SPI1_Init(void)
{
  SPI_AutonomousModeConfTypeDef HAL_SPI_AutonomousMode_Cfg_Struct = {0};

  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES_TXONLY;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_64;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 0x7;
  hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
  hspi1.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi1.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi1.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi1.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi1.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi1.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi1.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  hspi1.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
  hspi1.Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerState = SPI_AUTO_MODE_DISABLE;
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerSelection = SPI_GRP1_GPDMA_CH0_TCF_TRG;
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerPolarity = SPI_TRIG_POLARITY_RISING;
  if (HAL_SPIEx_SetConfigAutonomousMode(&hspi1, &HAL_SPI_AutonomousMode_Cfg_Struct) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_SPI2_Init(void)
{
  SPI_AutonomousModeConfTypeDef HAL_SPI_AutonomousMode_Cfg_Struct = {0};

  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 0x7;
  hspi2.Init.NSSPMode = SPI_NSS_PULSE_DISABLE;
  hspi2.Init.NSSPolarity = SPI_NSS_POLARITY_LOW;
  hspi2.Init.FifoThreshold = SPI_FIFO_THRESHOLD_01DATA;
  hspi2.Init.MasterSSIdleness = SPI_MASTER_SS_IDLENESS_00CYCLE;
  hspi2.Init.MasterInterDataIdleness = SPI_MASTER_INTERDATA_IDLENESS_00CYCLE;
  hspi2.Init.MasterReceiverAutoSusp = SPI_MASTER_RX_AUTOSUSP_DISABLE;
  hspi2.Init.MasterKeepIOState = SPI_MASTER_KEEP_IO_STATE_DISABLE;
  hspi2.Init.IOSwap = SPI_IO_SWAP_DISABLE;
  hspi2.Init.ReadyMasterManagement = SPI_RDY_MASTER_MANAGEMENT_INTERNALLY;
  hspi2.Init.ReadyPolarity = SPI_RDY_POLARITY_HIGH;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerState = SPI_AUTO_MODE_DISABLE;
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerSelection = SPI_GRP1_GPDMA_CH0_TCF_TRG;
  HAL_SPI_AutonomousMode_Cfg_Struct.TriggerPolarity = SPI_TRIG_POLARITY_RISING;
  if (HAL_SPIEx_SetConfigAutonomousMode(&hspi2, &HAL_SPI_AutonomousMode_Cfg_Struct) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM3_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 191;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 9999;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV2;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 5000;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  if (HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3) != HAL_OK)
  {
    Error_Handler();
  }
  HAL_TIM_MspPostInit(&htim3);
}

static void MX_UART5_Init(void)
{
  huart5.Instance = UART5;
  huart5.Init.BaudRate = 115200;
  huart5.Init.WordLength = UART_WORDLENGTH_8B;
  huart5.Init.StopBits = UART_STOPBITS_1;
  huart5.Init.Parity = UART_PARITY_NONE;
  huart5.Init.Mode = UART_MODE_TX_RX;
  huart5.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart5.Init.OverSampling = UART_OVERSAMPLING_16;
  huart5.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart5.Init.ClockPrescaler = UART_PRESCALER_DIV1;
  huart5.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetTxFifoThreshold(&huart5, UART_TXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_SetRxFifoThreshold(&huart5, UART_RXFIFO_THRESHOLD_1_8) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_UARTEx_DisableFifoMode(&huart5) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  HAL_GPIO_WritePin(GPIOC, ADC_RESET_Pin|ADC_CS_Pin|LCD_CS_Pin|LCD_DISP_Pin, GPIO_PIN_RESET);

  /* ADS1299 DRDY */
  GPIO_InitStruct.Pin = ADC_DRDY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADC_DRDY_GPIO_Port, &GPIO_InitStruct);

  /* ADS CS, RESET, LCD系ピン(使わなくても出力にしておく) */
  GPIO_InitStruct.Pin = ADC_RESET_Pin|ADC_CS_Pin|LCD_CS_Pin|LCD_DISP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /* PA8: MCO出力 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* ★ PA4: DAC1_OUT1 → アナログモード */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* USER CODE BEGIN 4 */

static void ads_send_command(uint8_t cmd)
{
  HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_SPI_Transmit(ADS_SPI_HANDLE, &cmd, 1, HAL_MAX_DELAY);
  HAL_Delay(1);
  HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_SET);
}

static void ads_write_reg(uint8_t addr, uint8_t data)
{
  uint8_t tx_buf[3];
  tx_buf[0] = CMD_WREG | addr;
  tx_buf[1] = 0x00;
  tx_buf[2] = data;

  HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_SPI_Transmit(ADS_SPI_HANDLE, tx_buf, 3, HAL_MAX_DELAY);
  HAL_Delay(1);
  HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_SET);
}

static uint8_t ads_read_reg(uint8_t addr)
{
  uint8_t tx_buf[3];
  uint8_t rx_buf[3] = {0};

  tx_buf[0] = CMD_RREG | addr;
  tx_buf[1] = 0x00;
  tx_buf[2] = 0x00;

  HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_SPI_TransmitReceive(ADS_SPI_HANDLE, tx_buf, rx_buf, 3, HAL_MAX_DELAY);
  HAL_Delay(1);
  HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_SET);

  return rx_buf[2];
}

static void ads_init(void)
{
  HAL_GPIO_WritePin(ADS_RESET_PORT, ADS_RESET_PIN, GPIO_PIN_SET);
  HAL_Delay(100);
  HAL_GPIO_WritePin(ADS_RESET_PORT, ADS_RESET_PIN, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(ADS_RESET_PORT, ADS_RESET_PIN, GPIO_PIN_SET);
  HAL_Delay(100);

  ads_send_command(CMD_SDATAC);
  HAL_Delay(1);

  ads_write_reg(REG_CONFIG3, 0xE0);
  HAL_Delay(1);

  ads_write_reg(REG_CH1SET, 0x60);
  HAL_Delay(1);

  for (int i = 0; i < 7; i++)
  {
    ads_write_reg(REG_CH2SET + i, 0x81);
    HAL_Delay(1);
  }

  ads_send_command(CMD_START);
}

static int32_t ads_read_ch1_data(void)
{
  uint8_t tx_cmd = CMD_RDATA;
  uint8_t rx_buffer[27] = {0};
  int32_t ch1_val = 0;

  HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_RESET);
  HAL_Delay(1);

  HAL_SPI_TransmitReceive(ADS_SPI_HANDLE, &tx_cmd, rx_buffer, 1, HAL_MAX_DELAY);
  HAL_SPI_Receive(ADS_SPI_HANDLE, &rx_buffer[1], 26, HAL_MAX_DELAY);

  HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_SET);

  ch1_val = (int32_t)((rx_buffer[3] << 16) |
                      (rx_buffer[4] << 8)  |
                       rx_buffer[5]);

  if (ch1_val & 0x00800000)
  {
    ch1_val |= 0xFF000000;
  }

  return ch1_val;
}

/* USER CODE END 4 */

void BSP_PB_Callback(Button_TypeDef Button)
{
  if (Button == BUTTON_USER)
  {
    button_request_change = 1;
    BspButtonState = BUTTON_PRESSED;
  }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
  (void)file;
  (void)line;
}
#endif
