# Assembly image (what it will look like inside)

<img width="865" height="492" alt="thumbnail" src="https://github.com/user-attachments/assets/9dd15b03-97c7-4893-9618-0281f208641a" />

## Schematic (connections of the components)

<img width="740" height="499" alt="image" src="https://github.com/user-attachments/assets/1e73c4c2-36f6-4510-a0b4-5c1ce27f6c9b" />

## PCB tracings 

<img width="502" height="508" alt="image" src="https://github.com/user-attachments/assets/4bba5906-61ff-4c52-9a06-9c3501b772a9" />
<img width="508" height="499" alt="image" src="https://github.com/user-attachments/assets/05ac2aec-6649-4cdf-8ca3-3934eb9f305c" />

# What is this rediculous looking thing?? 
It is a really cheap and simple Weather station! it shows you data of your surrounding area such as temp, humidity, AQI  
i made it as a challenge to make a weather station that shows data not just on a LCD but on your phone from YOUR surrounding, not some  
sensor miles away from your surrounding.  
# Firmware 
since this uses ESP32, simply flash the firmware onto the board 
# ussage
connect it to the Wifi and you should be able to be see the data as a webpage on it's IP.  




# BOM 
| Reference | Qty | Value | DNP | Exclude from BOM | Exclude from Board | Footprint | Datasheet |
|---|---:|---|---|---|---|---|---|
| C1,C2 | 2 | 4.7 µF | | | | Capacitor_SMD:C_0805_2012Metric | |
| C3,C4,C5,C6 | 4 | 10 µF | | | | Capacitor_SMD:C_0805_2012Metric | |
| D1 | 1 | STATUS LED | | | | LED_THT:LED_D5.0mm | |
| J1 | 1 | USB_C_Receptacle_USB2.0_16P | | | | Connector_USB:USB_C_Receptacle_GCT_USB4105-xx-A_16P_TopMnt_Horizontal | [USB Type-C Specification](https://www.usb.org/sites/default/files/documents/usb_type-c.zip) |
| J2 | 1 | TPS61023 | | | | Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical | |
| J3 | 1 | Battery | | | | Connector_PinHeader_2.54mm:PinHeader_1x02_P2.54mm_Vertical | |
| J4 | 1 | BME280 | | | | Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical | |
| J5 | 1 | PMS5003 | | | | Connector_JST:JST_GH_BM08B-GHS-TBT_1x08-1MP_P1.25mm_Vertical | |
| R1 | 1 | 330 Ω | | | | Resistor_THT:R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal | |
| R2 | 1 | 2 kΩ | | | | Resistor_SMD:R_0805_2012Metric | |
| R3,R4 | 2 | 5.1 kΩ | | | | Resistor_SMD:R_0805_2012Metric | |
| SW1,SW2,SW3 | 3 | SW_Push | | | | Button_Switch_THT:SW_PUSH_6mm | |
| U1 | 1 | ESP32-WROOM-32 | | | | ESP32 symbol and footprint:MODULE_ESP32-DEVKITC | [ESP32-WROOM-32 Datasheet](https://www.espressif.com/sites/default/files/documentation/esp32-wroom-32_datasheet_en.pdf) |
| U2 | 1 | MCP73831-2-OT | | | | Package_TO_SOT_SMD:SOT-23-5 | [MCP73831 Datasheet](http://ww1.microchip.com/downloads/en/DeviceDoc/20001984g.pdf) |
| U3 | 1 | AP2112K-3.3 | | | | Package_TO_SOT_SMD:SOT-23-5 | [AP2112K Datasheet](https://www.diodes.com/assets/Datasheets/AP2112.pdf) |
