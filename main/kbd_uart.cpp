#include "freertos/idf_additions.h"
#include "esp_log.h"
#include "freertos/queue.h"
#include "freertos/task.h"
#include "driver/uart.h"
#include "soc/uart_reg.h"
#ifdef __cplusplus
extern "C" {
#include "bsp/input.h"
}
#endif

#define UART_DEBUG

// Constants
static char const* TAG = "uart";

#define MY_UART UART_NUM_0
#define RD_BUF_SIZE 256
#define UART_EMPTY_THRESH_DEFAULT  (10)
#define UART_FULL_THRESH_DEFAULT  (120)
#define UART_TOUT_THRESH_DEFAULT   (10)
#define UART_CLKDIV_FRAG_BIT_WIDTH  (3)
#define UART_TOUT_REF_FACTOR_DEFAULT (UART_CLK_FREQ/(REF_CLK_FREQ<<UART_CLKDIV_FRAG_BIT_WIDTH))
#define UART_TX_IDLE_NUM_DEFAULT   (0)
#define UART_PATTERN_DET_QLEN_DEFAULT (10)
#define UART_MIN_WAKEUP_THRESH      (2)

static QueueHandle_t uart_queue;

static void handle_data(uint8_t *data)
{
    bsp_input_event_t event;
    event.type = INPUT_EVENT_TYPE_SCANCODE;
    event.args_scancode.scancode = (bsp_input_scancode_t) data[0];
    bsp_input_inject_event(&event);
}

static void uartTask(void *pvParameter) {
    uart_event_t event;
    uint8_t* dtmp = (uint8_t*) malloc(RD_BUF_SIZE);

    for(;;) {
        //Waiting for UART event.
        if (xQueueReceive(uart_queue, &event, portMAX_DELAY)) {
            switch(event.type) {
                //Event of UART receving data
                /*We'd better handler data event fast, there would be much more data events than
                other types of events. If we take too much time on data event, the queue might
                be full.*/
                case UART_DATA:
                    uart_read_bytes(MY_UART, dtmp, event.size, portMAX_DELAY);
#ifdef UART_DEBUG
                    for (int i = 0; i < event.size; i++) {
                        ESP_LOGI(TAG, "data[%2d] = %02X", i, dtmp[i]);
                    }
#endif
                    handle_data(dtmp);
                    break;
                //Event of HW FIFO overflow detected
                case UART_FIFO_OVF:
                    ESP_LOGW(TAG, "hw fifo overflow");
                    // If fifo overflow happened, you should consider adding flow control for your application.
                    // The ISR has already reset the rx FIFO,
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(MY_UART);
                    xQueueReset(uart_queue);
                    break;
                //Event of UART ring buffer full
                case UART_BUFFER_FULL:
                    ESP_LOGW(TAG, "ring buffer full");
                    // If buffer full happened, you should consider encreasing your buffer size
                    // As an example, we directly flush the rx buffer here in order to read more data.
                    uart_flush_input(MY_UART);
                    xQueueReset(uart_queue);
                    break;
                //Event of UART RX break detected
                case UART_BREAK:
                    ESP_LOGI(TAG, "uart rx break");
                    break;
                //Event of UART parity check error
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart parity error");
                    break;
                //Event of UART frame error
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart frame error");
                    break;
                //UART_PATTERN_DET
                case UART_PATTERN_DET:

                    break;
                //Others
                default:
                    ESP_LOGI(TAG, "uart event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = NULL;
    vTaskDelete(NULL);
}

#ifdef __cplusplus
extern "C" {
#endif
void my_uart_init() {
    //uart_param_config(MY_UART, &uart_config);   //Configure the uart hardware
    //uart_set_pin(MY_UART, CONFIG_DRIVER_FSOVERBUS_UART_TX, CONFIG_DRIVER_FSOVERBUS_UART_RX, CONFIG_DRIVER_FSOVERBUS_UART_CTS, -1); //Change pins
    uart_config_t uartcfg = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .rx_flow_ctrl_thresh = 0,      // new in ESP-IDF 5.x
        .source_clk = UART_SCLK_DEFAULT,
        .flags = 0                     // new in ESP-IDF 5.x
    };

    uart_param_config(MY_UART, &uartcfg);

    // Install driver: RX buffer, TX buffer, event queue
    uart_driver_install(
        MY_UART,
        RD_BUF_SIZE,     // RX buffer
        RD_BUF_SIZE,     // TX buffer
        40,              // event queue size
        &uart_queue,
        0                // no flags
    );
    xTaskCreatePinnedToCore(uartTask, "fsoverbus_uart", 16000, NULL, 8, NULL, 0);
}
#ifdef __cplusplus
}
#endif
