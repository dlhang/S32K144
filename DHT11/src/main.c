#include "sdk_project_config.h"
#include <stdio.h>
#include <string.h>

/*
 * DHT11 GPIO configuration
 * Data pin of DHT11 is connected to PTD10
 */
#define DHT_PIN         10U
#define DHT_PORT        PTD

/* LPIT channel used for periodic timer */
#define LPIT_CHANNEL        0UL
#define LPIT_Channel_IRQn   LPIT0_Ch0_IRQn


/*
 * Simple delay in microseconds
 * Approximation based on 80 MHz CPU clock
 */
void delay_us(uint32_t us)
{
    volatile uint32_t count;
    while (us--)
    {
        count = 8;   // ~1 us delay at 80 MHz
        while (count--) __asm("nop");
    }
}

/*
 * Simple delay in milliseconds
 */
void delay_ms(uint32_t ms)
{
    while(ms--)
    {
        for (volatile uint32_t i = 0; i < 8000; i++)
            __asm("nop");
    }
}


/*
 * Send string via LPUART1 (blocking mode)
 */
void uart_print(char *str)
{
    LPUART_DRV_SendDataBlocking(1, (uint8_t *)str, strlen(str), 1000);
}


/*
 * Wait for DHT11 response sequence
 *
 * After MCU start signal, DHT11 will respond with:
 *  - 80us LOW
 *  - 80us HIGH
 *
 * Function waits for this sequence to confirm sensor response
 */
bool dht11_wait_response(void)
{
    uint32_t timeout = 200000;

    /* Wait for DHT11 to pull line LOW */
    while (PINS_DRV_ReadPins(DHT_PORT) & (1UL << DHT_PIN))
        if (--timeout == 0) return false;

    /* Wait for DHT11 to pull line HIGH */
    timeout = 200000;
    while (!(PINS_DRV_ReadPins(DHT_PORT) & (1UL << DHT_PIN)))
        if (--timeout == 0) return false;

    /* Wait for DHT11 to release line LOW again */
    timeout = 200000;
    while (PINS_DRV_ReadPins(DHT_PORT) & (1UL << DHT_PIN))
        if (--timeout == 0) return false;

    return true;
}


/*
 * Send start signal to DHT11
 *
 * MCU pulls line LOW for at least 18 ms
 * Then releases line and waits for sensor response
 */
void dht11_start(void)
{
    /* Configure pin as output */
    PINS_DRV_SetPinDirection(DHT_PORT, DHT_PIN, GPIO_OUTPUT_DIRECTION);

    /* Pull line LOW */
    PINS_DRV_WritePin(DHT_PORT, DHT_PIN, 0);
    delay_ms(18);

    /* Release line */
    PINS_DRV_WritePin(DHT_PORT, DHT_PIN, 1);
    delay_us(30);

    /* Switch pin to input to read sensor response */
    PINS_DRV_SetPinDirection(DHT_PORT, DHT_PIN, GPIO_INPUT_DIRECTION);
}


/*
 * Read one bit from DHT11
 *
 * Bit format:
 *  - 50us LOW
 *  - HIGH duration determines bit value
 *      ~26-28us -> bit 0
 *      ~70us    -> bit 1
 */
uint8_t dht11_read_bit(void)
{
    uint32_t timeout = 10000;

    /* Wait for start of HIGH signal */
    while (!(PINS_DRV_ReadPins(DHT_PORT) & (1UL << DHT_PIN)))
    {
        if (--timeout == 0) return 0xFF;
    }

    /* Wait 30us then sample signal */
    delay_us(30);

    uint8_t bit = (PINS_DRV_ReadPins(DHT_PORT) & (1UL << DHT_PIN)) ? 1 : 0;

    /* Wait until line goes LOW again */
    timeout = 10000;
    while (PINS_DRV_ReadPins(DHT_PORT) & (1UL << DHT_PIN))
    {
        if (--timeout == 0) break;
    }

    return bit;
}


/*
 * Read temperature and humidity from DHT11
 *
 * DHT11 sends 40 bits:
 *  - 8  bits humidity integer
 *  - 8  bits humidity decimal
 *  - 8  bits temperature integer
 *  - 8  bits temperature decimal
 *  - 8  bits checksum
 */
bool dht11_read(uint8_t *temp, uint8_t *humi)
{
    uint8_t data[5] = {0};

    dht11_start();

    if (!dht11_wait_response())
        return false;

    /* Read 40 bits */
    for (int i = 0; i < 40; i++)
    {
        uint8_t b = dht11_read_bit();
        if (b == 0xFF)
            return false;

        data[i / 8] <<= 1;
        data[i / 8] |= b;
    }

    /* Extract humidity and temperature */
    *humi = data[0];
    *temp = data[2];

    return true;
}


int main(void)
{
    status_t status;

    /* Initialize system clock */
    status = CLOCK_DRV_Init(&clockMan1_InitConfig0);
    DEV_ASSERT(status == STATUS_SUCCESS);

    /* Initialize pin configuration */
    status = PINS_DRV_Init(NUM_OF_CONFIGURED_PINS0, g_pin_mux_InitConfigArr0);
    DEV_ASSERT(status == STATUS_SUCCESS);

    /*
     * Initialize LPIT (Low Power Interrupt Timer)
     */
    LPIT_DRV_Init(INST_LPIT_CONFIG_1, &lpit1_InitConfig);

    /* Configure LPIT channel as periodic timer */
    status = LPIT_DRV_InitChannel(INST_LPIT_CONFIG_1, LPIT_CHANNEL, &lpit1_ChnConfig0);
    DEV_ASSERT(status == STATUS_SUCCESS);

    /* Initialize LPUART1 for debug output */
    LPUART_DRV_Init(1, &INST_LPUART_LPUART_1, &lpuart_1_InitConfig0);

    uart_print("UART OK\r\n");

    uint8_t temp, humi;

    /* Main loop */
    for(;;)
    {
        if (dht11_read(&temp, &humi))
        {
            char buf[64];
            sprintf(buf, "Temp = %d \r\nHumi = %d \r\n", temp, humi);
            uart_print(buf);
        }
        else
        {
            uart_print("DHT11 ERROR\n");
        }

        /* Read sensor every 2 seconds */
        delay_ms(2000);
    }
}
