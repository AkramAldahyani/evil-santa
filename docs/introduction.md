# Introduction

Welcome to the **Evil Santa** documentation. This project exists to explain the **Evil Twin Wi-Fi attack** — how it works, how to recognize it, and how to defend against it.

---

## What This Is

An Evil Twin attack is a wireless technique where an attacker creates a fake access point that imitates a legitimate Wi-Fi network in order to trick people into connecting. From there, an attacker can intercept traffic or attempt to steal credentials.

These docs cover the concept, the hardware commonly involved (the ESP32), and — most importantly — the **detection and defense** skills that let you protect yourself and others.

---

## Important: Scope and Boundaries

This is a real wireless technique, not a harmless toy. Anything hands-on must stay inside strict limits:

- Only on **networks and devices you own**, or
- Inside a **physically isolated lab**, or
- Under a **signed, authorized engagement**.

Targeting other people's networks — cloning their SSID, knocking their devices offline, or capturing their passwords — is illegal and harmful. Please read the [Safety, Ethics, and Legal Use](./safety-rules.md) page before anything else.

---

## Lab Setup

Before you can run the demo hands-on, you will need the following:

- **A PC or laptop** — used to upload the code to the ESP32 via Arduino IDE.
- **An ESP32 device** — the microcontroller that runs the demo. See [What Is the ESP32?](./what-is-esp32.md) for details and purchase links.
- **A USB cable** — used to connect the ESP32 to your computer for code upload. The connector type depends on the board variant you bought — most modern boards use **Type-C**, though some older ones use Micro-USB. Check your board before buying a cable.
- **Arduino IDE** — the software used to upload the code to the ESP32. See [Setting Up Arduino IDE](./setting-up-arduino-ide.md) for the installation guide.
- **A Wi-Fi network you are authorized to use** — the demo must only be run on a network you own or have explicit permission to test on. Never run it near other people's networks.

---

## Recommended Reading Order

Start with the concept, then move to defense and the supporting background:

1. [What Is an Evil Twin Attack?](./what-is-an-evil-twin.md) — the core concept, explained simply with diagrams
2. [Detecting and Defending Against Evil Twin Attacks](./detection-and-defense.md) — how to spot one and protect yourself
3. [Safety, Ethics, and Legal Use](./safety-rules.md) — the boundaries that must never be crossed
4. [What Is the ESP32?](./what-is-esp32.md) — background on the hardware involved
5. [Connecting the ESP32 to Your Computer](./connecting-esp32.md) — identifying its COM port
6. [Setting Up Arduino IDE](./setting-up-arduino-ide.md) — installing the development environment
7. [Adding ESP32 Support to Arduino IDE](./adding-esp32-to-arduino-ide.md) — configuring the toolchain and uploading the code
8. [Running the Demo](../demo/running-the-demo.md) — powering the device and running the awareness demo

---

## The Goal

The point of this material is **awareness and defense**. The more people who can recognize a fake network and refuse to hand over their credentials, the less effective these attacks become.

---

➡️ **Next:** [What Is an Evil Twin Attack?](./what-is-an-evil-twin.md)
