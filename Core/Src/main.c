/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "adc.h"
#include "i2c.h"
#include "rtc.h"
#include "tim.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include "lcd.h"
#include <stdbool.h>
#include "eeprom.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint32_t value;

float temp,p_temp=0.0f, dest_temp_C = 15.0f, p_dest_temp_C = 0.0f, dest_temp_H = 24.0f, p_dest_temp_H = 0.0f;//variables for keeping temperature values
							//temp - current temperature, dest_temp_c/h - target temperature cold/hot, p_xxxx - previous value of this variable

bool work_state = true, p_work_state = false, boiler_state = false,p_boiler_state = false;
		//work_state - chosen target temperature (cold/hot), boiler_state - signal sent to boiler, p_xxxx - previous value of this variable

bool tick_stopped = false;//variable is set to true, when controller is in sleep mode. tick has to be stopped to prevent waking up processor by "increment tick interrupt" function

char* days[] = {"Pon ","Wt  ","Sr  ","Czw ","Pt  ","Sob ","Nied"};

char* work_state_strings[] = {"GT", "DT"};

RTC_TimeTypeDef time = {0};
RTC_TimeTypeDef pTime = {1};
RTC_DateTypeDef date = {0};
RTC_DateTypeDef pDate = {0};

bool schedule [7][24];//this array is a plan heating plan for every hour every day

int week_day=0,p_week_day = 0,hour=0, p_hour = 0;

bool p_mode = false;

volatile bool menu_enabled = false;

void CheckHisteresis()//changes boiler state according to histeresis
{
	if(work_state == true)
			  {
				  if(temp<=dest_temp_H-0.4f)
				  {
					  boiler_state = true;
					  HAL_GPIO_WritePin(OUTPUT_SIG_GPIO_Port, OUTPUT_SIG_Pin, GPIO_PIN_SET);
				  }
				  else if(temp>=dest_temp_H+0.4f)
				  {
					  boiler_state = false;
					  HAL_GPIO_WritePin(OUTPUT_SIG_GPIO_Port, OUTPUT_SIG_Pin, GPIO_PIN_RESET);
				  }
			  }
			  else
			  {
				  if(temp<=dest_temp_C-0.4f)
				  	{
				  		boiler_state = true;
				  		HAL_GPIO_WritePin(OUTPUT_SIG_GPIO_Port, OUTPUT_SIG_Pin, GPIO_PIN_SET);
				  	}
				  	else if(temp>=dest_temp_C+0.4f)
				  	{
				  		boiler_state = false;
				  		HAL_GPIO_WritePin(OUTPUT_SIG_GPIO_Port, OUTPUT_SIG_Pin, GPIO_PIN_RESET);
				  	}
			  }
}
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(tick_stopped == true)
	{
		HAL_ResumeTick();
		tick_stopped = false;
		__HAL_TIM_SET_COUNTER(&htim6,0);
		HAL_TIM_Base_Start_IT(&htim6);
	}

	if(htim==&htim7)//ADC reading for temperature sensor
	{
		HAL_ADC_Start(&hadc1);
		HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
		value = HAL_ADC_GetValue(&hadc1);
		temp = value*330.0f/4096.0f;
		CheckHisteresis();
		//printf("przerwanie\n");
	}
	if(htim==&htim6)//sleep mode activation
	{
		HAL_TIM_Base_Stop_IT(&htim6);
		tick_stopped = true;
		HAL_SuspendTick();
		HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
	}
}

/*int __io_putchar(int ch)
{
	if(ch=='\n')
	{
		uint8_t ch2 = '\r';
		HAL_UART_Transmit(&huart2, &ch2, 1, HAL_MAX_DELAY);
	}
	HAL_UART_Transmit(&huart2, (uint8_t*)&ch, 1, HAL_MAX_DELAY);
	return 1;
}*/

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)//interruptions from buttons
{
	__HAL_TIM_SET_COUNTER(&htim6,0);
	if(tick_stopped == true)
	{
		HAL_ResumeTick();
		tick_stopped = false;
		__HAL_TIM_SET_COUNTER(&htim6,0);
		HAL_TIM_Base_Start_IT(&htim6);
	}
	RTC_TimeTypeDef time = {0};
	RTC_DateTypeDef date = {0};

	time.TimeFormat = RTC_HOURFORMAT_24;
	HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);


	switch(GPIO_Pin)
	{
		case DAY_INPUT_Pin://changes day in main view or schedule menu
		{
			if(menu_enabled == false)
			{
				if(date.WeekDay == RTC_WEEKDAY_SUNDAY)
					date.WeekDay = RTC_WEEKDAY_MONDAY;
				else
					date.WeekDay++;
				HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN);
			}
			else
			{
				if(week_day<6)
					week_day++;
				else
					week_day = 0;
			}
			break;
		}
		case HOUR_INPUT_Pin://changes hour in main view or schedule menu
		{
			if(menu_enabled == false)
			{
				if(time.Hours>=23)
				{
					time.Hours = 0;
				}

				else
				{
					time.Hours++;
				}
				HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
			}
			else
			{
				if(hour<23)
					hour++;
				else
					hour = 0;
			}
			break;
		}
		case MINUTE_INPUT_Pin://changes minute in main view
		{
			if(menu_enabled == false)
			{
				if(time.Minutes >= 59)
				{
					time.Minutes = 0;
				}
				else
				{
					time.Minutes++;
				}
				HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
			}
			break;
		}
		case DEST_TEMP_CHANGE_Pin://changes destination temperature to high or low (work_state)
		{
			if(menu_enabled == false)
			{
				work_state = !work_state;
				CheckHisteresis();
			}
			else
			{
				schedule[week_day][hour] = !schedule[week_day][hour];
			}
			break;
		}
		case TEMP_PLUS_Pin://changes value of destination temperature
		{
			if(menu_enabled == false)
			{
				if(work_state == true)
				{
					if(dest_temp_H<30.0f)
						dest_temp_H += 0.5f;
				}
				else
				{
					if(dest_temp_C<=dest_temp_H-1.0f)
						dest_temp_C += 0.5f;
				}
				CheckHisteresis();
			}
			break;
		}
		case TEMP_MINUS_Pin://changes value of destination temperature
		{
			if(menu_enabled == false)
			{
				if(work_state == true)
				{
					if(dest_temp_H>=dest_temp_C+1.0f)
						dest_temp_H = dest_temp_H - 0.5f;
				}
				else
				{
					if(dest_temp_C>5)
						dest_temp_C -= 0.5f;
				}
				CheckHisteresis();
			}
			break;
		}
		case MENU_BUTTON_Pin://enters or leaves schedule menu
		{
			menu_enabled = !menu_enabled;
			break;
		}
	}
	HAL_Delay(50);
}



// Lcd_PortType ports[] = { D4_GPIO_Port, D5_GPIO_Port, D6_GPIO_Port, D7_GPIO_Port };
  Lcd_PortType ports[] = { GPIOC, GPIOB, GPIOA, GPIOC };
  // Lcd_PinType pins[] = {D4_Pin, D5_Pin, D6_Pin, D7_Pin};
  Lcd_PinType pins[] = {GPIO_PIN_7, GPIO_PIN_6, GPIO_PIN_7, GPIO_PIN_4};
  Lcd_HandleTypeDef lcd;

void DisplayDay()//displays day in main view
{
	Lcd_cursor(&lcd, 0,0);
	Lcd_string(&lcd, days[date.WeekDay-1]);
}
void DisplayDayInMenu()//displays day in menu
{
	Lcd_cursor(&lcd, 1,0);
	Lcd_string(&lcd, days[week_day]);
}
void DisplayHourInMenu()//displays hour in menu
{
	Lcd_cursor(&lcd, 1,5);
	Lcd_string(&lcd, "godz:");
	if(hour<10)
		Lcd_int(&lcd, 0);
	Lcd_int(&lcd, hour);
}
void DisplayModeInMenu()//displays chosen target temperature in schedule menu
{
	Lcd_cursor(&lcd, 1,12);
	Lcd_string(&lcd, "->");
	if(schedule[week_day][hour]==true)
	{
		Lcd_string(&lcd, work_state_strings[0]);
	}
	else
	{
		Lcd_string(&lcd, work_state_strings[1]);
	}
}
void DisplayTime()//displays time in main view
{
	Lcd_cursor(&lcd, 0,5);
	if(time.Hours<=9)
	    {
	    	Lcd_int(&lcd, 0);
	    	Lcd_int(&lcd, time.Hours);
	    }
	    else
	    	Lcd_int(&lcd, time.Hours);


	    Lcd_string(&lcd, ":");
	    Lcd_cursor(&lcd, 0,8);
	    if(time.Minutes<=9)
	        {
	        	Lcd_int(&lcd, 0);
	        	Lcd_int(&lcd, time.Minutes);
	        }
	    else
	    	Lcd_int(&lcd, time.Minutes);
}
void DisplayCurrentTemp()//displays current temperature in main view
{
	Lcd_cursor(&lcd, 0,11);
	int temp_float = (int)(temp*10)%10;
	Lcd_int(&lcd, (int)temp);
	Lcd_string(&lcd, ".");
	Lcd_int(&lcd, temp_float);
	Lcd_string(&lcd, "C");
}
void DisplayBoilerState()//displays state of boiler
{
	if(boiler_state == true)
	{
	    	Lcd_cursor(&lcd, 1,0);
	    	Lcd_string(&lcd, "Grzanie");
	}
	else
	{
		Lcd_cursor(&lcd, 1,0);
	    Lcd_string(&lcd, "       ");
	}
}
void DisplayWorkState()//displays work state and destination temperature
{
	int temp_float, temp;
	if(work_state == true)
	    {
			temp = (int)dest_temp_H;
	    	Lcd_cursor(&lcd, 1,8);
	    	Lcd_string(&lcd, "GT:");
	    	temp_float = (int)(dest_temp_H*10)%10;
	    	Lcd_int(&lcd, temp);
	    	Lcd_string(&lcd, ".");
	    	Lcd_int(&lcd, temp_float);
	    	if(temp<10)
	    		Lcd_string(&lcd, "C ");
	    	else
	    		Lcd_string(&lcd, "C");
	    }
	    else
	    {
	    	temp = (int)dest_temp_C;
	    	Lcd_cursor(&lcd, 1,8);
	    	Lcd_string(&lcd, "DT:");
	    	temp_float = (int)(dest_temp_C*10)%10;
	    	Lcd_int(&lcd, (int)dest_temp_C);
	    	Lcd_string(&lcd, ".");
	    	Lcd_int(&lcd, temp_float);
	    	if(temp<10)
	    		Lcd_string(&lcd, "C ");
	    	else
	    		Lcd_string(&lcd, "C");
	    }
}



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
  MX_RTC_Init();
  MX_ADC1_Init();
  MX_TIM7_Init();
  MX_I2C1_Init();
  MX_TIM6_Init();
  /* USER CODE BEGIN 2 */
  // Lcd_create(ports, pins, RS_GPIO_Port, RS_Pin, EN_GPIO_Port, EN_Pin, LCD_4_BIT_MODE);
  lcd = Lcd_create(ports, pins, GPIOB, GPIO_PIN_5, GPIOB, GPIO_PIN_4, LCD_4_BIT_MODE);




  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */

  HAL_TIM_Base_Start_IT(&htim7);//timer starts
  __HAL_TIM_SET_COUNTER(&htim7,1474);
  HAL_TIM_Base_Start_IT(&htim6);
  HAL_ADCEx_Calibration_Start(&hadc1, ADC_SINGLE_ENDED);//calibration of ADC

  HAL_ADC_Start(&hadc1);
  HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);//first reading


  value = HAL_ADC_GetValue(&hadc1);
  temp = value*330.0f/4096.0f;


  time.Hours = 00;
  time.Minutes = 00;
  time.Seconds = 00;
  time.TimeFormat = RTC_HOURFORMAT_24;
  HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN);
  time.TimeFormat = RTC_HOURFORMAT_24;


  if(eeprom_read(0x00, schedule, sizeof(schedule))==HAL_OK)
  {
	  printf("successful read\n");
  }
  else
	  printf("failed read\n");

  while (1)
  {
	  if(menu_enabled==true)
	  {
		  HAL_TIM_Base_Stop_IT(&htim6);
		  Lcd_clear(&lcd);
		  Lcd_cursor(&lcd, 0,3);
		  Lcd_string(&lcd, "Ustaw plan");
		  DisplayDayInMenu();
		  DisplayHourInMenu();
		  DisplayModeInMenu();
		  HAL_Delay(200);

		  while(menu_enabled == true)
		  {
			  if(p_week_day != week_day)
			  {
				  p_week_day = week_day;
				  DisplayDayInMenu();
			  }
			  if(p_hour != hour)
			  {
				  p_hour = hour;
				  DisplayHourInMenu();
			  }
			  if(p_mode != schedule[week_day][hour])
			  {
				  p_mode = schedule[week_day][hour];
				  DisplayModeInMenu();
			  }
		  }
			  /*if(HAL_GPIO_ReadPin(MENU_BUTTON_GPIO_Port, MENU_BUTTON_Pin) == GPIO_PIN_RESET)
			  {*/
				  //menu_enabled = false;

				  Lcd_clear(&lcd);
				  Lcd_cursor(&lcd, 0, 2);
				  Lcd_string(&lcd, "Zapisywanie");

				  work_state = schedule[date.WeekDay-1][time.Hours];
				  uint8_t day,hour,address = 0x00;
				  for(day=0;day<7;day++)
				  {
					  for(hour=0;hour<24;hour++)
					  {
						  if(eeprom_write(address, &schedule[day][hour], 1)==HAL_OK)
							  printf("%d %d write success\n",day,hour);
						  else
							  printf("%d %d write failed\n",day,hour);
						  address++;
					  }
				  }


				  Lcd_clear(&lcd);
				  DisplayTime();
				  DisplayDay();
				  DisplayCurrentTemp();
				  DisplayBoilerState();
				  DisplayWorkState();


				  HAL_Delay(200);
				  /*while(HAL_GPIO_ReadPin(MENU_BUTTON_GPIO_Port, MENU_BUTTON_Pin) == GPIO_PIN_RESET)
				  {}*/
			  //}
	  	  //}
		  __HAL_TIM_SET_COUNTER(&htim6,0);
		  HAL_TIM_Base_Start_IT(&htim6);
	  }
	  HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN);
	  HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN);
	  if(pTime.Minutes!=time.Minutes || pTime.Hours!=time.Hours)
	  {
		  if(pTime.Hours!=time.Hours)
		  {
			  work_state = schedule[date.WeekDay-1][time.Hours];
		  }
		  DisplayTime();
		  pTime = time;
	  }
	  if(pDate.WeekDay!=date.WeekDay)
	  {
		  DisplayDay();
		  pDate = date;
	  }
	  if(p_temp!=temp)
	  {
		  DisplayCurrentTemp();
		  p_temp = temp;
	  }
	  if(p_boiler_state!=boiler_state)
	  {
		  DisplayBoilerState();
		  p_boiler_state = boiler_state;
	  }
	  if(p_work_state!=work_state || p_dest_temp_C!=dest_temp_C || p_dest_temp_H!=dest_temp_H)
	  {
		  DisplayWorkState();
		  CheckHisteresis();
		  p_work_state = work_state;
		  p_dest_temp_C = dest_temp_C;
		  p_dest_temp_H = dest_temp_H;
	  }
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

  /** Configure the main internal regulator output voltage
  */
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_4;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

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

#ifdef  USE_FULL_ASSERT
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
