
#include "main.h"
#include <string.h>
#include <stdio.h>

ADC_HandleTypeDef hadc1;
I2C_HandleTypeDef hi2c1;
UART_HandleTypeDef huart2;

float A1_value = 0;
float A2_value = 0;
float threshold = 2.0;  // volts

uint8_t A1_flag = 0;
uint8_t A2_flag = 0;
uint8_t D1_state = 1;
uint8_t D1_prev = 1;
uint8_t D2_state = 1;
uint8_t D2_prev = 1;

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_I2C1_Init(void);
static void MX_USART2_UART_Init(void);
float Read_ADC(uint32_t channel);
uint8_t Read_Digital(GPIO_TypeDef *port, uint16_t pin);
void LED_Set(uint8_t id, uint8_t state);
void Relay_Set(uint8_t id, uint8_t state);
void GSM_Send_Command(char *cmd);
void GSM_Send_SMS(char *number, char *message);
void GSM_Init();
void LCD_Init(void);
void LCD_Send_Command(uint8_t cmd);
void LCD_Send_Data(uint8_t data);
void LCD_Send_String(char *str);
void LCD_Set_Cursor(uint8_t row, uint8_t col);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC1_Init();
  MX_I2C1_Init();
  MX_USART2_UART_Init();
  GSM_Init();
  LCD_Init();
  LCD_Send_String("System Ready");

  while (1)
  {
	  // --- Read Analog ---
	  A1_value = Read_ADC(ADC_CHANNEL_0);
	  A2_value = Read_ADC(ADC_CHANNEL_1);

	  // --- Read Digital ---
	  D1_state = Read_Digital(GPIOB, GPIO_PIN_0);
	  D2_state = Read_Digital(GPIOB, GPIO_PIN_1);

	  // --- Analog Logic ---
	  if(A1_value > threshold && A1_flag == 0)
	  {
	      LED_Set(1, 1);
	      Relay_Set(1, 1);
	      char msg[16];
	     // sprintf(msg, "A1: %.2fV HIGH", A1_value);
	      LCD_Set_Cursor(0,0);
          LCD_Send_String(msg);
	      char sms[50];
         // sprintf(sms, "Alert! A1=%.2fV", A1_value);
          GSM_Send_SMS("+91XXXXXXXXXX", sms);
	      A1_flag = 1;
	  }
	  if(A1_value <= threshold)
	      A1_flag = 0;

	  // A2...
	  if(A2_value > threshold && A2_flag == 0)
	  {
	      LED_Set(2, 1);
	      Relay_Set(2, 1);
	      char msg[16];
	      //sprintf(msg, "A2: %.2fV HIGH", A2_value);
	      LCD_Set_Cursor(1,0);
	      LCD_Send_String(msg);

	      char sms[50];
	      //sprintf(sms, "Alert! A2=%.2fV", A2_value);
	      GSM_Send_SMS("+91XXXXXXXXXX", sms);
	      A2_flag = 1;
	  }

	  if(A2_value <= threshold)
	  {
	      LED_Set(2, 0);
	      Relay_Set(2, 0);

	      A2_flag = 0;
	  }
	  // --- Digital Logic ---
	    if(D1_state == GPIO_PIN_RESET)
	    {
	        LED_Set(3, 1);
	        Relay_Set(3, 1);
	        LCD_Set_Cursor(1,0);
	        LCD_Send_String("D1 ACTIVE     ");
	    }
	    else
	    {
	        LED_Set(3, 0);
	        Relay_Set(3, 0);
	    }
	    if(D2_state == GPIO_PIN_RESET)
	    {
	        LED_Set(4, 1);
	        Relay_Set(4, 1);
	        LCD_Set_Cursor(1,0);
	        LCD_Send_String("D2 ACTIVE     ");
	    }
	    else
	    {
	        LED_Set(4, 0);
	        Relay_Set(4, 0);
	    }

	    HAL_Delay(100);

  }
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = ENABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 2;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfig.Channel = ADC_CHANNEL_1;
  sConfig.Rank = 2;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_I2C1_Init(void)
{
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
}
static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
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

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3
                           PC4 PC5 PC6 PC7 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

#define LCD_ADDR (0x27 << 1)
void LCD_Send(uint8_t data, uint8_t rs)
{
    uint8_t high = (data & 0xF0);
    uint8_t low  = ((data << 4) & 0xF0);

    uint8_t data_arr[4];

    data_arr[0] = high | rs | 0x08 | 0x04;
    data_arr[1] = high | rs | 0x08;
    data_arr[2] = low  | rs | 0x08 | 0x04;
    data_arr[3] = low  | rs | 0x08;

    HAL_I2C_Master_Transmit(&hi2c1, LCD_ADDR, data_arr, 4, HAL_MAX_DELAY);
}
void LCD_Send_Command(uint8_t cmd)
{
    LCD_Send(cmd, 0);
}

void LCD_Send_Data(uint8_t data)
{
    LCD_Send(data, 1);
}
void LCD_Init(void)
{
    HAL_Delay(50);
    LCD_Send_Command(0x30);
    HAL_Delay(5);
    LCD_Send_Command(0x30);
    HAL_Delay(1);
    LCD_Send_Command(0x30);
    LCD_Send_Command(0x20); // 4-bit mode
    HAL_Delay(1);
    LCD_Send_Command(0x28); // 2 line
    LCD_Send_Command(0x08); // display off
    LCD_Send_Command(0x01); // clear
    HAL_Delay(2);
    LCD_Send_Command(0x06); // entry mode
    LCD_Send_Command(0x0C); // display on
}
void LCD_Set_Cursor(uint8_t row, uint8_t col)
{
    uint8_t addr = (row == 0) ? 0x80 + col : 0xC0 + col;
    LCD_Send_Command(addr);
}
void LCD_Send_String(char *str)
{
    while(*str)
    {
        LCD_Send_Data(*str++);
    }
}
/////////////////////////////////////////////////////////

void GSM_Send_Command(char *cmd)
{
    HAL_UART_Transmit(&huart2, (uint8_t*)cmd, strlen(cmd), HAL_MAX_DELAY);
}
void GSM_Init()
{
    HAL_Delay(1000); // wait for module

    GSM_Send_Command("AT\r\n");
    HAL_Delay(500);

    GSM_Send_Command("AT+CMGF=1\r\n"); // Text mode
    HAL_Delay(500);
}
void GSM_Send_SMS(char *number, char *message)
{
    char cmd[50];

    sprintf(cmd, "AT+CMGS=\"%s\"\r\n", number);
    GSM_Send_Command(cmd);

    HAL_Delay(500);

    GSM_Send_Command(message);

    HAL_Delay(200);

    uint8_t ctrlZ = 26;
    HAL_UART_Transmit(&huart2, &ctrlZ, 1, HAL_MAX_DELAY);

    HAL_Delay(3000); // wait for sending
}
///////////////////////////////////////////////////////////
float Read_ADC(uint32_t channel)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    sConfig.Channel = channel;
    sConfig.Rank = 1;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);

    uint32_t raw = HAL_ADC_GetValue(&hadc1);

    return (raw * 3.3f) / 4095.0f;
}

uint8_t Read_Digital(GPIO_TypeDef *port, uint16_t pin)
{
    return HAL_GPIO_ReadPin(port, pin);
}

void LED_Set(uint8_t id, uint8_t state)
{
    GPIO_PinState pinState = state ? GPIO_PIN_SET : GPIO_PIN_RESET;

    switch(id)
    {
        case 1: HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0, pinState); break;
        case 2: HAL_GPIO_WritePin(GPIOC, GPIO_PIN_1, pinState); break;
        case 3: HAL_GPIO_WritePin(GPIOC, GPIO_PIN_2, pinState); break;
        case 4: HAL_GPIO_WritePin(GPIOC, GPIO_PIN_3, pinState); break;
    }
}

void Relay_Set(uint8_t id, uint8_t state)
{
    GPIO_PinState pinState = state ? GPIO_PIN_SET : GPIO_PIN_RESET;

    switch(id)
    {
        case 1: HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4, pinState); break;
        case 2: HAL_GPIO_WritePin(GPIOC, GPIO_PIN_5, pinState); break;
        case 3: HAL_GPIO_WritePin(GPIOC, GPIO_PIN_6, pinState); break;
        case 4: HAL_GPIO_WritePin(GPIOC, GPIO_PIN_7, pinState); break;
    }
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
