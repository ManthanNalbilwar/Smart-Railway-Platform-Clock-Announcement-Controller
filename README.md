# 🚉 Smart Railway Platform Clock & Announcement Controller

An embedded systems project built on the **NXP LPC2148 (ARM7TDMI-S)** microcontroller that functions as an automated railway platform display — showing live date/time, upcoming train schedules, delay status, and audio-visual alerts, with an admin mode for on-site schedule editing.

---

## 📋 Project Overview

This system continuously tracks the current time using the microcontroller's **on-chip Real-Time Clock (RTC)** and compares it against a stored train schedule database. Based on the comparison, it:

- Displays the live **date, time, and day** on one 16x2 LCD.
- Displays the **next train's number, name, platform, and arrival/departure time** on a second 16x2 LCD, with the train name scrolling automatically.
- Drives **LED indicators** (Green / Yellow / Red) to show overall platform status at a glance.
- Sounds a **buzzer** and blinks the schedule line when a train is delayed.
- Allows an **admin** to press an external interrupt switch to enter a keypad-driven menu and update train details or the RTC date/time.

---

## 🧩 Block Diagram

![Block Diagram](images/BD.png)

**Signal flow:**
- **Inputs** → Admin Edit Switch (external interrupt), 4x4 Matrix Keypad, on-chip RTC, and the train schedule table stored in MCU memory.
- **Processing (LPC2148)** → reads the RTC, compares it against the schedule, updates both LCDs, detects delays, drives the LED/buzzer indicators, and handles admin configuration.
- **Outputs** → LCD 1 (clock & date), LCD 2 (train information), 3 status LEDs, and a buzzer.
- A DB9 serial connector is used to program/communicate with the board from a PC during development.

---

## 🔌 Circuit Diagram

![Circuit Diagram](images/Circuit.jpeg)

### Pin Mapping (LPC2148 / LPC2124 core)

| Peripheral         | Function            | MCU Pin        |
|--------------------|---------------------|-----------------|
| Admin Edit Switch   | External Interrupt (EINT0) | P0.1 (SW) |
| Buzzer              | Delay alert         | P0.5 |
| Green LED            | On-time status     | P0.2 |
| Yellow LED           | Approaching status | P0.3 |
| Red LED              | Delayed status      | P0.4 |
| LCD Data Bus (D0–D7) | 8-bit data          | P0.8 – P0.15 |
| LCD RS                | Register Select    | P0.16 |
| LCD RW                | Read/Write          | P0.17 |
| LCD EN                | Enable              | P0.18 |
| Keypad Rows (R0–R3)   | Row scan            | P1.16 – P1.19 |
| Keypad Columns (C0–C3)| Column scan         | P1.20 – P1.23 |

**Display used:** LM016L 16x2 character LCD (x2)
**Keypad used:** 4x4 Matrix Keypad
**Power Supply:** 5V DC Regulated

---

## ⚙️ Hardware Components

| Component | Quantity | Purpose |
|---|---|---|
| LPC2148 Development Board | 1 | Core controller |
| 16x2 LCD (LM016L) | 2 | Clock/date display + Train info display |
| 4x4 Matrix Keypad | 1 | Admin data entry |
| Push Button Switch | 1 | Admin mode trigger (external interrupt) |
| LEDs (Green, Yellow, Red) | 3 | Train status indication |
| Buzzer | 1 | Delay alert |
| DB9 Serial Connector | 1 | PC ↔ MCU programming/communication |
| 5V Regulated Power Supply | 1 | Board power |

---

## 💻 Software / Firmware Structure

```
Smart Railway Platform Clock & Announcement Controller/
├── inc/                    # Header files
│   ├── defines.h           # Bit/byte/nibble macro utilities
│   ├── delay.h             # Delay routines
│   ├── eint_sw.h           # External interrupt (admin switch) config
│   ├── indicator.h         # LED & buzzer pin definitions
│   ├── KPM_defines.h       # Keypad row/column pin mapping
│   ├── kpm_v2.h            # Keypad scanning driver
│   ├── lcd.h / lcddefines.h# LCD driver & command definitions
│   ├── menu.h              # Admin menu & first-time setup
│   ├── Rtc.h               # On-chip RTC driver
│   ├── scheduler.h         # Train status/delay evaluation logic
│   └── train_db.h          # Train schedule data structure
└── src/                    # Source files
    ├── application.c       # main() – system loop & announcement logic
    ├── delay.c
    ├── eint_sw.c
    ├── indicator.c
    ├── kpm_v2.c
    ├── lcd_v2.c
    ├── menu.c
    ├── Rtc_v2.c
    ├── scheduler.c
    └── train_db.c
```

### How It Works

1. On boot, the RTC is initialized. If this is the **first boot**, the system prompts a one-time date/time setup via the keypad.
2. In the main loop, the current time, date, and day are read from the RTC and shown on **LCD-1** for a few seconds.
3. All trains in the schedule (`TOTAL_TRAINS`) are evaluated against the current time:
   - **On Time** → Green LED
   - **Approaching** (within a configurable threshold before arrival) → Yellow LED
   - **Delayed** → Red LED, LCD-2 blinks the schedule line, buzzer/announcement triggers
4. For any train that is approaching or delayed, **LCD-2** displays the train number, scrolling train name, platform number, and arrival/departure time.
5. Pressing the **Admin Edit Switch** at any time fires an external interrupt, pausing normal operation and opening the **Admin Menu**, where the keypad can be used to:
   - Edit a train's arrival/departure time, platform, or delay minutes.
   - Update the RTC date and time.
6. The system returns to normal display operation once admin editing is complete.

---

## 🛠️ Tools & Setup

- **MCU:** NXP LPC2148 / LPC2124 (ARM7TDMI-S)
- **IDE / Toolchain:** Keil µVision (ARM) or equivalent LPC21xx-compatible toolchain
- **Flash Utility:** Flash Magic (via UART/DB9 serial connector)
- **Language:** Embedded C

### Build & Flash Steps
1. Open the project in Keil µVision (or your preferred ARM toolchain).
2. Build the source in `src/` and `inc/` into a `.hex` file.
3. Connect the board to the PC via the DB9 serial connector.
4. Use Flash Magic (or equivalent) to flash the `.hex` file onto the LPC2148.
5. Power the board with a regulated 5V DC supply and observe the LCDs, LEDs, and buzzer.

---

## 📸 Media

- `images/BD.png` — System block diagram
- `images/Circuit.jpeg` — Circuit/schematic diagram
- *(Add photos/videos of the assembled hardware and a working demo here before submission.)*

---

## ✅ Project Submission Checklist

As per the assigned project workflow, before submitting the GitHub repository link on the student portal, confirm:

- [ ] Project implemented and tested on real hardware within the given time frame.
- [ ] Repository includes source code (`src/`, `inc/`).
- [ ] Repository includes circuit details (block diagram + schematic, as above).
- [ ] Repository includes hardware images/demo video.
- [ ] README file (this file) is complete and accurate.
- [ ] GitHub repository link submitted through the student login portal.
- [ ] Ready for hardware verification, GitHub review, and explanation/viva.
- [ ] Any modification requests from the review process are addressed and resubmitted.
- [ ] Final evaluation completed within the assigned project duration.

---

## 👤 Author

- **Student Name:** _[Add your name]_
- **Roll No / ID:** _[Add ID]_
- **Project Duration:** _[Add assigned dates]_
- **Guide/Mentor:** _[Add mentor name]_

---

## 📄 License

This project is submitted as part of an academic embedded systems course assignment.
