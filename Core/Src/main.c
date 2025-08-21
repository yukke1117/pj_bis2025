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
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
// Current calculation constants
#define CURRENT_OFFSET_MV 500
#define CURRENT_DIVISOR 151000
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

OPAMP_HandleTypeDef hopamp1;
OPAMP_HandleTypeDef hopamp2;

SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;

TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart5;

/* USER CODE BEGIN PV */
  static uint16_t scroll_counter = 0;
  static uint32_t last_animation_time = 0;
  static uint16_t bounce_y = 50;
  static int8_t bounce_direction = 1;
  static uint8_t blink_state = 1;
  
  // UART buffer for ADS1299 debug output
  static char uart_buf[100];

  // DAC and ADC test variables
  static uint32_t dac_value = 625;  // 固定値（約0.5V）
  static uint32_t adc_value = 0;
  // static uint8_t dac_direction = 1; // 1 for increasing, 0 for decreasing (不要なのでコメントアウト)

  // ADS1299のSPIハンドル (MX_SPI2_Init()で初期化されるもの)
  extern SPI_HandleTypeDef hspi2;
  #define ADS_SPI_HANDLE &hspi2

  // GPIOピンの定義 (CubeMXの"Pinout & Configuration"で設定したUser Labelと一致させる)
  #define ADS_CS_PORT     ADC_CS_GPIO_Port
  #define ADS_CS_PIN      ADC_CS_Pin
  #define ADS_RESET_PORT  ADC_RESET_GPIO_Port
  #define ADS_RESET_PIN   ADC_RESET_Pin
  #define ADS_DRDY_PORT   ADC_DRDY_GPIO_Port
  #define ADS_DRDY_PIN    ADC_DRDY_Pin

  // UARTデバッグ用 (MX_UART5_Init()で初期化されるもの)
  extern UART_HandleTypeDef huart5;
  #define ADS_UART_HANDLE &huart5s

  // コマンドとレジスタの定義
  // ADS1299のコマンド
  // System Commands
  #define CMD_WAKEUP    0x02
  #define CMD_STANDBY   0x04
  #define CMD_RESET     0x06
  #define CMD_START     0x08
  #define CMD_STOP      0x0A
  // Data Read Commands
  #define CMD_RDATAC    0x10
  #define CMD_SDATAC    0x11
  #define CMD_RDATA     0x12
  // Register Read Commands
  #define CMD_RREG      0x20
  #define CMD_WREG      0x40


  // ADS1299のレジスタアドレス
  // Read ONly ID Registers
  #define REG_ID        0x00
  // Global Settings Across Channels
  #define REG_CONFIG1   0x01
  #define REG_CONFIG2   0x02
  #define REG_CONFIG3   0x03
  #define REG_LOFF      0x04
  // Channel Settings
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
  #define REG_LOFF_FLIP 0x11
  // Lead-OFff Status Registers
  #define REG_LOFF_STATP 0x12
  #define REG_LOFF_STATN 0x13
  // GPIO and OTHER Registers
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
static void MX_OPAMP1_Init(void);
static void MX_ADC1_Init(void);
static void MX_OPAMP2_Init(void);
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

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_UART5_Init();
  MX_TIM3_Init();
  MX_SPI2_Init();
  MX_DAC1_Init();
  MX_OPAMP1_Init();
  MX_ADC1_Init();
  MX_OPAMP2_Init();
  MX_ADC2_Init();
  /* USER CODE BEGIN 2 */

  printf("Starting DAC/ADC/LCD Test System...\r\n");
  
  // LCD初期化
  LCD_Init();
  printf("LCD Initialized\r\n");

  // 画像を表示
  LCD_DrawImage();
  HAL_Delay(3000);
  LCD_FillWhite(); // 一時的にコメントアウト

  // LCD初期表示
  LCD_DrawString4bit(10, "DAC/ADC Test");
  LCD_DrawString4bit(30, "System Ready");
  
  // Start DAC
  if (HAL_DAC_Start(&hdac1, DAC_CHANNEL_1) != HAL_OK) {
      printf("DAC Start Error!\r\n");
      LCD_DrawString4bit(50, "DAC Error");
      Error_Handler();
  }
  printf("DAC Channel 1 Started\r\n");
  
  // Start OPAMP
  if (HAL_OPAMP_Start(&hopamp1) != HAL_OK) {
      printf("OPAMP Start Error!\r\n");
      LCD_DrawString4bit(70, "OPAMP Error");
      Error_Handler();
  }
  printf("OPAMP Started\r\n");
  
  // Start OPAMP2
  if (HAL_OPAMP_Start(&hopamp2) != HAL_OK) {
      printf("OPAMP2 Start Error!\r\n");
      LCD_DrawString4bit(90, "OPAMP2 Error");
      Error_Handler();
  }
  printf("OPAMP2 Started\r\n");
  
  // Calibrate ADC
  if (HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED) != HAL_OK) {
      printf("ADC Calibration Error!\r\n");
      LCD_DrawString4bit(90, "ADC Error");
      Error_Handler();
  }
  printf("ADC Calibrated and Ready\r\n");

  // Set initial DAC value
  dac_value = 625; // Middle value (1.65V for 3.3V reference)
  HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_value);
  printf("Initial DAC value set to: %lu (should be ~1.65V)\r\n", dac_value);

  printf("Starting ID Register Read Test...\r\n");

  // 一度リセット
  HAL_GPIO_WritePin(ADS_RESET_PORT, ADS_RESET_PIN, GPIO_PIN_SET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(ADS_RESET_PORT, ADS_RESET_PIN, GPIO_PIN_RESET);
  HAL_Delay(1);
  HAL_GPIO_WritePin(ADS_RESET_PORT, ADS_RESET_PIN, GPIO_PIN_SET);
  HAL_Delay(10);

  /* USER CODE END 2 */

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN BSP */

  /* -- Sample board code to send message over COM1 port ---- */
  printf("Welcome to STM32 world !\n\r");

  /* USER CODE END BSP */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    // DAC/ADC Test Loop with LCD Display
    // Generate a slowly changing DAC output (triangle wave)
    /*
    if (dac_direction == 1) {
        dac_value += 50; // Increase DAC value
        if (dac_value >= 4095) {
            dac_direction = 0; // Change direction at max
        }
    } else {
        dac_value -= 50; // Decrease DAC value
        if (dac_value <= 0) {
            dac_direction = 1; // Change direction at min
        }
    }
    */
    
    // Set new DAC value
    HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_value);
    
    // Read ADC value from the DAC output (via OPAMP follower) - 複数回測定して平均化
    uint32_t adc_sum = 0;
    const uint8_t num_samples = 10; // 10回測定して平均
    
    for (uint8_t i = 0; i < num_samples; i++) {
        if (HAL_ADC_Start(&hadc1) != HAL_OK) {
            printf("ADC Start Error!\r\n");
            break;
        }
        
        if (HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY) == HAL_OK) {
            adc_sum += HAL_ADC_GetValue(&hadc1);
        }
        HAL_ADC_Stop(&hadc1);
        HAL_Delay(1); // 測定間の短い遅延
    }
    
    adc_value = adc_sum / num_samples; // 平均値を計算
    
    // Calculate voltage values for display (mV単位で整数演算)
    uint32_t dac_voltage_mv = (dac_value * 3300) / 4095;
    uint32_t adc_voltage_mv = (adc_value * 3300) / 4095;
    
    // Calculate current from ADC voltage (μA単位)
    // I = (X/1000 - 0.5)/(151*1000) where X is in mV, I is in A
    // I_uA = ((X - 500) * 1000000) / (151 * 1000 * 1000)
    // I_uA = (X - 500) * 1000 / 151000
    // 整数演算で精度を保つため、64bit中間計算を使用してオーバーフロー防止
    int64_t temp = ((int64_t)adc_voltage_mv - CURRENT_OFFSET_MV) * 1000000;
    int32_t current_nA = temp / CURRENT_DIVISOR; // nA単位で計算
    int32_t current_uA = current_nA / 1000; // μA単位に変換
    int32_t current_uA_decimal = (current_nA < 0 ? -current_nA : current_nA) % 1000 / 100; // 小数第1位
    
    // Output DAC and ADC values via UART
    printf("DAC:%lu %lumV -> ADC:%lu %lumV (%ld.%ld uA)\r\n", 
           dac_value, dac_voltage_mv, adc_value, adc_voltage_mv, current_uA, current_uA_decimal);
    
    // Update LCD display every 10 iterations to reduce flicker
    static uint32_t lcd_update_counter = 0;
    lcd_update_counter++;
    
    if (lcd_update_counter % 10 == 0) { // LCDの更新頻度を下げる
        LCD_FillWhite();
        
        // Display title
        LCD_DrawString4bit(10, "Current Monitor");
        
        // Display DAC value and voltage (mV)
        char dac_str[32];
        snprintf(dac_str, sizeof(dac_str), "DAC: %lu mV", dac_voltage_mv);
        LCD_DrawString4bit(30, dac_str);
        
        // Display ADC voltage (mV)
        char adc_str[32];
        snprintf(adc_str, sizeof(adc_str), "ADC: %lu mV", adc_voltage_mv);
        LCD_DrawString4bit(50, adc_str);
        
        // Display calculated current (μA)
        char current_str[32];
        if (current_uA >= 0) {
            snprintf(current_str, sizeof(current_str), "Current: %ld.%ld uA", current_uA, current_uA_decimal);
        } else {
            snprintf(current_str, sizeof(current_str), "Current: -%ld.%ld uA", current_uA < 0 ? -current_uA : current_uA, current_uA_decimal);
        }
        LCD_DrawString4bit(70, current_str);
        
        // Display status
        LCD_DrawString4bit(90, "Status: RUNNING");
    }
    
    HAL_ADC_Stop(&hadc1);

    // Keep existing ADS1299 functionality (reduced frequency)
    static uint32_t ads_counter = 0;
    ads_counter++;
    
    if (ads_counter % 50 == 0) { // Every 50th iteration (less frequent for better LCD performance)
        uint8_t device_id = ads_read_reg(REG_ID);
        printf("ADS1299 ID: 0x%02X\r\n", device_id);
        
        // Display ADS1299 info on LCD
        char ads_str[22];
        snprintf(ads_str, sizeof(ads_str), "ADS ID: 0x%02X", device_id);
        LCD_DrawString4bit(90, ads_str);
        
        // Check DRDY pin and read data if available
        if (HAL_GPIO_ReadPin(ADS_DRDY_PORT, ADS_DRDY_PIN) == GPIO_PIN_RESET) {
            int32_t ch1_val = ads_read_ch1_data();
            printf("ADS CH1: %ld\r\n", ch1_val);
            
            // Display ADS1299 CH1 data on LCD
            char ch1_str[22];
            snprintf(ch1_str, sizeof(ch1_str), "CH1: %ld", ch1_val);
            LCD_DrawString4bit(110, ch1_str);
        }
    }
    
    HAL_Delay(100); // 100ms delay for readable output

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Enable Epod Booster
  */
  if (HAL_RCCEx_EpodBoosterClkConfig(RCC_EPODBOOSTER_SOURCE_MSIS, RCC_EPODBOOSTER_DIV1) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_PWREx_EnableEpodBooster() != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Set Flash latency before increasing MSIS
  */
  __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_2);

  /** Initializes the CPU, AHB and APB buses clocks
  */
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

  /** Initializes the CPU, AHB and APB buses clocks
  */
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

  /** MCO1 configuration
  */
  HAL_RCC_MCOConfig(RCC_MCO1, RCC_MCO1SOURCE_SYSCLK, RCC_MCODIV_64);
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Common config
  */
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

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief ADC2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC2_Init(void)
{

  /* USER CODE BEGIN ADC2_Init 0 */

  /* USER CODE END ADC2_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC2_Init 1 */

  /* USER CODE END ADC2_Init 1 */

  /** Common config
  */
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

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_2;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  if (HAL_ADC_ConfigChannel(&hadc2, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC2_Init 2 */

  /* USER CODE END ADC2_Init 2 */

}

/**
  * @brief DAC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_DAC1_Init(void)
{

  /* USER CODE BEGIN DAC1_Init 0 */

  /* USER CODE END DAC1_Init 0 */

  DAC_ChannelConfTypeDef sConfig = {0};
  DAC_AutonomousModeConfTypeDef sAutonomousMode = {0};

  /* USER CODE BEGIN DAC1_Init 1 */

  /* USER CODE END DAC1_Init 1 */

  /** DAC Initialization
  */
  hdac1.Instance = DAC1;
  if (HAL_DAC_Init(&hdac1) != HAL_OK)
  {
    Error_Handler();
  }

  /** DAC channel OUT1 config
  */
  sConfig.DAC_HighFrequency = DAC_HIGH_FREQUENCY_INTERFACE_MODE_DISABLE;
  sConfig.DAC_DMADoubleDataMode = DISABLE;
  sConfig.DAC_SignedFormat = DISABLE;
  sConfig.DAC_SampleAndHold = DAC_SAMPLEANDHOLD_DISABLE;
  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  sConfig.DAC_ConnectOnChipPeripheral = DAC_CHIPCONNECT_EXTERNAL;
  sConfig.DAC_UserTrimming = DAC_TRIMMING_FACTORY;
  if (HAL_DAC_ConfigChannel(&hdac1, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Autonomous Mode
  */
  sAutonomousMode.AutonomousModeState = DAC_AUTONOMOUS_MODE_DISABLE;
  if (HAL_DACEx_SetConfigAutonomousMode(&hdac1, &sAutonomousMode) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN DAC1_Init 2 */

  /* USER CODE END DAC1_Init 2 */

}

/**
  * @brief OPAMP1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OPAMP1_Init(void)
{

  /* USER CODE BEGIN OPAMP1_Init 0 */

  /* USER CODE END OPAMP1_Init 0 */

  /* USER CODE BEGIN OPAMP1_Init 1 */

  /* USER CODE END OPAMP1_Init 1 */
  hopamp1.Instance = OPAMP1;
  hopamp1.Init.Mode = OPAMP_FOLLOWER_MODE;
  hopamp1.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO0;
  hopamp1.Init.PowerMode = OPAMP_POWERMODE_NORMALPOWER_NORMALSPEED;
  hopamp1.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
  if (HAL_OPAMP_Init(&hopamp1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OPAMP1_Init 2 */

  /* USER CODE END OPAMP1_Init 2 */

}

/**
  * @brief OPAMP2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_OPAMP2_Init(void)
{

  /* USER CODE BEGIN OPAMP2_Init 0 */

  /* USER CODE END OPAMP2_Init 0 */

  /* USER CODE BEGIN OPAMP2_Init 1 */

  /* USER CODE END OPAMP2_Init 1 */
  hopamp2.Instance = OPAMP2;
  hopamp2.Init.Mode = OPAMP_FOLLOWER_MODE;
  hopamp2.Init.NonInvertingInput = OPAMP_NONINVERTINGINPUT_IO0;
  hopamp2.Init.PowerMode = OPAMP_POWERMODE_NORMALPOWER_NORMALSPEED;
  hopamp2.Init.UserTrimming = OPAMP_TRIMMING_FACTORY;
  if (HAL_OPAMP_Init(&hopamp2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN OPAMP2_Init 2 */

  /* USER CODE END OPAMP2_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  SPI_AutonomousModeConfTypeDef HAL_SPI_AutonomousMode_Cfg_Struct = {0};

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
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
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  SPI_AutonomousModeConfTypeDef HAL_SPI_AutonomousMode_Cfg_Struct = {0};

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
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
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
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
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */
  HAL_TIM_MspPostInit(&htim3);

}

/**
  * @brief UART5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_UART5_Init(void)
{

  /* USER CODE BEGIN UART5_Init 0 */

  /* USER CODE END UART5_Init 0 */

  /* USER CODE BEGIN UART5_Init 1 */

  /* USER CODE END UART5_Init 1 */
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
  /* USER CODE BEGIN UART5_Init 2 */

  /* USER CODE END UART5_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, ADC_RESET_Pin|ADC_CS_Pin|LCD_CS_Pin|LCD_DISP_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : ADC_DRDY_Pin */
  GPIO_InitStruct.Pin = ADC_DRDY_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(ADC_DRDY_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : ADC_RESET_Pin ADC_CS_Pin LCD_CS_Pin LCD_DISP_Pin */
  GPIO_InitStruct.Pin = ADC_RESET_Pin|ADC_CS_Pin|LCD_CS_Pin|LCD_DISP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : PA8 */
  GPIO_InitStruct.Pin = GPIO_PIN_8;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF0_MCO;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief  ADS1299に単一コマンドを送信する
  * @param  cmd 送信するコマンド
  * @retval None
  */
static void ads_send_command(uint8_t cmd) {
    HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_RESET);
    HAL_Delay(1); // 最小限の遅延
    HAL_SPI_Transmit(ADS_SPI_HANDLE, &cmd, 1, HAL_MAX_DELAY);
    HAL_Delay(1);
    HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_SET);
}

/**
  * @brief  ADS1299のレジスタに1バイト書き込む
  * @param  addr レジスタアドレス
  * @param  data 書き込むデータ
  * @retval None
  */
static void ads_write_reg(uint8_t addr, uint8_t data) {
    uint8_t tx_buf[3];
    tx_buf[0] = CMD_WREG | addr; // WREGコマンド + アドレス
    tx_buf[1] = 0x00;            // 書き込むレジスタ数 - 1 (今回は1つなので0)
    tx_buf[2] = data;            // 書き込むデータ

    HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_SPI_Transmit(ADS_SPI_HANDLE, tx_buf, 3, HAL_MAX_DELAY);
    HAL_Delay(1);
    HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_SET);
}

/**
  * @brief  ADS1299のレジスタから1バイト読み出す
  * @param  addr レジスタアドレス
  * @retval 読み出したデータ
  */
static uint8_t ads_read_reg(uint8_t addr) {
    uint8_t tx_buf[3];
    uint8_t rx_buf[3] = {0};
    
    tx_buf[0] = CMD_RREG | addr; // RREGコマンド + アドレス
    tx_buf[1] = 0x00;            // 読み出すレジスタ数 - 1 (今回は1つなので0)
    tx_buf[2] = 0x00;            // ダミーバイト

    HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_SPI_TransmitReceive(ADS_SPI_HANDLE, tx_buf, rx_buf, 3, HAL_MAX_DELAY);
    HAL_Delay(1);
    HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_SET);
    
    return rx_buf[2]; // 3バイト目にレジスタの値が入っている
}

/**
  * @brief  ADS1299を初期化する
  * @retval None
  */
static void ads_init(void) {
    // 1. デバイスをリセット
    HAL_GPIO_WritePin(ADS_RESET_PORT, ADS_RESET_PIN, GPIO_PIN_SET);
    HAL_Delay(100);
    HAL_GPIO_WritePin(ADS_RESET_PORT, ADS_RESET_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(ADS_RESET_PORT, ADS_RESET_PIN, GPIO_PIN_SET);
    HAL_Delay(100); // リセット後の安定化待ち [cite: 1713]

    // 2. RDATACモードを停止
    ads_send_command(CMD_SDATAC);
    HAL_Delay(1);

    // 3. レジスタ設定
    // CONFIG3: 内部リファレンスを有効化
    ads_write_reg(REG_CONFIG3, 0xE0); // PD_REFBUF=1 [cite: 2375]
    HAL_Delay(1);

    // CH1SET: CH1を有効化、ゲイン24倍、通常入力に設定
    ads_write_reg(REG_CH1SET, 0x60); // PD1=0, GAIN=24, MUX=Normal [cite: 2460]
    HAL_Delay(1);

    // CH2-CH8をパワーダウン
    for (int i = 0; i < 7; i++) {
        ads_write_reg(REG_CH2SET + i, 0x81); // PDn=1, MUXn=Input Short [cite: 2460]
        HAL_Delay(1);
    }

    // 4. 変換開始
    ads_send_command(CMD_START); // [cite: 1961]
}

/**
  * @brief  ADS1299のCH1データを読み出す
  * @retval 32ビット符号付き整数に変換されたCH1のデータ
  */
static int32_t ads_read_ch1_data(void) {
    uint8_t tx_cmd = CMD_RDATA;
    uint8_t rx_buffer[27] = {0};
    int32_t ch1_val = 0;

    HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_RESET);
    HAL_Delay(1);

    // RDATAコマンドを送信し、同時にデータフレーム全体(27バイト)を受信する
    HAL_SPI_TransmitReceive(ADS_SPI_HANDLE, &tx_cmd, rx_buffer, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(ADS_SPI_HANDLE, &rx_buffer[1], 26, HAL_MAX_DELAY);

    HAL_GPIO_WritePin(ADS_CS_PORT, ADS_CS_PIN, GPIO_PIN_SET);

    // 受信バッファからCH1のデータ(4,5,6バイト目)を取得
    // [cite: 1756, 1895, 1896]
    ch1_val = (int32_t)((rx_buffer[3] << 16) | (rx_buffer[4] << 8) | rx_buffer[5]);

    // 24ビットの2の補数表現を32ビットに符号拡張する
    if (ch1_val & 0x00800000) {
        ch1_val |= 0xFF000000;
    }

    return ch1_val;
}

/* USER CODE END 4 */

/**
  * @brief BSP Push Button callback
  * @param Button Specifies the pressed button
  * @retval None
  */
void BSP_PB_Callback(Button_TypeDef Button)
{
  if (Button == BUTTON_USER)
  {
    BspButtonState = BUTTON_PRESSED;
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
