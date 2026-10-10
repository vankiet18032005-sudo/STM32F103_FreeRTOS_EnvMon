# STM32F103 FreeRTOS Environmental Monitor (V3.0)

Embedded firmware project built on **STM32F103C8T6 (Blue Pill)** using **FreeRTOS (CMSIS-RTOS V2)** and **STM32CubeIDE**, designed to monitor environmental parameters in real-time.


## Key Features
* **Real-Time Multi-Tasking Architecture (FreeRTOS):** Divided into 5 independent priority-managed tasks (DHT Sensor, OLED Display, Button Handler, LED Status, and Error Management).
* **Sensor Acquisition:** Interfaces with the DHT11 sensor to read temperature and humidity.
* **Display Interface:** Renders real-time data onto an I2C-based OLED screen.
* **Peripherals & Communication:** Utilizes I2C1 (Fast-Mode 400kHz with DMA TX), USART1, GPIO interrupts, and Independent Watchdog (IWDG) for system reliability.
* **Inter-Task Communication:** Uses FreeRTOS message queues and synchronization primitives for safe resource sharing.



## Project Structure
text
STM32F103_FreeRTOS_EnvMon/
├── Core/
│   ├── Inc/         # Header files (tasks.h, dht11.h, oled.h, key.h, led.h, etc.)
│   └── Src/         # Source files (main.c, freertos.c, app_tasks.c, peripherals drivers)
├── Drivers/         # STM32F1xx HAL Drivers & CMSIS
├── Middlewares/     # FreeRTOS Kernel (CMSIS_RTOS_V2)
└── README.md
