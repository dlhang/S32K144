# S32K144 Learning Projects

This repository contains **exercise projects and example firmware** developed while learning the **NXP S32K144 microcontroller**.

The purpose of this repository is to explore and practice embedded system development using different peripherals available on the S32K144 MCU.

---

## Hardware Platform

- **Microcontroller:** NXP S32K144
- **Development Board:** FPT Automotive S32K144 Development Board

---

## Development Environment

- **IDE:** S32 Design Studio
- **Programming Language:** C
- **SDK:** NXP S32K SDK

---

## Projects

### UART_Motor

Example project demonstrating **motor control via UART communication**.

Features:
- UART-based command interface
- Basic motor start/stop control
- Embedded control logic implementation

---

## Repository Structure

```
S32K144
│
├── UART_Motor
│   ├── src
│   ├── board
│   ├── Generated_Code
│   └── Project_Settings
│
└── README.md
```

Each folder represents an individual **exercise project** focused on a specific peripheral or embedded concept.

---

## Building the Project

1. Open **S32 Design Studio**
2. Import the project
   
    File → Import → Existing Projects into Workspace

4. Select the project directory (e.g., `UART_Motor`)
5. Build the project
6. Flash the firmware to the **S32K144 development board**

---

## Future Exercises

Additional learning projects may include:

- GPIO control
- PWM signal generation
- ADC data acquisition
- CAN communication
- SPI communication

---

## Disclaimer

These projects are created for **learning and experimentation purposes** and are not intended for production use.

---

## Author

dlhang
