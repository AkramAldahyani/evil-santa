<div align="center">
  <img src="diagrams/logo.png" alt="Evil Santa Logo" width="900"/>

  # Evil Santa

  **An Evil Twin Wi-Fi attack awareness project**

  Built on an ESP32 · For education and defense only

</div>

---

## What Is This?

Evil Santa is an awareness project that demonstrates how **Evil Twin Wi-Fi attacks** work. An Evil Twin attack is when an attacker creates a fake access point that looks exactly like a legitimate Wi-Fi network to trick people into connecting.

This project uses a small, inexpensive **ESP32** device to simulate that concept in a safe and isolated environment. When someone connects to the fake network, they land on an awareness page that explains what just happened and how to protect themselves.

No credentials are collected. No real networks are targeted. No data leaves the device.

---

## What Is Included

| Folder | Contents |
|---|---|
| `docs/` | Full documentation — concept, defense, hardware setup, and Arduino guides |
| `demo/` | Step-by-step guide for running the demo |
| `diagrams/` | Visual diagrams explaining the attack and defense workflow |
| `screenshots/` | Screenshots used throughout the documentation |
| `evil_santa/scanner/` | The ESP32 Arduino sketch that runs the demo |

---

## How It Works

1. The ESP32 broadcasts a Wi-Fi network named **"Santa"**
2. A nearby device connects to it
3. The user is automatically redirected to an awareness page
4. The page explains the Evil Twin concept and teaches defensive habits
5. The user disconnects — informed and aware

---

## Documentation

Start here and follow the reading order:

1. [Introduction](docs/introduction.md)
2. [What Is an Evil Twin Attack?](docs/what-is-an-evil-twin.md)
3. [Detecting and Defending Against Evil Twin Attacks](docs/detection-and-defense.md)
4. [Safety, Ethics, and Legal Use](docs/safety-rules.md)
5. [What Is the ESP32?](docs/what-is-esp32.md)
6. [Connecting the ESP32 to Your Computer](docs/connecting-esp32.md)
7. [Setting Up Arduino IDE](docs/setting-up-arduino-ide.md)
8. [Adding ESP32 Support to Arduino IDE](docs/adding-esp32-to-arduino-ide.md)
9. [Running the Demo](demo/running-the-demo.md)

---

## Important

This project must only be used on networks and devices **you own** or have **explicit written permission** to test. Running an Evil Twin attack against real networks or non-consenting people is illegal.

Read the full [Disclaimer](DISCLAIMER.md) before using this project.

---

## License

See [LICENSE](LICENSE) for details.
