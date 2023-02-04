// Imports!
#include <Arduino.h>
#include <VescUart.h>

//*******************************
// Motor Driver Configuration
//*******************************
int motorSpd = 0;
float maxPower = 0.5;
float motorPower = 0;
int MAX_SPEED = 6000; // [RPM]
int interval = 1000;
bool enableDriver;
bool vescStatus;	// TODO: disable/enable driver
u_int32_t lastDriverTime = 0;
u_int32_t lastRcvTime = 0;
u_int32_t lastSendTime = 0;

/*
* By default when using the Arduino Cores:
*   Serial is a virtual Serial port operating over the USB
*   Serial1 is tied to UART0 on pins 1 and 2
*   Serial2 may need creating (not for philhower core), and is UART1 on pins 8&9
*/

VescUart VESC1; // Front Right
// HardwareSerial Serial2;
#define driverUART &Serial1	// motor driver UART port - pico pins 	GP8 & GP9, teensy pins 0 (RX1) & 1 (TX1)


void printVESCStatus();
void printVESCStatus_readable();

//*******************************
// Command Handling
//*******************************
#define cmdSerial Serial	// Serial port to receive commands from - USB Port (virtual)
// #define cmdSerial Serial1	// Serial port to receive commands from - pico pins GP0&GP1

void parseCommand(String command);
void setMotors(float magnitude, float direction);

void s()
{
	cmdSerial.print("status;");
}

void setup()
{
	// Serial1.setTX(16);
	// Serial1.setRX(17);

	// Serial Communications
	cmdSerial.begin(115200);

	// Setup VESC Motors

	Serial1.begin(115200);

	VESC1.setSerialPort(driverUART);	// TODO
	// VESC1.setDebugPort(&cmdSerial);

	if (VESC1.getVescValues())
	{
		s();
		cmdSerial.println("VESC 1 Connected!");
		
	}
	else
	{
		s();
		cmdSerial.println("No VESC1");
	}
	// Give VESC time to boot up
	delay(1000);

	// Tell VESC to send us its data
	VESC1.getVescValues();
}

void loop()
{
	if (millis() - lastDriverTime > 1000)
	{
		// Tell VESC to send us its data
		vescStatus = VESC1.getVescValues();
		lastDriverTime = millis();
		// if (!vescStatus) { cmdSerial.println("Unsuccessful Driver status!"); } // #FIXME
	}

	if (cmdSerial.available())
	{
		cmdSerial.print("Received cmd: ");
		String command = cmdSerial.readStringUntil('\n');
		cmdSerial.println(command);
		parseCommand(command);
		
	}

	if (millis() - lastSendTime > interval)
	{
		lastRcvTime = millis();
		motorSpd;
		lastSendTime = millis();
		cmdSerial.println("Status: Speed:"+motorSpd); //#FIXME
	}
	if (millis() - lastSendTime > 5000)
	{
		VESC1.printVescValues();
	}

	if (millis() - lastRcvTime > 200)
	{
		motorSpd = 0;
	}

	VESC1.setRPM(motorSpd);
}

// Command Format: [commmand_type],power_str
// Commands: max_power, esc_status
void parseCommand(String command)
{
	// String exec = command.substring(0, command.indexOf(';'));
		lastRcvTime = millis();
	// cmdSerial.println(command);	//#DEBUG
	// cmdSerial.println(exec);		//#DEBUG

	lastRcvTime = millis();

	int firstComma = command.indexOf(',');
	String motor = command.substring(0, firstComma);
	String power_str = command.substring(firstComma + 1);
	float power = strtof(power_str.c_str(), NULL);	// Power, Max power, RPM value

	if (motor.equals("RPM"))
	{
		motorSpd = (int)constrain(power, -MAX_SPEED, MAX_SPEED);
		cmdSerial.print("Set RPM to ");
		cmdSerial.println(motorSpd);
	} 
	else if (motor.equals("esc_status"))
	{
		cmdSerial.println("Detailed status:");
		printVESCStatus_readable();
	} 
	else
	{
		if (motor.equals("set_power"))
		{
			motorPower = power;
			cmdSerial.print("Set power to ");
			cmdSerial.println(motorPower);
		}
		else if (motor.equals("max_power"))
		{
			maxPower = power;
			cmdSerial.print("Set max power to ");
			cmdSerial.println(power);
		}

		motorSpd = (int)constrain(motorPower * MAX_SPEED * maxPower, -MAX_SPEED, MAX_SPEED);
		cmdSerial.print("New Motor speed: ");
		cmdSerial.println(motorSpd);
	}
}

void printVESCStatus_readable()
{
	s();
	cmdSerial.print("ID: ");
	cmdSerial.println(VESC1.data.id);
	s();
	cmdSerial.print("RPM: ");
	cmdSerial.println(VESC1.data.rpm);
	s();
	cmdSerial.print("Inp Voltage: ");
	cmdSerial.println(VESC1.data.inpVoltage);
	s();
	cmdSerial.print("AmpHours: ");
	cmdSerial.println(VESC1.data.ampHours);
	s();
	cmdSerial.print("Tachometer: ");
	cmdSerial.println(VESC1.data.tachometerAbs);
}

void printVESCStatus()
{
	s();
	cmdSerial.println(VESC1.data.id);
	s();
	cmdSerial.println(VESC1.data.rpm);
	s();
	cmdSerial.println(VESC1.data.inpVoltage);
	s();
	cmdSerial.println(VESC1.data.ampHours);
	s();
	cmdSerial.println(VESC1.data.tachometerAbs);
}