# LevitaTUor



## About

This is the LevitaTUor App to control the Acoustifly using an Android Device via USB-Connection written for 
Students@School at the TU Darmstadt by Thomas Schreck

## Origin of files

This app builds upon an Andoriod Studio Drawer template and the
- [SimpleUSBTerminal by kai-morich](https://github.com/kai-morich/SimpleUsbTerminal/tree/master) 


## Known Issues

Due to how the SerialListener is handled two mayor issues are known to me:

- at the start screen the button label sometimes doesn't change from "connected" to "disconnected" when it should
- the correct return values of "Schnelleingabe" Buttons is only printed if the button is pressed twice

## Conversion

If there are some namings that I overlooked they are converted as follows:

- home -> home
- gallery -> terminal
- slideshow -> quick_access

The app is also not tested with the refactoring done, as I neither own a Smartphone with a USB-C Conntector, nor a USB-C to micro-USB Cable.
