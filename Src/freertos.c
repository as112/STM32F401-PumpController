/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "lcd_hd44780_i2c.h"
#include "ds18b20.h"
#include "stdio.h"

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
/* USER CODE BEGIN Variables */
float TempKot;
/* USER CODE END Variables */
osThreadId defaultTaskHandle;
osThreadId myTask02Handle;

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
extern I2C_HandleTypeDef hi2c1;
extern IWDG_HandleTypeDef hiwdg;
/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void const * argument);
void mainTask(void const * argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* GetIdleTaskMemory prototype (linked to static allocation support) */
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize );

/* USER CODE BEGIN GET_IDLE_TASK_MEMORY */
static StaticTask_t xIdleTaskTCBBuffer;
static StackType_t xIdleStack[configMINIMAL_STACK_SIZE];
  
void vApplicationGetIdleTaskMemory( StaticTask_t **ppxIdleTaskTCBBuffer, StackType_t **ppxIdleTaskStackBuffer, uint32_t *pulIdleTaskStackSize )
{
  *ppxIdleTaskTCBBuffer = &xIdleTaskTCBBuffer;
  *ppxIdleTaskStackBuffer = &xIdleStack[0];
  *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
  /* place for user code */
}                   
/* USER CODE END GET_IDLE_TASK_MEMORY */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
       
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* definition and creation of myTask02 */
  osThreadDef(myTask02, mainTask, osPriorityNormal, 0, 256);
  myTask02Handle = osThreadCreate(osThread(myTask02), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used 
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN StartDefaultTask */
	Ds18b20_Init(osPriorityRealtime);
	
  /* Infinite loop */
  for(;;)
  {
		osDelay(14400000);
		NVIC_SystemReset();
  }
  /* USER CODE END StartDefaultTask */
}

/* USER CODE BEGIN Header_mainTask */
/**
* @brief Function implementing the myTask02 thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_mainTask */
void mainTask(void const * argument)
{
  /* USER CODE BEGIN mainTask */
	char stringForLcd[16];
	uint8_t nasosFlag = 1;
	lcdInit(&hi2c1, (uint8_t)0x27, (uint8_t)2, (uint8_t)16);
	lcdPrintStr((uint8_t*)"Termostat-v3.1    ", 16);
	lcdSetCursorPosition(0, 1);
	lcdPrintStr((uint8_t*)"F401-FreeRTOS    ", 16);
	vTaskDelay(2000);
	lcdCommand(LCD_CLEAR, LCD_PARAM_SET);
  /* Infinite loop */
  for(;;)
  {
//		GPIOC->ODR ^= GPIO_ODR_OD13;
		HAL_IWDG_Refresh(&hiwdg);
		TempKot = (!ds18b20[0].DataIsValid) ? 0 : ds18b20[0].Temperature;             	//get t from sensor 1
		sprintf(stringForLcd, "Temp = %.1f      ", TempKot);
    lcdSetCursorPosition(0, 0);
		lcdPrintStr((uint8_t*)stringForLcd, 16);
    lcdSetCursorPosition(0, 1);
		
		if((TempKot >= 1) && (TempKot <= 60) && (nasosFlag == 1)) {
			NASOS_OFF;
			nasosFlag = 0;
			lcdPrintStr((uint8_t*)"NASOS OFF", 9); 
		}
		if((TempKot >= 1) && (TempKot >= 61) && (nasosFlag == 0)) {		
			NASOS_ON;
			nasosFlag = 1;
			lcdPrintStr((uint8_t*)"NASOS ON  ", 10); 
		}

		if(TempKot < 1) {
			NASOS_ON;
			nasosFlag = 1;
			lcdSetCursorPosition(0, 0);
			lcdPrintStr((uint8_t*)"ALARM            ", 16);
			lcdSetCursorPosition(0, 1);
			lcdPrintStr((uint8_t*)"NASOS ON  ", 10);
		}
    osDelay(1000);
  }
  /* USER CODE END mainTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
     
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
