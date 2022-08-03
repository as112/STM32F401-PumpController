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
#include "lcd_i2c_pcf8574.h"
#include "ds18b20.h"
#include "stdio.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
void PrintNasosON(void);
void PrintNasosOFF(void);
void PrintAlarm(void);
void PrintTemp(void);
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

	lcd_Init();
	lcd_SendString((char*)"Termostat-v3.2");
	lcd_SetCursor(0, 1);
	lcd_SendString((char*)"F401-FreeRTOS");
	vTaskDelay(2000);
	lcd_Clearscreen();
//	if(RTC->BKP0R == 0) {
//		RTC->BKP0R = 1;
//		HAL_NVIC_SystemReset();
//	}
  /* Infinite loop */
  for(;;)
  {
	//HAL_IWDG_Refresh(&hiwdg);
	TempKot = (!ds18b20[0].DataIsValid) ? 0 : ds18b20[0].Temperature;             	//get t from sensor 1
	lcd_SetCursor(0, 0);
	if(TempKot == 0) {
		NASOS_ON;
		nasosFlag = 1;
		PrintAlarm();
		lcd_SetCursor(0, 1);
		PrintNasosON();
		osDelay(3000);
		continue;
	}
	
	if((TempKot >= 1) && (TempKot <= 60) && (nasosFlag == 1)) {
		NASOS_OFF;
		nasosFlag = 0;
		PrintTemp();
		lcd_SetCursor(0, 1);
		PrintNasosOFF();
	}
	if((TempKot >= 61) && (nasosFlag == 0)) {			
		NASOS_ON;
		nasosFlag = 1;
		PrintTemp();
		lcd_SetCursor(0, 1);
		PrintNasosON();
	}
	lcd_SetCursor(4, 0);
	sprintf(stringForLcd, " = %.1f  ", TempKot);
	lcd_SendString(stringForLcd);
	
    osDelay(1000);
  }
  /* USER CODE END mainTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
uint8_t symL[8]   = { 0x0F, 0x05, 0x05, 0x05, 0x05, 0x15, 0x09 }; 		// Л
uint8_t symY[8] = { 0x11, 0x11, 0x11, 0x19, 0x15, 0x15, 0x19, 0x00};	// Ы
uint8_t symI[8]   = { 0x11, 0x11, 0x13, 0x15, 0x19, 0x11, 0x11 }; 		// И
uint8_t symJA[8]  = { 0x0F, 0x11, 0x11, 0x0F, 0x05, 0x09, 0x11 }; 		// Я
uint8_t symP[8]   = { 0x1F, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11 }; 		// П

void PrintNasosON(void) {
	
	lcd_SendString((char*)"HACOC BK   ");
	lcdLoadCustomChar(0, symL);
	lcd_SetCursor(8, 1);
	lcd_data(0);
}
void PrintNasosOFF(void) {
	
	lcd_SendString((char*)"HACOC B   ");
	lcdLoadCustomChar(1, symY);
	lcd_SetCursor(7, 1);
	lcd_data(1);
	lcd_SetCursor(8, 1);
	lcd_SendString((char*)"K");
	lcdLoadCustomChar(2, symL);
	lcd_SetCursor(9, 1);
	lcd_data(2);
}
void PrintAlarm(void) {
	
	lcd_SendString((char*)"ABAP          ");
	lcdLoadCustomChar(3, symI);
	lcd_SetCursor(4, 0);
	lcd_data(3);
	lcdLoadCustomChar(4, symJA);
	lcd_SetCursor(5, 0);
	lcd_data(4);
}
void PrintTemp(void) {
	
	lcd_SendString((char*)"TEM        ");
	lcdLoadCustomChar(5, symP);
	lcd_SetCursor(3, 0);
	lcd_data(5);
}
/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
