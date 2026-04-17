# ✈️ Aircraft Telemetry System

## 📌 Overview

The **Aircraft Telemetry System** is a distributed C++ application designed to simulate and monitor aircraft fuel consumption in real-time.

It follows a **client-server architecture**, where multiple aircraft (clients) send telemetry data over TCP/IP to a central monitoring server.

---

## 🏗️ System Architecture

### 🔹 Telemetry Server

The server handles multiple concurrent client connections and performs real-time data processing.

**Components:**

* **ServerNetwork**
  Manages the TCP listening socket and creates a detached thread for each aircraft connection.

* **FuelCalculator**
  A thread-safe shared module that:

  * Tracks fuel levels
  * Calculates consumption rates
  * Generates flight summaries

---

### 🔹 Telemetry Client

Each client simulates an aircraft sending telemetry data.

**Components:**

* **ClientNetwork**
  Handles connection to the server and binary packet transmission.

* **FileReader**

  * Reads telemetry data from a CSV file
  * Simulates real-time transmission (1-second delay)
  * Converts data into a shared packet format

---

## 🔌 Communication Protocol

The system uses a **fixed-size binary structure** for efficient communication.

* **SharedPacket.h** defines the `TelemetryPacket` structure
* Uses `#pragma pack(push, 1)` to ensure consistent memory layout
* Ensures compatibility across compilers and platforms

### 📡 Data Flow

1. Client sends telemetry packets
2. Server processes incoming data
3. Flight ends with `isEOF` flag
4. Server generates final report

---

## ⚙️ Build & Requirements

* **Language:** C++11 or higher
* **Platform:** Windows
* **Networking:** Winsock2
* **Library:** `ws2_32.lib` (linked via pragma)

---

## 🚀 Getting Started

### ▶️ Run the Server

```bash
ServerProject.exe
```

Default port: **8080**

---

### ▶️ Run a Client

```bash
ClientProject.exe <Server_IP> 8080 
```


---

## 📁 Project Structure (Example)

```
project/
├── src/
│   ├── Server/
│   ├── Client/
├── include/
│   ├── SharedPacket.h
├── README.md
```

---

## 📊 Features

* Real-time telemetry simulation
* Multi-client support using threading
* Efficient binary communication
* Thread-safe data processing
* Automatic flight summary generation

---

## 🛠️ Future Improvements

* Add GUI dashboard for visualization
* Support cross-platform networking
* Implement secure communication (TLS)
* Add logging and analytics

---

## 👨‍💻 Author

Group 11

## 📅 Date

2026-04-17
