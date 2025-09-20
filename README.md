# Arche Engine

![CMake](https://img.shields.io/badge/build-CMake-blue.svg)  
![C++](https://img.shields.io/badge/language-C%2B%2B20-brightgreen.svg)  
![License: MIT](https://img.shields.io/badge/license-MIT-yellow.svg)  
![Platform](https://img.shields.io/badge/platform-cross--platform-lightgrey.svg)

---

**Arche Engine** is a modular physics and rendering engine designed for experimentation, learning, and extensibility. The project is built in modern **C++**, with an emphasis on clean architecture, separation of concerns, and ease of debugging.

---

## 🚀 Features (Phase 1 MVP)

- **Physics Core**  
  - Fixed timestep simulation  
  - Basic particle system with gravity  

- **GUI (ImGui-powered)**  
  - **Log Panel** – real-time engine logs  
  - **2D Viewport** – visualize particles and their motion  
  - **Performance Metrics** – FPS, CPU, and memory utilization  

- **Diagnostics**  
  - Scoped RAII timers for performance measurement  
  - Modular design for future extensions  

---

## 📂 Project Structure

**Arche Engine** is a modular physics and rendering engine designed for experimentation, learning, and extensibility. The project is built in modern **C++**, with an emphasis on clean architecture, separation of concerns, and ease of debugging.

---

## 🚀 Features (Phase 1 MVP)

- **Physics Core**  
  - Fixed timestep simulation  
  - Basic particle system with gravity  

- **GUI (ImGui-powered)**  
  - **Log Panel** – real-time engine logs  
  - **2D Viewport** – visualize particles and their motion  
  - **Performance Metrics** – FPS, CPU, and memory utilization  

- **Diagnostics**  
  - Scoped RAII timers for performance measurement  
  - Modular design for future extensions  

---

## 📂 Project Structure

/engine → Core physics and simulation code
/gui → ImGui-based user interface
/app → Example application (ties engine + GUI together)


---

## 🛠️ Build Instructions

The project uses **CMake** for cross-platform builds.

```bash
git clone https://github.com/yourusername/arche-engine.git
cd arche-engine
cmake -S . -B build
cmake --build build
```

Run the sample application:

```bash
./build/app/arche_app
```

## 🎯 Goals

Provide a foundation for experimenting with physics, rendering, and system architecture.

Build a clean and extensible engine that can scale with future features.

Learn and apply best practices in C++ and software design.

## 📌 Roadmap

Phase 0 – Core math + diagnostics

Phase 1 – MVP GUI + particle simulation

Phase 2 – Expand physics kernel & rendering capabilities

Phase 3 – Advanced debugging tools & modular extensions

## 📸 Screenshots (Coming Soon)

A visual preview of the engine in action will be added here.

## 📜 License

This project is licensed under the MIT License
.
