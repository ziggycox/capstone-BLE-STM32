/**
  * Main file that contains while() loop with BLE
  * receiving / sending capability to phone app.
  *
  * Author: Terry Cox (coxte@pm.me)
  * Date:   6/9/19
  */
  
#include "main.h"
#include "DataStructsBLE.h"
#include "HitDataQueueBLE.h"
#include "LogDataQueueBLE.h"
#include "DataInQueueBLE.h"
#include "BLE.h"
#include <stdbool.h>
#include <stdlib.h>

#define capacity 60

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;
UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);		// UART for Computer
static void MX_USART1_UART_Init(void);		// UART for BLE board
//static void MX_RTC_Init(void);			// For Real Time Clock

/* Public Flags --------------------------------------------------------------------*/
bool transmitHitDataFlag = false;
bool transmitLogDataFlag = false;
bool dataInFlag = false;

/* Helper things---------------------------------------------------------------------------------*/
bool dataReceived = false;
RTC_TimeTypeDef currTime;
HitDataBLE newData;				// used to add data to HitData queue and transmit
DataInBLE dataTestFromQueue;	// used to look at received data

/* Helper functions ------------------------------------------------------------*/
void testTransmissionCases(unsigned char caseNum);
void testTransmission(unsigned char panel);
void switchTargetOn(unsigned char num);
void switchTargetOff(unsigned char num);
unsigned int getTimeSeconds(void);

// TEST things
bool testFlag = false;
unsigned char oldPanel = 0, newPanel = 0;
unsigned char testCounter = 0;

/**
  * Main Program that sets up UART1 for BLE
  * UART1 (BLE) using pins TX: PA9 -- RX: PA10 (STM side)
  * LED1 - 5 using pins PB13, PB14, PB15, PB1, PB2 respectively
  */
int main(void) {

	/* MCU Configuration--------------------------------------------------------*/
	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* Configure the system clock */
	SystemClock_Config();

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART2_UART_Init();				// UART for Computer
	MX_USART1_UART_Init();				// UART for BLE board
	//MX_RTC_Init();                      // RTC

	// data transfer buffer
	unsigned char phoneSendBuffHit[24];
	unsigned char phoneSendBuffLog[26];
	unsigned char phoneReceiveBuff[15];
	unsigned char phoneSendAck[3];

	// Seed random number generator for picking a hit panel to light up
	srand(27);

    while(1) {
		// Code to simulate "hits"
		/*
		newPanel = ((rand() % 5) + 1);		// Returns num 1 - 5
		switchTargetOn(newPanel);
		*/
	// -----------Production Code------------------------------------------------------------------	
		
		// Receive data buffer from BLE (sent from phone)
		HAL_UART_Receive_IT(&huart1,phoneReceiveBuff,sizeof(phoneReceiveBuff));
		if(dataReceived) {
			// See if message received is ack or not
			bool ackReceived = decodeAck(phoneReceiveBuff);
			// Not ack, update flag
			dataReceived = false;
			if(!ackReceived) {
				// Is DataIn, decode + put into queue
				addDataInQueue(decode(phoneReceiveBuff));
			
			//----Start test code that is started from phone app----------------------------------
				// For testing cases from phone
				if(phoneReceiveBuff[0] == 4) {
					// In test
					if(!isEmptyDataInQueue()) {
					  dataTestFromQueue = removeDataInQueue();
					}
					// Check to see if test or not
					// New test reset variables
					if(dataTestFromQueue.startEnd == 1) {
					  testCounter = 0;
					  resetHitDataSeq();
					  testFlag = true;
					}
					// send ack back to phone, phoneSendAck gets populated
					sendAckTest(phoneSendAck);
					HAL_UART_Transmit_IT(&huart1,phoneSendAck,sizeof(phoneSendAck));
				} else {
			//-----End test code---------------------------------------------------------------------
					
					// update flags
					dataInFlag = true;
					// send ack back to phone, phoneSendAck gets populated
					sendAck(phoneSendAck);
					HAL_UART_Transmit_IT(&huart1,phoneSendAck,sizeof(phoneSendAck));
				}
			}
		}
		
	  //----Start test code that is started from phone app----------------------------------
		// Populate queue with known test cases
		if(testFlag) {
			testCounter++;
			// only send 4 HitDataBLE packets
			if(testCounter < 5) {
				testTransmissionCases(testCounter);
			} else {
				testFlag = false;
			}
		}
	  //-----End test code---------------------------------------------------------------------

		// transmit data to phone
		// Condition: data to transmit or queue not empty
		// Hit Data
		if(transmitHitDataFlag || (!isEmptyHitDataQueue())) {
		  // acks all catch up
		  if(canTransmitHitData()) {			  
			  // encode data and put it into phoneSendBuffHit
			  encodeHitData(peekHitDataQueue(),phoneSendBuffHit, 1);
			  HAL_UART_Transmit_IT(&huart1,phoneSendBuffHit,sizeof(phoneSendBuffHit));
			  transmitHitDataFlag = false;
		  } else {
			  // didn't receive ack for queue item 
			  // resend old item
			  // encode data and put it into phoneSendBuffHit
			  encodeHitData(peekHitDataQueue(),phoneSendBuffHit, 0);
			  HAL_UART_Transmit_IT(&huart1,phoneSendBuffHit,sizeof(phoneSendBuffHit));
		  }
		}
		// Log Data
		if(transmitLogDataFlag || (!isEmptyLogDataQueue())) {
			// acks all catch up
			if(canTransmitLogData()) {
			  // encode data and put it into phoneSendBuffLog
			  encodeLogData(peekLogDataQueue(),phoneSendBuffLog, 1);
			  HAL_UART_Transmit_IT(&huart1,phoneSendBuffLog,sizeof(phoneSendBuffLog));
			  transmitLogDataFlag = false;
			} else {
			  // didn't receive ack for queue item 
			  // resend old item
			  // encode data and put it into phoneSendBuffLog
			  encodeLogData(peekLogDataQueue(),phoneSendBuffLog, 0);
			  HAL_UART_Transmit_IT(&huart1,phoneSendBuffLog,sizeof(phoneSendBuffLog));
			}
		}
		
	// -------End Production Code----------------------------------------------------------------------

		HAL_Delay(500);
	}
}

// Function that will take a random number and first turn off a panel's LED
// through GPIO pins and then turn on a new one for a player to hit at.
// Calls testTransmission() at the end to simulate adding data to be sent in
// the queue and then setting the transmission flag.
// LED1 - 5 using pins PB13, PB14, PB15, PB1, PB2 respectively
void switchTargetOn(unsigned char num) {
  // num will be 1 - 5 for LED target to turn on
  // Toggle old off
  switchTargetOff(oldPanel);
  switch(num) {
    case 1  :
      // Toggle new on
      HAL_GPIO_WritePin(GPIOB, LED1_Pin, GPIO_PIN_SET);
      break;
    case 2  :
      HAL_GPIO_WritePin(GPIOB, LED2_Pin, GPIO_PIN_SET);
      break;
    case 3  :
      HAL_GPIO_WritePin(GPIOB, LED3_Pin, GPIO_PIN_SET);
      break;
    case 4  :
      HAL_GPIO_WritePin(GPIOB, LED4_Pin, GPIO_PIN_SET);
      break;
    case 5  :
      HAL_GPIO_WritePin(GPIOB, LED5_Pin, GPIO_PIN_SET);
      break;
  }
  oldPanel = num;
  testTransmission(num);
}

// Function that turns old LED panel off
void switchTargetOff(unsigned char num) {
  // num will be 1 - 5 for LED target to turn on
  switch(num) {
    case 1  :
      HAL_GPIO_WritePin(GPIOB, LED1_Pin, GPIO_PIN_RESET);
      break;
    case 2  :
      HAL_GPIO_WritePin(GPIOB, LED2_Pin, GPIO_PIN_RESET);
      break;
    case 3  :
      HAL_GPIO_WritePin(GPIOB, LED3_Pin, GPIO_PIN_RESET);
      break;
    case 4  :
      HAL_GPIO_WritePin(GPIOB, LED4_Pin, GPIO_PIN_RESET);
      break;
    case 5  :
      HAL_GPIO_WritePin(GPIOB, LED5_Pin, GPIO_PIN_RESET);
      break;
  }
}

// Used for final test cases that is started from phone
// Some data is hardcoded as phone checks it against known values
void testTransmissionCases(unsigned char caseNum) {
	newData.dataType = 1;		// HitData
	switch(caseNum) {
		case 1: 
			newData.timeMillis = 2007;   // use HAL_GetTick() normally
			newData.timeStamp = 55652;   // when RTC is working this will be seconds (getTimeSeconds())
			newData.gameMode = dataTestFromQueue.gameMode;
			newData.targetPanel = 3;
			newData.hitPanel = 3;
			newData.gameTimer = 20;
			newData.score = 50;
			newData.handicapTime = dataTestFromQueue.handicapTime;
			break;
		case 2:
			newData.timeMillis = 5005;
			newData.timeStamp = 55657;
			newData.gameMode = dataTestFromQueue.gameMode;
			newData.targetPanel = 5;
			newData.hitPanel = 5;
			newData.gameTimer = 75;
			newData.score = 62;
			newData.handicapTime = dataTestFromQueue.handicapTime;
			break;
		case 3:
			newData.timeMillis = 7001;
			newData.timeStamp = 55664;
			newData.gameMode = dataTestFromQueue.gameMode;
			newData.targetPanel = 1;
			newData.hitPanel = 2;
			newData.gameTimer = 81;
			newData.score = 62;
			newData.handicapTime = dataTestFromQueue.handicapTime;
			break;
		case 4:
			newData.timeMillis = 12002;
			newData.timeStamp = 55669;
			newData.gameMode = dataTestFromQueue.gameMode;
			newData.targetPanel = 2;
			newData.hitPanel = 2;
			newData.gameTimer = 93;
			newData.score = 70;
			newData.handicapTime = dataTestFromQueue.handicapTime;
			break;
	}
	// Adding to queue
	if(!isFullHitDataQueue()) {
		addHitDataQueue(newData);
		// Update flag new info in queue
		transmitHitDataFlag = true;
	}
}

// Can be called to continuously add data to queue to be sent
// Change data to be whatever you feel fit
void testTransmission(unsigned char panel) {
	// Testing HitData
	newData.dataType = 1;
	newData.timeMillis = HAL_GetTick();
	newData.timeStamp = 500;
	newData.gameMode = 1;
	newData.targetPanel = panel;
	newData.hitPanel = panel;
	newData.gameTimer = 512;
	newData.score = 136;
	newData.handicapTime = 65;

	// Adding to queue
	if(!isFullHitDataQueue()) {
		addHitDataQueue(newData);
		// Update flag new info in queue
		transmitHitDataFlag = true;
	}
}

// Gets time from RTC and returns seconds
// Needs debug as it doesn't return any value
unsigned int getTimeSeconds(void) {
  HAL_RTC_GetTime(&hrtc, &currTime, RTC_FORMAT_BIN);
  // Time in seconds
  unsigned int temp = 0;
  temp += (currTime.Hours * 3600);
  temp += (currTime.Minutes * 60);
  temp += (currTime.Seconds);
  return temp;
}

/**
  * @brief Will be called when all data is received on uart
  * 	   Used to set flag so code knows data has been received.
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart) {
    dataReceived = true;
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void) {
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /**Initializes the CPU, AHB and APB busses clocks */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL12;
  RCC_OscInitStruct.PLL.PREDIV = RCC_PREDIV_DIV1;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
    Error_Handler();
  }
	
  /**Initializes the CPU, AHB and APB busses clocks */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK) {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART1;
  PeriphClkInit.Usart1ClockSelection = RCC_USART1CLKSOURCE_PCLK1;
  PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void) {
  /**Initialize RTC Only 
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK) {
	  Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void) {
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 57600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart1) != HAL_OK) {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void) {
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 57600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK) {
    Error_Handler();
  }
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void) {
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  
  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LED4_Pin|LED5_Pin|LED1_Pin|LED2_Pin 
                          |LED3_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LED4_Pin LED5_Pin LED1_Pin LED2_Pin 
                           LED3_Pin */
  GPIO_InitStruct.Pin = LED4_Pin|LED5_Pin|LED1_Pin|LED2_Pin 
                          |LED3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void) {
  /* User can add his own implementation to report the HAL error return state */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(char *file, uint32_t line) { 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif
