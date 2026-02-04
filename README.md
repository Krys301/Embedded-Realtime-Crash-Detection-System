<h1>Real-Time Crash Detection System (PIC Embedded C)</h1> 

A real-time embedded system designed to detect crash/impact conditions and trigger a fast safety response (airbag system simulation).  
Developed and deployed onto a PIC microcontroller using embedded C, with a focus on reliability, real-time response, and hardware I/O integration.

---

## 🎯 Objective

The objective of this project was to build an embedded safety-critical style system that:

- continuously monitors crash-related sensor input(s)
- detects impact conditions using threshold-based logic
- triggers immediate response outputs (airbag deployment logic + alerts)
- executes reliably under real-time constraints on microcontroller hardware

---

## 🧠 Key Concepts Covered

- Embedded programming in C
- Real-time input monitoring + event detection logic
- Microcontroller I/O (digital outputs, ADC input if applicable)
- Deterministic decision-making (predictable response timing)
- Safety-style logic: latch states, lock-out after deploy, fault-resilient behaviour

---

## 🛠️ Tech Stack

- **PIC microcontroller**
- Embedded C
- (Optional depending on your build)
  - ADC sensor input
  - GPIO outputs (LED/Buzzer)
  - LCD feedback
  - EEPROM logging

---

## ⚙️ System Behaviour (High Level)

### Inputs
- Crash/impact sensor signal (read through microcontroller input)
- System arming state / reset condition
- Threshold detection logic

### Outputs
- Deployment trigger output
- Audible/visual alert outputs (buzzer / LEDs)
- Status feedback (optional LCD)

---

## ✅ Features

- Real-time crash event detection with microcontroller sensor monitoring
- Rapid safety response sequence (deploy + alert)
- Latching behaviour after deployment (prevents repeated triggers)
- Hardware-level validation by running on a physical PIC board

---

## 📂 Files

<div style="background-color:#f5f5f5; padding:15px; border-radius:10px; margin-bottom:10px;">
  <a href="docs/CarAirbagSystemCode.c ">
    <img src="https://img.shields.io/badge/Crash System Code-gray?style=for-the-badge" alt="Web app"/>
  </a>
  <br>

  <div style="background-color:#f5f5f5; padding:15px; border-radius:10px; margin-bottom:10px;">
  <a href="docs/EE302AirbagSystem.pdf">
    <img src="https://img.shields.io/badge/Crash System Report-gray?style=for-the-badge" alt="Web app"/>
  </a>
  </br>
 
---

## ▶️ Build / Run (PIC Workflow)

This project was deployed to a PIC board using a standard embedded workflow:

1. Open the project in the PIC toolchain (e.g., MPLAB X)
2. Configure device + clock settings
3. Build / compile
4. Upload/flash to the PIC board
5. Verify response behaviour using test conditions

---

## ✅ What I Learned

- Developing embedded software for real hardware constraints
- Translating system requirements into reliable control logic
- Debugging microcontroller I/O, timing issues, and edge cases
- Building safety-style logic: deterministic response + deploy lockout

---

## 🚀 Future Improvements

- Convert logic into a formal Finite State Machine (FSM)
- Add noise filtering / debouncing for sensor stability
- Add event logging (EEPROM) for crash history + timestamps
- Add structured testing plan for edge cases and sensor failures
