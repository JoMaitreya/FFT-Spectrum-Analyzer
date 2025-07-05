# FFT Spectrum Analyzer

A real-time FFT Spectrum Analyzer built using STM32CubeIDE (STM32F446RE) and Python.

## 📌 Project Overview

This project captures analog signals using the STM32F446RE's ADC, computes the FFT, and visualizes it either via the ITM console or a connected interface. A Python script is also provided to perform FFT analysis or validate results.

## 🧰 Contents

- `stm32-project/`: STM32CubeIDE firmware (based on CubeMX)
- `fft.py`: Python script to simulate or process FFT from test data or UART dump

## 🛠️ STM32 Features

- STM32F446RE microcontroller
- Analog signal acquisition via ADC
- FFT implementation using CMSIS-DSP or custom fixed-point method
- Output to ITM or serial

## 🐍 Python Script

Use the `fft.py` script for offline FFT testing or signal simulation.

```bash
python fft.py
```

## 🚀 Getting Started

1. Clone the repo:
    ```bash
    git clone https://github.com/yourusername/fft-spectrum-analyzer.git
    cd fft-spectrum-analyzer
    ```

2. Open the STM32 project in STM32CubeIDE (`stm32-project` folder)

3. Build and flash it to your STM32 Nucleo F446RE

4. View FFT output via ITM or serial plotter

## 🖼️ Demo

_Add demo images or videos if available_

## 📄 License

MIT (or choose another license)