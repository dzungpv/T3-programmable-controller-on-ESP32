
# TEMCO ESP main App CODE

This application establishes a TCP connection between Ethernet and WIFI, and an RS485 connection. The device can be accessed through the standard modbus protocol, so that the value of the sensor on the device can be obtained. A more intuitive way is to scan and read the device through the T3000 software of TEMCO CONTROLS.


## How to compile

Please download ESP idf integrated development environment of Espressif Company to compile the code, the version is 4.0.1
1.	Install the ESP-IDF tool installer
Enter the 
https://dl.espressif.com/dl/esp-idf/?idf=4.4 
page, you can download the online version of the esp - idf - tools - setup, also can download the offline version of the esp - idf - tools - setup, the online version of the relatively small, But the component will be downloaded during installation
2.	 Run esp-idf-tools-setup-offline-x.x.exe
You can change the default ESP IDF installation path, which will be used later.
3.	Once installed, you should get an ESP-IDF Eclipse, which is the compiler we used to compile ESP32 code.
4.	Copy the three folders, driver. temco_bacnet and temco_IO_control, from the code download on Github to the components directory in esp-idf and replace the original three folders. This is the change we made to the IDF.
5.	Open the ESP-IDF Eclipse program, File ->Import, Espressif ->Existing IDF Project, select the project directory downloaded from github, and click Finish to load the project into the compiler. Right click the project name and select Build Project to compile.
6.	You can get more help from 
https://esp32.com/index.php
https://espressif-docs.readthedocs-hosted.com/projects/esp-idf/en/latest/index.html



## How to load the firmware to T3-nano
Please refer to the documentation in the following path.
./Bootloader update guide for T3.pdf