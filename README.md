# SmartCoach (aka RV-Bridge): HomeKit to RV-C Adapter 
This is a forked version of Randy Ubillos's RV-Bridge

## Overview
### Credit and Origin
The RV-Bridge was originally developed by Randy Ubillos, you may find the original documentation from him, here at https://github.com/rubillos/RV-Bridge. After reviewing his code, I felt it needed significant work to make it more modular and Object Oriented and less monolithic. I forked from his GitHub and re-architected the design. My GitHub link is:
https://github.com/wahart77399/RV-Bridge

## Development and Hardware
Originally, I used Randy’s original hardware and 3D casing. Unfortunately, the ESP32 board he used did not allow me to build all the devices I wanted. So, I researched and found a different board, the ESP32-S3 with CAN and LIN bus connections from SKPang. You can find the board [here](https://copperhilltech.com/esp32s3-can-lin-bus-board/). Once I made that decision, I decided to add more capability, including:
* Captive Portal for setting up Wifi Credentials
* Device Creation from JSON file
* Coach Specification Creation from JSON file
* Auto Discovery of devices
* Web Portal
     * Learning / listening of devices on RVC CAN bus.
     * Coach Spec editing
     * Name assignment and enabling of devices
     * Listening of CAN bus for unassigned devices
     * CAN bus diagnostics
     * Reboot of SmartCoach
     * Automatic email diagnostic and device reports
     * Wipe Wifi Credentials
 
These capabilities add substantial features making it more of a SmartCoach solution and not just a simple bridge.
For email relay setup, see [the guide](cloud/gmail-relay/README.md).
In addition, I've upgraded the library stack to be current with HomeSpan and uses the internal TWAI on the ESP32. Finally, I added regression test suite for the builds.

---
## Sample EVE Home Screens
These screenshots show RV devices in the EVE Home app, including awning, motion, and door-lock controls. The available screens depend on the devices installed in the coach.

<img width="200" alt="Door awning and window covering controls" src="images/IMG_6506.PNG"/>    <img width="200" alt="Chassis motion sensor status" src="images/IMG_6507.PNG"/>    <img width="200" alt="Front door lock controls" src="images/IMG_6508.PNG"/>  |  <img width="200" alt="EVE Home accessory screen" src="images/IMG_6524.PNG"/>
<img width="200" alt="EVE Home accessory screen" src="images/IMG_6509.PNG"/>    <img width="200" alt="EVE Home accessory screen" src="images/IMG_6510.PNG"/>    <img width="200" alt="EVE Home accessory screen" src="images/IMG_6511.PNG"/>  |  <img width="200" alt="EVE Home accessory screen" src="images/IMG_6512.PNG"/>

---
<img width="200" alt="EVE Home accessory screen" src="images/IMG_6513.PNG"/>    <img width="200" alt="EVE Home accessory screen" src="images/IMG_6514.PNG"/>
<img width="200" alt="EVE Home accessory screen" src="images/IMG_6521.PNG"/>

---
<img width="200" alt="EVE Home accessory screen" src="images/IMG_6522.PNG"/>

---
<img width="200" alt="EVE Home accessory screen" src="images/IMG_6523.PNG"/>  <img width="200" alt="EVE Home accessory screen" src="images/IMG_6525.PNG"/>

<img width="200" alt="EVE Home accessory screen" src="images/IMG_6526.PNG"/>  <img width="200" alt="EVE Home accessory screen" src="images/IMG_6527.PNG"/>

<img width="200" alt="EVE Home accessory screen" src="images/IMG_6528.PNG"/>

## Web Portal
The first screen shows the coach profile and discovered devices.
The upper-right menu provides live Status, Diagnostics, Email Reports, Email setup, and the Privacy Notice. System actions are separated in the menu; Wi-Fi reset and reboot require confirmation. Status refreshes from the bridge's routine status poll without running a discovery scan.

<img width="200" alt="Web Portal Top of Page" src="images/IMG_6503.PNG"/>

---
This screenshot, below, shows the coach details when you click on the "View Coach Details" button at the top.

<img width="200" alt="Coach Details" src="images/IMG_6504.PNG"/>

---
This Water Pump has been discovered but is still unassigned. Its card shows the two related RV-C messages.

<img width="200" alt="Unassigned Water Pump card showing its RV-C messages" src="images/IMG_6505.PNG"/>

---
This screenshot shows an earlier footer layout. The current portal places these tools and system actions in the upper-right menu.

<img width="200" alt="Diagnostic Page Bottom" src="images/IMG_6520.PNG"/>

### Diagnostic Portal
The diagnostics page shows bridge health, configured devices, and unmapped CAN traffic.

<img width="200" alt="Diagnostics Page Top" src="images/IMG_6516.PNG"/>   <img width="200" alt="Diagnostics Page Showing Configured Device Metrics" src="images/IMG_6517.PNG"/>

---
<img width="850" alt="Diagnostics table showing unmapped CAN messages" src="images/IMG_6518.PNG"/>
<img width="850" alt="Diagnostics table showing decoded DM-RV details" src="images/IMG_6519.PNG"/>


### Email Portal
The email page lets owners choose report options and configure email delivery.

## Captive Portal
The captive portal collects coach Wi-Fi, coach details, and report preferences during setup.
<img width="850" alt="SmartCoach captive portal setup page" src="images/CaptivePortal.png"/>

---
## MVC
The architecture relies heavily on the Model View Controller pattern. The Model is the RVC side of the house, the View is the Apple HomeKit and the Controller, if it is not a sensor, is typically a class defined within the View and private to the view. The controller interacts with the model per any changes made within the View (HomeKit). For example, in the ThermostatView, there are 2 controllers, one for the fan and one for the thermostat settings. Why did I make them private within the View Class? I did this to “encapsulate and hide” specific attributes from other non-related classes. This means that the Model, View, and Controller are cohesive and only share what they need to share but not to other un-related classes / objects. 

---
## Libraries
lib_deps =
     elapsedMillis
     homespan/HomeSpan@2.1.8       
     bblanchon/ArduinoJson@^7.2.0
     
---
## Archive
Archive has all of the original code from Randy Ubillos. I created this so that I had ease of access to the original code.

---
## Main
The main.cpp is located directly under the src directory tree. It manages the initialization and start of the code.
<img width="850" alt="main associations" src="images/RV-Bridge.png" />

---
## Base
The Base folder contains code used across the project. `CoachESP32` manages the board. `CoachWifi` connects to the coach Wi-Fi and starts the setup page when needed. Wi-Fi details are saved on the device, so changing them does not require rebuilding the firmware.

---
## Class Hierarchy
This is a general class hierarchy, meaning Controllable and Sensors are specific devices within the directory structure. Controllable devices include switches, sliders, thermostats, etc. Sensors include power, tanks, and the like. 
DeviceView is a generic term for each View into a specific device. It is the translator of messages to/from HomeKit via the library HomeSpan. In general, when updates occur from HomeKit for controllables, it will translate the message and use the controller to update the model via the ESP32CAN library. Some of the earlier MVC models, like WaterPump, I wrote use update without the controller, that’s a flaw I intended to come back and fix. Sensors do not have a controller since they don’t control anything, they just read values.

NOTE: Since HomeKit does NOT have a unit for volume,  voltage, current or percentage, I use temperature in Fahrenheit to represent those values.

<img width="850" height="490" alt="Class Hierarchy" src="https://github.com/user-attachments/assets/5788e130-3d93-4321-b525-be7026ab2faa" />


---
## Packets
There are several files in the Packet directory, including:
DGN.h - this contains all the DGN’s defined in the RVC specification
PacketKit.h, Packet.h and Packet.cpp - manages packets sent to the queue on the ESP32. This is also where you will find several print statements commented out where I was discovering what DGNs were communicating on the RVC bus.
PacketQueue.h and PacketQueue.cpp - the packet queue code 
Packets are referenced or used by models and controllers.

---
## Devices
At startup, `DeviceFactory` reads `devices.json` and `coach.json` to create the devices for this coach. You can change many device names and settings in the web portal without rebuilding the firmware.

Saving coach details restarts SmartCoach after the configuration is successfully saved. Use **Apply Device Changes** beside device search to restart and load saved device changes; the button asks for confirmation and does not save unfinished edits.

For an Inverter or Charger whose AC-point instance differs from its device instance, set `"extra": {"acPointInstance": 3}` on that device. Valid AC-point instances are 1 through 13. Without this setting, the bridge uses `rvcIndex` for AC telemetry as before; ordinary status messages still use `rvcIndex`. This maps one AC-point instance per configured device, not multiple independent measurement points.

ATS input/output readings follow the source selected by `ATS_STATUS`. Until a valid source is reported, or when no source is active, the displayed readings remain zero; packets from inactive sources are cached separately.

### RV-C Support Boundaries
For temporary awning interlock diagnosis only, the `AwningDiagnostics` build environment enables `SMARTCOACH_AWNING_PARK_BYPASS`. It bypasses the bridge's park-brake check for awning writes and movement, not shades. Use only under direct supervision with the coach physically immobilized, keep the area clear, and restore `Release` immediately after testing. It must not be shipped to clients. Building does not deploy the bypass; uploading this firmware does. No filesystem upload is needed for this diagnostic test.

The portal's Diagnostics page and downloadable report show named charger operating states, inverter modes, generator states, and ATS source/mode details. Voltage, current, temperature, percentages, and runtime remain numeric. Apple Home's existing tiles are unchanged; custom text is not substituted into numeric HomeKit characteristics.

Generator, inverter, charger, and ATS AC displays consume AC Status Page 1. Pages 2 through 4 (peak readings, power/phase data, and additional qualification/fault data) are not consumed. `CHARGER_STATUS_3` derating telemetry and internal charger DC-bus telemetry are also not consumed.

Battery accessories monitor `DC_SOURCE_STATUS` battery-bank instances, not individual `BATTERY_STATUS` devices. Unsupported bank fields are not displayed. Chassis motion uses speed from `CHASSIS_MOBILITY_STATUS`; the park-brake field remains the command safety gate. `CHASSIS_MOBILITY_STATUS_2` is not displayed. Shade position remains time-estimated because its status message has no absolute position.

Run `perl test/host/run-learning-tests.pl --rvc` for focused production-method payload checks, or omit `--rvc` for the full host suites. These checks and a successful firmware build do not certify live-coach interoperability or full RV-C compliance.

### Generic Device
GenericDevice is model base class and has much of the functionality that all devices use as well as the management of the views. 

### SpanView
SpanView is the base class for all views. Views are HomeKit displays of the specific device. If the device is controllable, then a controller will be embedded privately in the view. Note all devices are ‘friends’ to their views since the view would not exist without the device. Hence, they are cohesive.

### Controllable
All controllable devices (switches and HVAC devices) are located in this directory. 

#### Switches
Switches, including adjustable switches, are located in a sub directory, switches, under controllable. This includes:
* WaterPump
* DoorLock
* LightDevice
* CoverDevices for Awnings and Shades.

#### HVAC
The HVAC devices included are:
* Thermostat: manages and controls the temperature, heat/cool/off setting and the fan settings
* FloorHeat: turns on the floor heat to a desired temperature setting

### Sensors
Sensors are devices that are NOT controlled. I chose not to control the Generator and some of the other devices simply for safety sake. Generator could easily be controlled if you choose to do so.
Non-power sensors include:
* ChassisMobility: detects whether the parking brake is on and if the transmission is in drive/reverse. This is defined as a motion sensor.
* Tanks: tanks are reported in temperature as a percentage. As stated earlier, HomeKit does not have a percent or volume metric to report on. Tanks included are:
   * Fresh Water
   * Gray Tank
   * Black Tank

#### Power
Power is a set of sensors to monitor batteries, inverters, a generator, and automatic transfer switches.
Power devices have an inheritance that is set up to simplify the code; all power devices, except Battery, are inherited from PowerSensor. The following Power devices are available:
* AutomaticTransferSwitch
* Battery
* Generator
* Inverter
* Charger
---
## Other Considerations
In my coach, I’ve added the capability of including Ring devices into the HomeKit. Ring devices are not native to HomeKit, but can be added via a tool similar to my RV-Bridge, Homebridge.org . I use a homebridge on a Orange PI board. I use a Ring plugin to map Ring devices to the Apple Home so, for example, when I lock my door, the Ring alarm is armed and unarmed when the door is unlocked. 

In addition, I use the EVE Home app for any automation, EVE Home is more sophisticated and includes the ability to perform conditional statements where HomeKit does not. In addition, EVE Home is a vendor for motion sensors, security devices, etc.

Finally, I don’t use any RV based networking system like Winegard, I use Unifi’s Ubiquiti. If Ubiquiti was used, you could set up a firewall and link it to Starlink, use an 8 port POE switch, connect the Homebridge solution directly into the switch, and provide a wired connection to an AP on the ceiling and one in the basement for sensors. This would secure the Coach. Also, if clients wanted it, they could opt in to the Unifi Ubiquiti 5G Max which allows for dual sims and Starlink with a dual WAN on the Unifi Ubiquiti UCG-Max or UCG-Ultra. The UCG can be configured to be primary/secondary or load balanced.

---
## Randy's Original Readme

![RV-Bridge](/images/box_wire_scale.jpeg)

---

1. [Features](#features)
2. [Background](#background)
3. [Current Project State](#state)
4. [To-Do](#todo)
5. [Hardware](#hardware)
6. [Wiring](#wiring)
7. [Firmware Setup](#firmware)
8. [Finding Output Numbers](#outputs)
9. [Supported RVs](#rvs)
10. [3D Printing](#3dprint)
11. [Notes and Tips](#notes)
12. [Links](#links)

---
## <a name="features"></a>Features

* Connects to the RV-C network in many modern RVs.
    * RV-C is a subset of CAN-Bus running at 250kbps.
* Uses an ESP32 with a CAN-Bus interface.
* Connects lights, fans, switches, and thermostats to HomeKit.
* Fits inside the RV wiring panel.
* Plugs into an unused CAN-Bus socket for power and data.
* STL files for a 3D printed case are included.

---
## <a name="background"></a>Background

At the start of the pandemic we realized that international travel was going to be off the table for a significant duration. We decided it was time [to see more of the Western US](http://rickandrandy.com/map/index.html?RV) so we bought a class-A motorhome which we call ***The Penguin Express***. We've put [almost 30k miles on it so far](http://rickandrandy.com/?rvlife).

![Penguin Express](/images/Penguin_Express.jpg)

In addition to a bunch of 3D printed upgrades (we travel with a Prusa MK3S on board), I've done some arduino powered electronics work: a water valve for a reverse osmosis water filter with an LCD control panel, a GPS based clock for the bedroom that knows the exact timezone boundaries so it never needs to be set, and a GPS based altimeter and tire pressure monitor with a 7" color touchscreen for the dash.

The RV's lights, fans, and climate are all controlled through a [Firefly Integrations](https://fireflyint.com) [Vegatouch Spectrum](https://www.vegatouch.com) multiplex system. A 7" LCD panel is used to control everything, in addition to wireless keypads around the RV. There is also a bluetooth module that connects to an iOS app for controlling via an iPhone.

|   |   |
| --- | --- |
| ![Firefly Main](/images/Firefly_main.jpeg) | ![Firefly Lights](/images/Firefly_lights.jpeg) |
| ![Firefly Climate](/images/Firefly_climate.jpeg) | ![Firefly Fans](/images/Firefly_fans.jpeg) |

It's a great system and works really well for control of the RV devices, but the iOS app is a bit slow to load/connect and can only be used in proximity to the RV. Since we already have an Apple TV onboard driving our TVs which could double as a hub, I've always wondered if there was a way to control it all via HomeKit and the Home app.

Recently I came across some documentation for the bus protocol that's used by the Spectrum system, RV-C, a subset of CAN-Bus, as well as the open source project [CoachProxyOS](https://github.com/linuxkidd/coachproxy-os) that documents getting a Raspberry Pi set up to host a web page for controlling an RV's network using RV-C.

I also recently started playing with [HomeSpan](https://github.com/HomeSpan/HomeSpan), a library for implementing HomeKit accessories on an ESP32 microcontroller, for a HomeKit doorbell project.

RV-Bridge is the result of putting these pieces together.

---
## <a name="state"></a>Current Project State

## v1.0.9 - In progress
* Added option to create two "Thermostats" which show the voltage of the chassis and house batteries as temperature * 10
* Added support for exterior awnings.

## v1.0.0
* Homespan pairing works, devices show up in the Home app.
* CAN-Bus packet receiving works.
* RV-C messages are routed correctly to the HomeKit tiles.
* The correct RV-C packets are being sent over the bus based on changes made in the Home app for lights, switches, and fans.
* The RV devices respond correctly.
* Lights, switches, and fans are complete.
* Temperature readings from thermostats are reflected in the Home app.
* Thermostat control is complete.

---
## <a name="todo"></a>To-Do

* **Tech debt (awning/chassis refinement, deferred until owner returns from visiting family):** Investigate the intermittent park-brake-unavailable event that rejected awning writes in Eve. Preserve the normal safety interlock and client pairing/accessory identities. Capture unavailable-state packets with source addresses and timestamps, check decoding/routing/startup/freshness behavior, and verify accepted/rejected writes and scene behavior across brake states. See the analysis below; a permanent decoder fix has not yet been established.
* **Tech debt (verify on next development deployment):** HomeKit service definitions were corrected for Charger State, AutoFill Last Operation, and Generator Engine Status. Release build and host tests pass; still verify HomeSpan reports zero database warnings on-device and check existing Home/Eve displays. Characteristic order/count was kept stable; preserve client AIDs, IIDs, and pairings before any client release.
* **Tech debt (stability, deferred until owner returns from visiting family):** Routine portal polling now reuses a cached discovery snapshot; `/discovery` is fetched on initial load and explicit Scan, while `/status` keeps learning state live. Browser checks confirmed no repeated discovery fetch during polling and manual Scan retries after an HTTP 503. The bridge previously recorded minimum heap as low as 512 bytes and `NetworkClient` errno 11; this mitigation does not prove the cause or live stability. Profile peak allocations and verify heap headroom under repeated scans, diagnostics requests, and HomeKit activity.
* **Tech debt (client-release requirement, deferred until owner returns from visiting family):** Provide a portal-update process that automatically preserves saved coach settings, device configuration, accessory IDs (`aid` and `chassisAid`), and HomeKit pairings. Filesystem/portal updates must not replace client settings with bundled defaults. Manual configuration backups are a development workaround, not an acceptable client-update requirement. Verify preservation through updates and update failures before client release.
* **Tech debt (soon, deferred until owner is ready):** Replace numeric power-state tiles in Apple Home (for example, Charger State `6`) with understandable named status indicators such as "Float Charging." Review other power-state codes too. Apple Home cannot display arbitrary status strings. HomeKit service changes and re-pairing are acceptable during development only. Client releases must preserve existing pairings and accessory identities without requiring re-pairing. Readable status text already exists in portal diagnostics.
* Some RVs have AC units that can work as a heat pump; only the AC portion of these units is currently handled. (Ours only has a furnace so I don't currently have a way to implement this.)

### Awning And Chassis Analysis
* Eve reported read/write failure and reverted the awning switch to Off while diagnostics reported **Park brake unavailable**. The guard rejects released, unknown, reserved, or unreceived brake status; physical parking alone does not establish what the bridge received.
* The temporary `AwningDiagnostics` bypass allowed extension, isolating the rejection to the park-brake guard. The bypass is now disabled on the live bridge: guarded `Release` was restored. Do not ship or retain the bypass as a client fix.
* Passive instrumentation captured `CHASSIS_MOBILITY_STATUS` (`0x1FFF4`) from source address 148: `00 00 00 00 FD FF 7D 7D`. Byte 4 is `FD`, and bits 0-1 are `01` (park brake engaged). Fresh observations continued to report engaged. The earlier unavailable payload was not captured, so its cause remains unproven.
* A separate host-reproduced HomeKit conflict was fixed: an Out/In callback no longer calls `setVal()` on an already-pending `TargetPosition` write. Standalone On still requests fully out; Off requests fully in. An explicit pending position takes precedence in a combined write. This defect is not proven to have caused the original live brake-status failure.
* After guarded Release deployment, the owner verified front-awning extension through Eve and the **Retract Front Awning** scene. Both directions worked with the safety guard enabled; other awnings and longer-term stability still need verification.
* On return, capture the missing failure evidence and add bounded transition history/rejection reasons if needed. Test engaged/released/unavailable/stale/startup status, conflicting sources, standalone switches, explicit positions, combined writes, and scenes. Never infer a safe park state solely from zero speed or unknown brake bits.

---
## <a name="hardware"></a>Hardware

Uses an ESP32 with a CAN-Bus interface, either as separate components, or more easily, this board I found from [skpang.co.uk](https://www.skpang.co.uk):

The photo shows the ESP32 board used for the bridge.

![ESP32 Module](/images/board_in_box.jpeg)

In the U.S. it's available on the CopperHillTech Website:<br>
[ESP32 with WiFi, Bluetooth Classic, BLE, CAN Bus Module](https://copperhilltech.com/esp32-wifi-bluetooth-classic-ble-can-bus-module/)

This board has everything needed, including a regulator for powering the device off of the 12V provided by the RV-C connector.

---
## <a name="wiring"></a>Wiring

The connector used by the Firefly system is a ***3M 37104-A165-00E MB*** which can be sourced from [Digikey](https://www.digikey.com/en/products/detail/3m/37104-A165-00E%2520MB/1855697)

Insert four 24AWG wires into the Can-Bus connector (I used silicone covered wire as they are much more flexible) and compress to make the connections. Twist the data and power pairs together and screw them into the terminal block on the CAN-Bus interface on the ESP32.

The CAN-Bus connector plugs into one of the available sockets inside the system wiring panel.

The photos below show the cable, connector wiring, the coach wiring panel, and the Home app after setup.

|  |  |
| :---: | :---: |
| <br>![Cable Wiring](/images/cable.jpeg) |![Can-Bus Connector Wiring](/images/CAN-connector-wiring.jpg) |
| ![G7 Panel](/images/G7_panel.jpeg) | ![Home App](/images/Home_App.PNG) |

---
## <a name="firmware"></a>Firmware Setup

- The project is set up for compilation with PlatformIO.
    * I use it via Microsoft's Visual Studio Code.
- Pretty sure you could also use the Arduino IDE. You'd need to install the following libraries:
    * elapsedMillis
    * miwagner/ESP32CAN
    * homespan/HomeSpan
- `config.h`
    * Duplicate `config-sample.h` to `config.h`. (Do not check this file into git, it will have your wifi passowrd in it)
    * Enter Wifi SSID and password for your RV network.
    * Include a definition file for your RV devices (see `Miramar_2020_3202.h` for an example).
        * Each switch has:
            * Output number.
            * Type: Lamp, DimmableLamp, or Switch.
            * Name.
        * Each fan has:
            * Output number for fan power.
            * Output number for up - optional, -1 if not used.
            * Output number for down - optional, -1 if not used.
            * Name.
        * Each thermostat has:
            * ID number, typically 0 based.
            * Output number for the A/C compressor.
            * Output number for low fan.
            * Output number for high fan.
            * Output number for furnace - optional, -1 if not present.
            * Name.
        * Each awning has:
            * Output number for extend.
            * Output number for retract.
            * Time (in ms) that the awning takes to get to the "unroll" portion.
            * Time (in ms) that the awning takes to "unroll" at the end. Set to 0 if not applicable.
            * Time (in ms) that the awning takes to get from the "roll" portion to fully retracted.
            * Time (in ms) that the awning takes to "roll" at the beginning. Set to 0 if not applicable.
            * Name.
        * See [Finding Output Numbers](#outputs) below for details on output numbers.
    * Uncomment `#define CREATE_BATTERIES` to create "Thermostats" for the house and chassis batteries.
        * The "thermostat" will show the current battery voltage times 10 as temperature.
            * Homekit does not currently have a way to display voltages, so this is my hack to be able to see this very useful information.
        * You will likely want to turn off "Include in Home Summaries" in the Status setting for these "thermostats".
        * When in screen "rearrange" mode you can tap on a thermostat and choose to show it in a smaller sized tile.
- Flashing
    * If you are using an ESP32 with a USB-C connector and flashing from a Mac, you may need to connect it via a USB hub due to some timing weirdness around resetting the ESP32 into boot mode. I use a USB-C to 4 port USB-A hub with a USB-A to USB-C cable.
- Startup
    * Connect to the ESP32 via the Serial Monitor.
    * You should see a bunch of startup logging.
    * Then a message about being connected to Wifi and not being paired.
    * HomeSpan provides a command line interface that you can access through the Serial Monitor. Type '?' to see the available comands.
- Pairing
    * Be on the same wifi network and in close proximity to the ESP32.
    * In the Home app choose "Add Accessory".
    * Point the camera at this image:
    <br><br>
    ![Pairing Code](/images/defaultSetupCode.png)
    <br><br>
    * Tap on `RV-Bridge`.
    * Accept that this is an "unsupported" device.
    * Add the bridge and all of your accessories, choosing appropriate rooms and names for them.
    * Done!
- The status LED will flash based on what the bridge is doing:
    * 🟢 Green every 2 seconds as a heartbeat indicator.
    * 🔴 Red when CAN-Bus packets are sent.
    * 🔵 Blue when HomeKit messages are received.

---
## <a name="outputs"></a>Finding Output Numbers

The whole multiplex system connects back to a panel with outputs for all of the lights and fans. Each of these outputs has a unique number which may be printed on the panel's cover, and should also be found on a Network Diagnostic screen on the main LCD control screen.

![G7 Outputs](/images/G7_Outputs.jpeg)

*** ***USE EXTREME CAUTION WHEN ENTERING OUTPUT NUMBERS. THERE ARE OUTPUTS FOR THE RV SLIDES AND THINGS LIKE MOVEABLE BUNKS. YOU DO NOT WANT TO MISTAKENLY PICK ONE OF THOSE OUTPUTS FOR A LIGHT OR FAN!*** ***

---
## <a name="rvs"></a>Supported RV's

Currently the project includes definition files for these RVs in the `RV` folder:

[Miramar_2020_3202.h](/src/RV/Miramar_2020_3202.h) - 2020 Thor Miramar 32.2<br>
[Aria_2019_3901.h](/src/RV/Aria_2019_3901.h) - 2019 Thor Aria 39.1<br>
[Tiffin_2019_34PA.h](/src/RV/Tiffin_2019_34PA.h) - 2019 Tiffin Open Road 34PA<br>
[Jayco_2023_Terrain.h](/src/RV/Jayco_2023_Terrain.h) - 2023 Jayco Terrain 19Y

(Additional definition files are welcome!)

---
## <a name="3dprint"></a>3D Printing

- A case will keep the microcontroller isolated from any exposed contacts inside the wiring panel.
- STL Files are in the `3D` folder:
    * [RV-Bridge_Box_Bottom.stl](/3D/RV-Bridge_Box_Bottom.stl)
    * [RV-Bridge_Box_Top.stl](/3D/RV-Bridge_Box_Top.stl)
- Slicer
    * I used Prusa Slicer 2.5.0
- Build Plate
    * A textured build plate give a nice surface finish for the top and bottom of the box.
- Filament
    * PETG - handles heat better than PLA and sticks to a textured build plate much better than PLA.
- Settings to adjust:
    * `Layer Height`: 0.3mm (faster printing)
    * `Extrusion Width`: 0.55mm (eliminates tiny infill strips in the walls).
    * `Perimeter Transitioning Threshold Angle`: 20 (keeps the lettering connected).
    * `Bridging Angle`: 180º (bridging in the layer above the lettering should be parallel to the baseline of the text.)

---
## <a name="notes"></a>Notes and Tips

- If the bridge seems to become unresponsive at some point, verify that the controlling device is on the RV's Wifi and not some other weak Wifi.
- If the bridge doesn't seem available for pairing, it may already think it's paired. Try using the H command via the cli in the serial monitor, then reflash the ESP32 and try again.
- If pairing fails, it seems that sometimes HomeKit gets fussy about a device changing it's properties too much and refuses to pair. You can change the MAC address of the wifi interface on the ESP32 by defining `OVERRIDE_MAC_ADDRESS` in `config.h` and re-flashing. Anecdotal evidence suggests that this can help.

---
## <a name="links"></a>Links:

- [Apple's HomeKit Accessory Protocol Specification Release R2 (HAP-R2)](https://developer.apple.com/homekit/specification/)
    * For some reason this link appears to be broken on the Apple side at the moment... with _just a tiny bit_ of hunting on the internet you can find it 😉
- [RV-C Organization](http://www.rv-c.com)
- [RV-C Spec 2022-12-01](http://www.rv-c.com/sites/rv-c.com/files/RV-C%20Protocol%20FullLayer-12-01-22.pdf)
- [SK Pang Electronics](https://www.skpang.co.uk)
- [Schematic for the ESP32 CAN-Bus Board above](https://cdn.shopify.com/s/files/1/0563/2029/5107/files/ESP32_CAN_rev_B.pdf?v=1620032162)

---

Copyright © 2023-2024 [Randy Ubillos](http://rickandrandy.com)
