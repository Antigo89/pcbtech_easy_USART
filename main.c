/*
File    : main.c

*/

#include "main.h"

char *pt_buffer;

/****************************** function**********************************/
void send_USART_STR(char *sendbuffer){
  while((USART1->SR & USART_SR_TC) == 0){}
  pt_buffer = &(*sendbuffer);
  USART1->CR1 &= ~(USART_CR1_RXNEIE);
  EXTI->IMR &= ~(EXTI_IMR_IM10|EXTI_IMR_IM11|EXTI_IMR_IM12);
  USART1->DR = (*pt_buffer & (uint16_t)0x01FF);
  USART1->CR1 |= USART_CR1_TCIE;
}

/*********************************main************************************/
int main(void) {
  //Values initial

  //System Initial
  SystemInit();
  RCC_Init();
  // Clock BUS Initial
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOEEN|RCC_AHB1ENR_GPIOAEN;
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN|RCC_APB2ENR_USART1EN;
  SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI10_PE|SYSCFG_EXTICR3_EXTI11_PE;
  SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI12_PE;
  
  // 84MHz / 115200bod / 16 = 45,57  M=45 (0x2D) F=0,57*16=9 (0x09)
  USART1->BRR = 0x02D9;

  //Connections Initial
  USART1->CR1 |= USART_CR1_RXNEIE|USART_CR1_TE|USART_CR1_RE;
  USART1->CR1 &= ~(USART_CR1_M|USART_CR1_PCE);
  USART1->CR2 &= ~(USART_CR2_STOP);

  __enable_irq();
  //GPIO Initial
  GPIOE->MODER |= GPIO_MODER_MODE13_0|GPIO_MODER_MODE14_0|GPIO_MODER_MODE15_0;
  GPIOE->MODER &= ~(GPIO_MODER_MODE10|GPIO_MODER_MODE11|GPIO_MODER_MODE12);
  GPIOA->MODER |= GPIO_MODER_MODE9_1|GPIO_MODER_MODE10_1;
  GPIOA->AFR[1] |= (7<<GPIO_AFRH_AFSEL9_Pos)|(7<<GPIO_AFRH_AFSEL10_Pos);
 
  //Interrupts Initial
  //S1 & S3 & S3
  EXTI->PR |= EXTI_PR_PR10|EXTI_PR_PR11|EXTI_PR_PR12;
  EXTI->FTSR |= EXTI_FTSR_TR10|EXTI_FTSR_TR11|EXTI_FTSR_TR12;
  EXTI->IMR |= EXTI_IMR_IM10|EXTI_IMR_IM11|EXTI_IMR_IM12; 
  
  //Interrupt NVIC Enable
  NVIC_EnableIRQ(EXTI15_10_IRQn);
  NVIC_EnableIRQ(USART1_IRQn);
  
  //Enable USART1
  USART1->CR1 |= USART_CR1_UE;
  //LED turn off
  GPIOE->BSRR |= GPIO_BSRR_BS13|GPIO_BSRR_BS14|GPIO_BSRR_BS15;
  send_USART_STR(" \r\n");

  while(1){
    __NOP();
  }
}
/***********************interrupts function**************************/
void EXTI15_10_IRQHandler(void){
  switch(EXTI->PR & (EXTI_PR_PR10|EXTI_PR_PR11|EXTI_PR_PR12)){
      case EXTI_PR_PR10:
        send_USART_STR("Button S1\r\n");
        break;
      case EXTI_PR_PR11:
        send_USART_STR("Button S2\r\n");
        break;
      case EXTI_PR_PR12:
        send_USART_STR("Button S3\r\n");
        break;
  }
  EXTI->PR |= EXTI_PR_PR10|EXTI_PR_PR11|EXTI_PR_PR12;
}

void USART1_IRQHandler(void){
  if(USART1->SR & USART_SR_TC){
    if(*pt_buffer == '\n'){
      USART1->CR1 &= ~(USART_CR1_TCIE);
      EXTI->IMR |= EXTI_IMR_IM10|EXTI_IMR_IM11|EXTI_IMR_IM12; 
      USART1->CR1 |= USART_CR1_RXNEIE;
    }else{
      USART1->DR = (*++pt_buffer & (uint16_t)0x01FF);
    } 
  }
  if(USART1->SR & USART_SR_RXNE){
    char receive = (uint16_t)USART1->DR & (uint16_t)0x01FF;
    switch (receive) {
      case '1':
      GPIOE->BSRR |= GPIO_BSRR_BR13;
      break;
      case '2':
      GPIOE->BSRR |= GPIO_BSRR_BR14;
      break;
      case '3':
      GPIOE->BSRR |= GPIO_BSRR_BR15;
      break;
      default:
      GPIOE->BSRR |= GPIO_BSRR_BS13|GPIO_BSRR_BS14|GPIO_BSRR_BS15;
      break;
    }
  }
  NVIC_ClearPendingIRQ(USART1_IRQn);
}
/*************************** End of file ****************************/
