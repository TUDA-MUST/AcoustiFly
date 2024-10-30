<img src="doc/images/tud_logo.gif" align="right" width="140"/>

# AcoustiFly, a single transducer Acoustic Levitator
[![Badge](https://img.shields.io/badge/Built%20w%2F-KiCad-blue)](https://www.kicad.org/)
[![Badge](https://img.shields.io/badge/Built%20w%2F-PlatformIO-blue)](https://platformio.org/)

<img align="right" src="doc/images/logo_acoustifly_for_GitHub.svg"  width="120">

Unlock the Power of Sound: Explore Acoustic Levitation with Ease and Precision.
This project focuses on the development of an acoustic levitator, a fascinating technology capable of suspending small objects in mid-air using ultrasonic waves. This repository provides all the necessary resources, including code, schematics, and documentation, to build and experiment with your own acoustic levitation system.

![Move](doc/images/ezgif-7-e92c7d3068.gif)

***

## Table of Contents

- [Introduction](#introduction)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)
- [Frequently Asked Questions \(FAQ\)](#frequently-asked-questions-faq)

## Introduction

Our repository is dedicated to showcasing a method of acoustic levitation using a single transducer and a reflector. Acoustic levitation, also known as acoustic trapping, is a fascinating technique that allows objects to float in mid-air using sound waves.

In our approach, we utilize a single transducer, which emits high-frequency sound waves, and a carefully designed reflector. The transducer generates an acoustic field that interacts with the reflector to create standing waves within a confined region. These standing waves produce regions of high and low pressure, forming what is known as an "acoustic pressure node".

When an object is placed at a specific position within the acoustic pressure node, the forces exerted by the surrounding sound waves counteract the gravitational force, resulting in levitation. By precisely adjusting the distance of the transducer and the reflector, we can control the levitation height and stability of objects.

This unique approach offers several advantages. Firstly, it simplifies the setup by using only a single transducer and a reflector, reducing complexity and cost. Secondly, it enables precise control over the levitation process, allowing for manipulation and positioning of objects with great accuracy. Lastly, it opens up possibilities for various applications, including material handling, microfluidics, and scientific experiments.

In our repository, we provide detailed documentation, source code, and demonstrations to help you understand and implement this acoustic levitation technique. Whether you are a researcher, teacher, hobbyist, or enthusiast, we invite you to explore our repository, contribute to its development, and unlock the potential of acoustic levitation with a single transducer and a reflector.

**Join us on this exciting journey of harnessing the power of sound waves to defy gravity. Visit [our homepage](https://www.etit.tu-darmstadt.de/must/home_must/index.en.jsp) to delve into our research and learn more about our team.**

## Features

[Highlight the key features of your acoustic levitator. Enumerate the capabilities and performance metrics that make it stand out. You can use bullet points or a table to present this information.]

* **Low Cost:** Affordable and accessible to everyone -> TODO: add cost estimate
* **Easy to Build:** Simple setup with readily available components
* **Open Source:** Free to use, modify, and distribute
* **Standalone:** Battery powered operation possible
* **Adjustable:** Distance between transducer and reflector can be adjusted, thus stability can be tuned

## Installation

### PlatformIO
Here's a [Quickstart Guide](https://docs.platformio.org/en/latest/integration/ide/vscode.html#quick-start) on how to use Platform IO with Visual Studio Code.
### Arduino IDE
To enable working with the underlying microcontroller (ESP32-S3-MINI-1-N4R2) you are required to install the relevant board package. To do this, open _Tools > Board > Boards Manager_ and install the _'esp32'_ package by Espressif Systems.\
The Board should be set to `Adafruit Feather ESP32-S3 2MB PSRAM` as this comes closest to the used microcontroller. The relevant options under Tools should be set according to the following image. Make sure to enable `USB CDC on Boot`, which permits USB communication.

![Arduino IDE options](doc/images/Arduino_options_Highlight.png)

### Uploading Code
Since this board does not use a separate usb-to-serial chip, the microcontroller needs to be put into the bootloading state manually. To accomplish this, simply hold down down the <kbd> BOOT </kbd> button on the PCB and then quickly press the <kbd> Reset </kbd> button once. After releasing the <kbd> BOOT </kbd> button, you are now able to upload your code.\
_! The COM Port is likely to change after entering Boot-mode and may need to be set again in your Software !_

## Usage

### Pinout
| Pin | Name | Description |
|-----|------|-------------|
|1|LED_YELLOWGREEN|emits yellow-green light (current sink!)|
|2|LED_RED|emits red light (current sink!)|
|4|HBRIDGE_ENABLE      |HIGH enables the h_bridge/output for both transducers|
|5|HBRIDGE_A_NORMAL|PWM Input for Transducers A|
|6|HBRIDGE_A_INVERTED|inverted PWM Input for Transducers A|
|7|HBRIDGE_B_NORMAL|PWM Input for Transducers B|
|8|HBRIDGE_B_INVERTED|inverted PWM Input for Transducers B|
|9|HBRIDGE_CURRENTSENSE|read analog voltage to measure transducer current consumption|
|10|BAT_STATE      |used to read battery voltage|
|40|MISO      |SCPI|
|41|SCK      |SCPI|
|41|MOSI      |SCPI|

The **LED**s are setup in a current sink configuration, meaning a LOW signal will emit light while a HIGH signal will shut them off. Consider the command `GPIO.func_out_sel_cfg[PIN].inv_sel = 1` to reverse this. \
The **HBRIDGE_*** inputs require either the typical PWM signal or the inverted one. The easiest way to achieve this is to set both to the same [LEDC Channel](https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/peripherals/ledc.html) and invert one output with the already mentioned command. \
**CURRENTSENSE** can be used to measure the current consumption of both transducers. **TODO EXPLAIN SHUNT/AMPLIFIER SETUP**. \
**BAT_STATE** can be used to read the battery voltage. However, it is reduced by a voltage divider. `Multiply` the measured voltage `by 11` to read the correct voltage level. \


[Explain how to use your acoustic levitator effectively. Provide step-by-step instructions, code examples, and demonstrations. Include any command-line or configuration options that users can utilize.]

[If applicable, include visual aids such as diagrams, screenshots, or videos to enhance the understanding of your acoustic levitator in action.]

## Contributing

We welcome contributions to this project! If you would like to contribute, please review the guidelines in the [CONTRIBUTING](CONTRIBUTING.md) file.

## License

This project is licensed under the GNU GENERAL PUBLIC LICENSE Version 3, 29 June 2007. Please see the [LICENSE](LICENSE) file for more details.

## Related Projects

[If you have any related projects or repositories that users might find interesting or useful, list them here along with brief descriptions or links.]

Remember to update the sections accordingly with the relevant information for your acoustic levitator. A well-crafted README will enhance the accessibility and usability of your project, enabling users to understand, install, and utilize your acoustic levitator effectively.
