#include "commons.h"
#include "ring_buff.h"
#include "usart.h"


#define U1_TX 9   // PA
#define U1_RX 10  // PA
#define U2_TX 2   // PA
#define U2_RX 3   // PA
#define U6_TX 6   // PC
#define U6_RX 7   // PC


void __usart1_init(void) {

  RCC->APB2ENR |= RCC_APB2ENR_USART1EN_Msk;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  // alternate function mode
  GPIOA->MODER &= ~((3 << (2 * U1_TX)) | (3 << (2 * U1_RX)));
  GPIOA->MODER |= 2 << (2 * U1_TX) | 2 << (2 * U1_RX);
  // high speed
  GPIOA->OSPEEDR |= (3 << (U1_TX * 2)) | (3 << (U1_RX * 2));
  // clear the bits in AFRH register
  GPIOA->AFR[1] &= ~((0xf << 4) | (0xf << 8));
  // set for af7
  GPIOA->AFR[1] |= (7 << 4) | (7 << 8);

  // set the baud rate (115200 in this case)
  USART1->BRR = 0x08B;

  // enable usart, reciever, transiever
  USART1->CR1 |=    USART_CR1_TE |
                    USART_CR1_RE |
                    USART_CR1_RXNEIE;

  USART1->CR1 |=    USART_CR1_UE;

  // enable usart1 interrupt via NVIC
  NVIC_EnableIRQ(USART1_IRQn);
}



void __usart2_init (void){

  RCC->APB1ENR |= RCC_APB1ENR_USART2EN_Msk;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
  // alternate function mode
  GPIOA->MODER &= ~((3 << (2 * U2_TX)) | (3 << (2 * U2_RX)));
  GPIOA->MODER |= 2 << (2 * U2_TX) | 2 << (2 * U2_RX);
  // high speed
  GPIOA->OSPEEDR |= (3 << (U2_TX * 2)) | (3 << (U2_RX * 2));
  // clear the bits in AFRL register
  GPIOA->AFR[0] &= ~((0xf << 12) | (0xf << 8));
  // set for af7usartusart
  GPIOA->AFR[0] |= (7 << 12) | (7 << 8);

  // set the baud rate (115200 in this case)
  USART2->BRR = 0x08B;

  // enable usart, reciever, transiever
  USART2->CR1 |=    USART_CR1_TE |
                    USART_CR1_RE |
                    USART_CR1_RXNEIE;

  USART2->CR1 |=    USART_CR1_UE;

  // enable usart1 interrupt via NVIC
  NVIC_EnableIRQ(USART2_IRQn);

}

void __usart6_init (void){

  RCC->APB2ENR |= RCC_APB2ENR_USART6EN_Msk;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
  // alternate function mode
  GPIOC->MODER &= ~((3 << (2 * U6_TX)) | (3 << (2 * U6_RX)));
  GPIOC->MODER |= 2 << (2 * U6_TX) | 2 << (2 * U6_RX);
  // high speed
  GPIOC->OSPEEDR |= (3 << (U6_TX * 2)) | (3 << (U6_RX * 2));
  // clear the bits in AFRL register
  GPIOC->AFR[0] &= ~((0xf << 24) | (0xf << 28));
  // set for af7
  GPIOC->AFR[0] |= (8 << 24) | (8 << 28);

  // set the baud rate (115200 in this case)
  USART6->BRR = 0x08B;

  // enable usart, reciever, transiever
  USART6->CR1 |=    USART_CR1_TE |
                    USART_CR1_RE |
                    USART_CR1_RXNEIE;

  USART6->CR1 |=    USART_CR1_UE;

  // enable usart1 interrupt via NVIC
  NVIC_EnableIRQ(USART6_IRQn);

}

void __usart1_print(const char *msg, uint32_t size) {

  int i = 0;
  while (i < size && msg[i] != '\0') {
    while (!(USART1->SR & USART_SR_TXE))
      ;
    USART1->DR = msg[i++];
  }
  while (!(USART1->SR & USART_SR_TC)) {
  }
}

void __usart2_print(const char *msg, uint32_t size) {

  int i = 0;
  while (i < size && msg[i] != '\0') {
    while (!(USART2->SR & USART_SR_TXE))
      ;
    USART2->DR = msg[i++];
  }
  while (!(USART2->SR & USART_SR_TC)) {
  }
}

void __usart6_print(const char *msg, uint32_t size) {

  int i = 0;
  while (i < size && msg[i] != '\0') {
    while (!(USART6->SR & USART_SR_TXE))
      ;
    USART6->DR = msg[i++];
  }
  while (!(USART6->SR & USART_SR_TC)) {
  }
}
void __usart1_scan (volatile Ring_buff_t *rb){ 
    while (!(USART1->SR & USART_SR_RXNE));
    Ring_buff_write (rb, (uint8_t *)(USART1->DR), 1);
}
void __usart2_scan (volatile Ring_buff_t *rb){ 
    while (!(USART2->SR & USART_SR_RXNE));
    Ring_buff_write (rb, (uint8_t *)(USART2->DR), 1);
}
void __usart6_scan (volatile Ring_buff_t *rb){ 
    while (!(USART6->SR & USART_SR_RXNE));
    Ring_buff_write (rb, (uint8_t *)(USART6->DR), 1);
}
