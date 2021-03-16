# Soccer Wall BLE Integration

This project covers Bluetooth Low Energy (BLE) integration into the wall. Features include sending / receiving data with a phone app, error / missing packet detection and queues that hold 60 objects each.

Code will first check to see if phone has sent data and if so will decode + send ack back to phone. It will then check to see if there is any new HitData or LogData to send along with checking the queues to see if any old data needs to be sent. The queue logic works so that it needs an ack back for each packet (HitData or LogData) before sending a new one. This ensures that the phone receives 100% of the transmitted data.

Preconditions:

* For HitDataBLE & LogDataBLE, all variables must be populated except **seq** before adding it to the queue & setting flag.
* For DataInBLE, after removing it from the queue you will need to go through it and take appropriate actions based on values.

## Deployment

Using a STM32F030R8 MCU with a Raytac BLE Board - [MDBT42Q-AT-UART-S](https://www.raytac.com/product/ins.php?index_id=83) using 3.3V from STM, 57600 BAUD & UART PD Pin tied to GND.

UART1 - (BLE Board) Using pins TX: PA9 -- RX: PA10 (TX/RX labels on STM side)

* **BLE.c** - Holds encode / decode & ack handling functions.
* **HitDataQueueBLE.c** - Use **addHitDataQueue()** (takes in HitDataBLE struct) to insert into queue for transmission. Seq is updated when encode is called.
* **LogDataQueueBLE.c** - Use **addLogDataQueue()** (takes in LogDataBLE struct) to insert into queue for transmission. Seq is updated when encode is called.
* **DataInQueueBLE.c** - Use **removeDataInQueue()** (returns DataInBLE struct) to remove received data from phone.
* **DataStructsBLE.h** - Holds the structs for HitDataBLE, LogDataBLE and DataInBLE.

Code in **main.c** and production code under while() can be integrated into existing program. Uses UART1 to interface with BLE board.

Will transmit when **transmitHitDataFlag** (or transmitLogDataFlag) is set (meaning new data is added to queue) and when new data is received, **dataInFlag** is set. Uses acks to make sure data is received correctly on phone side and will retransmit un-ack'd messages.

See **testTransmissionCases()** (main.c) on how to populate struct & add to queue + setting flag.

For DataInBLE (receiving):
```
if(DataInFlag == true) {
	if(!isEmptyDataInQueue()) {
		DataInBLE newDataIn = removeDataInQueue();
		dataInFlag = false;
	}
}
```

To remove test code used for the phone / wall test set, simply remove the sections in the while() loop that are sectioned off with the test code comments.

## Notes

STM Real Time Clock (RTC) is set up in the code (minus commented out lines) but was returning zero values from **getTimeSeconds()**. When the RTC is working the timeStamp variable can be populated with a time stamp in seconds.

To modify HitDataBLE or LogDataBLE:

* **NOTE** - When modifying leave dataType & seq as first two variables in struct.
* In **DataStructsBLE.h** add or remove desired variables.
* In **BLE.c** under either encodeHitData() or encodeLogData(), update the temp array to include the new variables.
* In **main.c** under main(), update phoneSendBuffHit[] or phoneSendBuffLog[] to the new correct amount of bytes the struct takes up.
* Update phone app code accordingly.

To modify DataInBLE:

* **NOTE** - When modifying leave dataType & seq as first two variables in struct.
* In **DataStructsBLE.h** add or remove desired variables.
* In **BLE.c** under decode(), update the temp array to include the new variables.
* In **main.c** under main(), update phoneReceiveBuff[] to the new correct amount of bytes the struct takes up.
* Update phone app code accordingly.

To update queue size:

* In **DataInQueueBLE.h**, **HitDataQueueBLE.h**, **LogDataQueueBLE.h**, & **main.c** change "#define capacity 60" to desired size.

## Built With

* [Keil MDK](http://www2.keil.com/mdk5) - ARM Software Development
* [STM32CubeMx](https://www.st.com/en/development-tools/stm32cubemx.html) - Code Generation / Pin Layout


## Authors

* **Terry Cox** - coxte@pm.me


## License
