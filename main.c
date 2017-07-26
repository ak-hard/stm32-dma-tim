#define STM32F723xx
#include <stm32f7xx.h>

#define SYSCLK_FREQ 16000000 // HSI in F7 is 16 MHz
#define LED_PIN 5
#define LED_PORT GPIOA

const struct
{
	TIM_TypeDef *tim;
	DMA_TypeDef *dma;
	DMA_Stream_TypeDef *stream;
	unsigned channel;
} CFG = {
// uncomment the needed combination below, only TIM1 and TIM8 work
//	TIM1, DMA2, DMA2_Stream5, 6
	TIM8, DMA2, DMA2_Stream1, 7
//	TIM2, DMA1, DMA1_Stream1, 3
//	TIM2, DMA1, DMA1_Stream7, 3
//	TIM3, DMA1, DMA1_Stream2, 5
//	TIM4, DMA1, DMA1_Stream6, 2
//	TIM5, DMA1, DMA1_Stream0, 6
//	TIM5, DMA1, DMA1_Stream6, 6
//	TIM6, DMA1, DMA1_Stream1, 7
//	TIM7, DMA1, DMA1_Stream2, 1
//	TIM7, DMA1, DMA1_Stream4, 1
};

extern unsigned long _estack;
void start(void);

struct CoreInterruptVector
{
	unsigned long *estack;
	void (*Reset_Handler)(void);
};

__attribute__ ((section(".isr_vector_core")))
const struct CoreInterruptVector isrs_core =
{
	&_estack,
	.Reset_Handler = start,
};

uint32_t delay_clk(uint32_t t0, uint32_t n)
{
	uint32_t t = SysTick->VAL;
	while (((t0 - t) & 0xffffffu) < n)
		t = SysTick->VAL;
	return t;
}

void delay_ms(uint32_t n)
{
	uint32_t t0 = SysTick->VAL;
	const uint32_t clk_per_unit = SYSCLK_FREQ / 1000;
	const uint32_t max_delay = 0xffffffu / (clk_per_unit + 1u);
	while (n >= max_delay)
	{
		t0 = delay_clk(t0, clk_per_unit * max_delay);
		n -= max_delay;
	}
	if (n != 0)
		delay_clk(t0, clk_per_unit * n);
}

enum
{
	DMA_SxCR_DIR_P2M = 0,
	DMA_SxCR_PSIZE_WORD = DMA_SxCR_PSIZE_1,
	DMA_SxCR_MSIZE_WORD = DMA_SxCR_MSIZE_1,
};

#define DMA_SxCR_CHSEL_NUM(ch) ((ch) << DMA_SxCR_CHSEL_Pos)

uint32_t buf;

void start(void)
{
	SysTick->LOAD = 0xffffffu;
	SysTick->VAL = 0;
	SysTick->CTRL = 5;

	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN | RCC_APB2ENR_TIM8EN;
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN | RCC_APB1ENR_TIM3EN | RCC_APB1ENR_TIM4EN | RCC_APB1ENR_TIM5EN
			| RCC_APB1ENR_TIM6EN | RCC_APB1ENR_TIM7EN;
	RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN | RCC_AHB1ENR_DMA2EN;

	LED_PORT->MODER   |= 1 << (2 * LED_PIN);
	LED_PORT->OSPEEDR |= 3 << (2 * LED_PIN); // fastest speed

	CFG.tim->CR1 |= TIM_CR1_ARPE;
	CFG.tim->ARR = 16;
	CFG.tim->PSC = 1000;
	CFG.tim->EGR = TIM_EGR_UG; // Generate Update Event to copy ARR to its shadow
	CFG.tim->DIER |= TIM_DIER_UDE;
	CFG.stream->CR |= DMA_SxCR_CHSEL_NUM(CFG.channel) | DMA_SxCR_DIR_P2M | DMA_SxCR_PSIZE_WORD | DMA_SxCR_MSIZE_WORD;
	CFG.stream->NDTR = 16;
	CFG.stream->PAR = (uint32_t) &GPIOA->IDR;
	CFG.stream->M0AR = (uint32_t) &buf;
	CFG.stream->CR |= DMA_SxCR_EN;
	CFG.tim->CR1 |= TIM_CR1_CEN;

	// wait until DMA state changes
	while (CFG.dma->LISR == 0 && CFG.dma->HISR == 0)
		delay_ms(1);

	// check for any TEIFx bits
	int error = (CFG.dma->LISR | CFG.dma->HISR) & 0x02080208;

	while (1)
	{
		LED_PORT->ODR ^= 1 << LED_PIN;
		delay_ms(error ? 100 : 500);
	}
}
