# User manual

### Hardware Setup

#### Parts List
 The hardware required, and specific purchase links used are as follows:
* ESP8266: [NodeMCU V3 Development Board - Aliexpress](https://www.aliexpress.com/item/1005005977505151.html?spm=a2g0o.productlist.main.1.655chiCOhiCO1B&algo_pvid=433da9db-ed7b-415e-bc84-49cf7948839a&algo_exp_id=433da9db-ed7b-415e-bc84-49cf7948839a-0&pdp_npi=4%40dis%21GBP%214.94%211.44%21%21%2143.92%2112.74%21%40210387dd17111011633648628e4bcc%2112000035141219305%21sea%21UK%212110106883%21&curPageLogUid=qJJMcWGrdllf&utparam-url=scene%3Asearch%7Cquery_from%3A)
* INA226: [INA226 - Aliexpress](https://www.aliexpress.com/item/1005005181953704.html?spm=a2g0o.order_list.order_list_main.24.5d991802mwiEgH)
* INA3221: [INA3221 - Aliexpress](https://www.aliexpress.com/item/4000402618181.html?spm=a2g0o.order_list.order_list_main.25.5d991802mwiEgH)
* Shunt Resistor (if required): [200A Shunt Resistor - Aliexpress](https://www.aliexpress.com/item/4000102337577.html?spm=a2g0o.order_list.order_list_main.29.5d991802mwiEgH)
* Boost-Buck Voltage Converter: [Voltage Converter - Aliexpress](https://www.aliexpress.com/item/1005001543920467.html?spm=a2g0o.order_list.order_list_main.91.5d991802mwiEgH)
* SD Card adapter: [Micro SD Adapter - eBay](https://www.ebay.co.uk/itm/315076760549?itmmeta=01HSJSP52M2HR8TRWHBQ1T1ZY4&hash=item495c09d3e5:g:yQsAAOSwLgNf6xce&itmprp=enc%3AAQAJAAAAwFIXQLmJqMgz%2BVrKvFrqV33pFPrQOKBsBOYLL8Q0oky6pOhZ3r4Mbho63QA8hBbpK4QCfmjLtPNbvXtPbFQ0dYKZdBx28qidg1xHg7ddVpxnZi5FKbdb6Oav54Hm9uKP7iF9LzA3K88FnNTm91x%2FIfne7rWTiS6Pkn5OYbFvlck%2BxRWMSd%2FyYiNyZ3bx6YgKgkQX3INwiH1PLJaSdTmXXVvp0FuacbGnYsvMHcEMEIkb1HEUKGfhaVzMM5CntGD4Nw%3D%3D%7Ctkp%3ABk9SR9DR2NnMYw)
* (Optional) OLED Display: [SSD1306 Display - Aliexpress](https://www.aliexpress.com/item/1005001636414945.html?spm=a2g0o.order_detail.order_detail_item.8.45d3f19cuC47gj)

#### Connections
The project is wired as shown below.
![Wiring Diagram](/src/wiring-diagram.jpg "Wiring Diagram")

The boost-buck converter, set to 5V output, is connected to the main voltage input of the ESP8266 development board.

The INA226, INA3221, and OLED display (if used), are connected to the I2C data pins (typically GPIO4 and GPIO5, labelled D1 and D2 on the NodeMCU) of the ESP8266, and the 3.3V power pins.

The SD card adaptor is connected to the SPI pins of the ESP8266 (GPIO12, 13, 14, 15, labelled D5, 6, 7, 8), and either the 3.3V or 5V power pins, depending on if the SD card adaptor used has a built in 5V to 3.3V converter.

If using an external shunt resistor, this must be connected to the shunt positive and negative pins on the INA226.

The main power connection from the drone is then wired through the current shunt in series, and in parallel to the buck/boost converter.