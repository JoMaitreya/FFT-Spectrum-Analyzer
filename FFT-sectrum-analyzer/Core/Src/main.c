#include "main.h"
#include "arm_math.h"
#include <stdio.h>
#include "stm32f4xx.h"
#define FFT_LENGTH          256           // Increased FFT length
#define SAMPLING_FREQ       1000.0f       // Hz
#define TEST_SIGNAL_FREQ    50.0f         // Adjusted frequency for clarity
#define AMPLITUDE           1.0f           // Signal amplitude
#define PI                  3.14159265358979f  // Define PI
UART_HandleTypeDef huart2;

// Define the FFT handle structure
typedef struct {
    float32_t input[FFT_LENGTH];
    float32_t output[FFT_LENGTH * 2];
    float32_t magnitude[FFT_LENGTH / 2];
    uint32_t peak_bin;
    float32_t peak_value;
} FFT_HandleTypeDef;

arm_cfft_instance_f32 fft_instance;  // Declare the FFT instance
FFT_HandleTypeDef fft_handle;         // Correct declaration of FFT handle

static void generate_test_signal(FFT_HandleTypeDef* fft_handle);
static void process_fft(FFT_HandleTypeDef* fft_handle);
static void analyze_spectrum(FFT_HandleTypeDef* fft_handle);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
void SystemClock_Config(void);
void UART_SendString(char *str);
int main(void)
{
    HAL_Init();
    // Initialize system clock and peripherals...
    SystemClock_Config();
    MX_GPIO_Init();
    MX_USART2_UART_Init();
    printf("FFT Analyzer Starting...\n");
    printf("Sampling Frequency: %.1f Hz\n", SAMPLING_FREQ);
    printf("Signal Frequency: %.1f Hz\n", TEST_SIGNAL_FREQ);
    printf("FFT Length: %d points\n\n", FFT_LENGTH);
    // FFT Initialization
    arm_cfft_init_f32(&fft_instance, FFT_LENGTH);

    while (1)
    {
        // Generate and process signal
        generate_test_signal(&fft_handle);
        process_fft(&fft_handle);
        analyze_spectrum(&fft_handle);

        // Output FFT magnitudes
        printf("FFT Magnitudes:\n");
        for (int i = 0; i < FFT_LENGTH / 2; i++) {
            printf("Bin %d: %.4f\n", i, fft_handle.magnitude[i]);
        }
        // Peak frequency and magnitude
        float32_t peak_freq = (float32_t)fft_handle.peak_bin * SAMPLING_FREQ / FFT_LENGTH;
        printf("Peak detected:\nFrequency: %.2f Hz\nMagnitude: %.4f\n", peak_freq, fft_handle.peak_value);

        char buffer[100];
        sprintf(buffer, "FFT Magnitudes:\n");
        UART_SendString(buffer);
        for (int i = 0; i < FFT_LENGTH / 2; i++) {
            sprintf(buffer, "Bin %d: %.4f\n", i, fft_handle.magnitude[i]);
            UART_SendString(buffer);
        }
        // Peak frequency and magnitude
        //float32_t peak_freq = (float32_t)fft_handle.peak_bin * SAMPLING_FREQ / FFT_LENGTH;
        sprintf(buffer, "Peak detected:\nFrequency: %.2f Hz\nMagnitude: %.4f\n", peak_freq, fft_handle.peak_value);
        UART_SendString(buffer);

        // Visual indicator (e.g., LED toggle)
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
        HAL_Delay(50);  // Delay for readability
    }
}

/*static void generate_test_signal(FFT_HandleTypeDef* fft_handle)
{
    float32_t phase = 0.0f;  // Track phase to ensure continuity
    float32_t dt = 1.0f / SAMPLING_FREQ;

    for (int i = 0; i < FFT_LENGTH; i++)
    {
        // Generate pure sine wave
        fft_handle->input[i] = AMPLITUDE * arm_sin_f32(2.0f * PI * TEST_SIGNAL_FREQ * i * dt);

        // Apply Hanning window
        float32_t window = 0.5f * (1.0f - arm_cos_f32(2.0f * PI * i / (FFT_LENGTH - 1)));
        fft_handle->input[i] *= window;

        // Prepare FFT buffer
        fft_handle->output[2 * i] = fft_handle->input[i];
        fft_handle->output[2 * i + 1] = 0.0f;
    }
}*/
static void generate_test_signal(FFT_HandleTypeDef* fft_handle){
    // Send a header to mark the start of new data
    UART_SendString("START_SIGNAL\n");
    float32_t dt = 1.0f / SAMPLING_FREQ;
    for (int i = 0; i < FFT_LENGTH; i++){
        // Generate sine wave
        float32_t t = (float32_t)i * dt;
        fft_handle->input[i] = AMPLITUDE * arm_sin_f32(2.0f * PI * TEST_SIGNAL_FREQ * t);
        // Apply Hanning window
        float32_t window = 0.5f * (1.0f - arm_cos_f32(2.0f * PI * i / (FFT_LENGTH - 1)));
        fft_handle->input[i] *= window;
        // Prepare FFT buffer
        fft_handle->output[2 * i] = fft_handle->input[i];
        fft_handle->output[2 * i + 1] = 0.0f;
        // Send raw signal data with clear formatting
        char buffer[50];
        sprintf(buffer, "RAW:%d:%.6f\n", i, fft_handle->input[i]);
        UART_SendString(buffer);
    }
    UART_SendString("END_SIGNAL\n");
}
static void process_fft(FFT_HandleTypeDef* fft_handle){
    // Perform FFT
    arm_cfft_f32(&fft_instance, fft_handle->output, 0, 1);
    // Calculate magnitude spectrum with proper scaling
    float32_t scaling = 2.0f / FFT_LENGTH;  // Scale factor for amplitude correction
    for (int i = 0; i < FFT_LENGTH / 2; i++){
        float32_t real = fft_handle->output[2 * i];
        float32_t imag = fft_handle->output[2 * i + 1];

        // Compute magnitude with proper scaling
        fft_handle->magnitude[i] = scaling * sqrtf(real * real + imag * imag);
    }
    // DC component should only be scaled by 1/N
    fft_handle->magnitude[0] *= 0.5f;
}
static void analyze_spectrum(FFT_HandleTypeDef* fft_handle){
    fft_handle->peak_value = 0;
    fft_handle->peak_bin = 0;
    for (int i = 1; i < FFT_LENGTH / 2; i++){
        if (fft_handle->magnitude[i] > fft_handle->peak_value){
            fft_handle->peak_value = fft_handle->magnitude[i];
            fft_handle->peak_bin = i;
        }
    }
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
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

    /** Initializes the RCC Oscillators according to the specified parameters
    * in the RCC_OscInitTypeDef structure.
    */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM = 16;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
    RCC_OscInitStruct.PLL.PLLQ = 2;
    RCC_OscInitStruct.PLL.PLLR = 2;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
    */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
        | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        Error_Handler();
    }
}

static void MX_USART2_UART_Init(void){

  huart2.Instance = USART2;
  huart2.Init.BaudRate = 921600;
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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}
void UART_SendString(char *str) {
    // Use HAL_UART_Transmit to send the string
    HAL_UART_Transmit(&huart2, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}
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
