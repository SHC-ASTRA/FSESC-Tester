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
bool vescStatus;
u_int32_t lastDriverTime = 0;
u_int32_t lastRcvTime = 0;
u_int32_t lastSendTime = 0;

VescUart VESC1; // Front Right
#define driverUART &Serial2	// motor driver UART port - pico pins 	GP8 & GP9

void calculateTankControlSpeeds(float magnitude, float direction, float speed);
void calculateMotorSpeeds(float magnitude, float direction, float speed);
void printVESCStatus();

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
	// Serial Communications
	cmdSerial.begin(115200);

	// Setup VESC Motors

	Serial2.begin(115200);

	VESC1.setSerialPort(driverUART);	// TODO

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
		cmdSerial.println("Status: Speed:"+motorSpd);
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
	} else 
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

	if (motor.equals("esc_status"))
	{
		cmdSerial.println("Detailed status:");
		printVESCStatus();
	}
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