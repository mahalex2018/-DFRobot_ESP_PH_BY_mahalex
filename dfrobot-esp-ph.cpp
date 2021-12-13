/*
 * file dfrobot-esp-ph.cpp * @ https://github.com/GreenPonik/DFRobotESPpH_BY_GREENPONIK
 *
 * Arduino library for Gravity: Analog pH Sensor / Meter Kit V2, SKU: SEN0161-V2
 * 
 * Based on the @ https://github.com/DFRobot/DFRobot_PH
 * Copyright   [DFRobot](http://www.dfrobot.com), 2018
 * Copyright   GNU Lesser General Public License
 *
 * ##################################################
 * ##################################################
 * ########## Fork on github by GreenPonik ##########
 * ############# ONLY ESP COMPATIBLE ################
 * ##################################################
 * ##################################################
 * 
 * version  V1.0
 * date  2019-05
 */
#include "dfrobot-esp-ph.h"


/**
 * @brief Initializes the pH sensor/hardware and assigns it the proper pins
 * 
 * @param PH_PIN_in Input for pH sensor on ESP32
 * @param ESPADC_in Input for ADC from ESP32
 * @param ESPVOLTAGE_in Input for ESP32 Voltage source
 */
void DFRobotESPpH::init(int PH_PIN_in, float ESPADC_in, int ESPVOLTAGE_in) {
    PH_PIN = PH_PIN_in;
    ESPADC = ESPADC_in;
    ESPVOLTAGE = ESPVOLTAGE_in;
}

/**
 * @brief Retrieves the pH value of the tank/solution
 * 
 * @param temp_in Temperature of tank(in Celsius?)
 * @return float
 */
float DFRobotESPpH::getPH(float temp_in) {
    float voltage = analogRead(PH_PIN) / ESPADC * ESPVOLTAGE; // read the voltage
    Serial.println(voltage);
    this->_voltage = voltage;
    this->_temperature = temp_in;
    return readPH(voltage, temp_in); // convert voltage to pH with temperature compensation
}

/**
 * @brief Constructor that assigns default(neutral) values to pH sensor metrics
 * 
 */
DFRobotESPpH::DFRobotESPpH()
{
    this->_temperature = 25.0;
    this->_phValue = 7.0;
    this->_acidVoltage = 1844.17;   //buffer solution 4.0 at 25C
    this->_neutralVoltage = 1348.68; //buffer solution 7.0 at 25C
    this->_voltage = 1348.68;
}

/**
 * @brief Destructor
 * 
 */
DFRobotESPpH::~DFRobotESPpH()
{
}

/**
 * @brief Gets the neutral voltage of tank/solution
 * 
 * @return float 
 */
float DFRobotESPpH::get_neutralVoltage(){
	return this->_neutralVoltage;
}

/**
 * @brief This is the startup function for the pH sensor. It gets everything ready so that the sensor can actually start to calibrate/read values
 * 
 */
void DFRobotESPpH::begin()
{
	preferences.begin("pHVals", false);
    //check if calibration values (neutral and acid) are stored in eeprom
    //preferences.putFloat("voltage7", this->_neutralVoltage);
    this->_neutralVoltage = preferences.getFloat("voltage7", 0); //load the neutral (pH = 7.0)voltage of the pH board from the EEPROM
    if (this->_neutralVoltage == 0)
    {
        this->_neutralVoltage = 1348.68; // new EEPROM, write typical voltage
        preferences.putFloat("voltage7", this->_neutralVoltage);
    }

    //preferences.putFloat("voltage4", this->_acidVoltage);
    this->_acidVoltage = preferences.getFloat("voltage4", 0); //load the acid (pH = 4.0) voltage of the pH board from the EEPROM
    if (this->_acidVoltage == 0)
    {
        this->_acidVoltage = 1844.17; // new EEPROM, write typical voltage
        preferences.putFloat("voltage4", this->_acidVoltage);
    }
	preferences.end();
}

/**
 * @brief Reads pH level of a given solution/water in fish tank
 * 
 * @param voltage // TODO
 * @param temperature temperature of the water
 * @return float 
 */
float DFRobotESPpH::readPH(float voltage, float temperature) {

    float slope = (7.0 - 4.0) / ((this->_neutralVoltage - 1500.0) / 3.0 - (this->_acidVoltage - 1500.0) / 3.0);
    float intercept = 7.0 - slope * (this->_neutralVoltage - 1500.0) / 3.0;
    this->_phValue = slope * (voltage - 1500.0) / 3.0 + intercept;
    return _phValue;
}

/**
 * @brief calibrates the pH sensor given a remote command
 * 
 * @param cmd calibration command
 */
void DFRobotESPpH::calibration(char *cmd) {
    strupr(cmd);

    // if received Serial CMD from the serial monitor, enter into the calibration mode
    phCalibration(cmdParse(cmd));
}


/**
 * @brief tries to search a remote command and calibrates the pH sensor if found
 * 
 */
void DFRobotESPpH::calibration() {
    if (cmdSerialDataAvailable() > 0)
    {
      if(strstr(this->_cmdReceivedBuffer, "MANCALPH") != NULL){
        int v7 = 0, v4 = 0;
        Serial.println("Manual Calibration: Please enter the voltage value for pH 4");
        while(cmdSerialDataAvailable() <= 0){}
        if(strstr(this->_cmdReceivedBuffer, "EXIT") != NULL){
          return;
        }
        else{
          v7 = atoi(_cmdReceivedBuffer);
        }
        Serial.println("Manual Calibration: Please enter the voltage value for pH 7");
        while(cmdSerialDataAvailable() <= 0){}
        if(strstr(this->_cmdReceivedBuffer, "EXIT") != NULL){
          return;
        }
        else{
          v4 = atoi(_cmdReceivedBuffer);
        }
        manualCalibration(v7, v4);
      }
      else{
          phCalibration(cmdParse()); // if received Serial CMD from the serial monitor, enter into the calibration mode
      }
    }
}
/**
 * @brief manual calibration function
 * 
 */
void DFRobotESPpH::manualCalibration() {
  
  if(cmdSerialDataAvailable() > 0){
    
  }
}

/**
 * @brief checks to see whether serial data is/is not available
 * 
 * @return boolean True if data is available, False otherwise
 */
boolean DFRobotESPpH::cmdSerialDataAvailable()
{
    char cmdReceivedChar;
    static unsigned long cmdReceivedTimeOut = millis();
    while (Serial.available() > 0)
    {
        if (millis() - cmdReceivedTimeOut > 500U)
        {
            this->_cmdReceivedBufferIndex = 0;
            memset(this->_cmdReceivedBuffer, 0, (ReceivedBufferLength));
        }
        cmdReceivedTimeOut = millis();
        cmdReceivedChar = Serial.read();
        if (cmdReceivedChar == '\n' || this->_cmdReceivedBufferIndex == ReceivedBufferLength - 1)
        {
            this->_cmdReceivedBufferIndex = 0;
            strupr(this->_cmdReceivedBuffer);
            return true;
        }
        else
        {
            this->_cmdReceivedBuffer[this->_cmdReceivedBufferIndex] = cmdReceivedChar;
            this->_cmdReceivedBufferIndex++;
        }
    }
    return false;
}

/**
 * @brief parses a remote command
 * 
 * @param cmd input command
 * @return byte index mode
 */
byte DFRobotESPpH::cmdParse(const char *cmd) {
    byte modeIndex = 0;
    if (strstr(cmd, "ENTERPH") != NULL)
    {
        modeIndex = 1;
    }
    else if (strstr(cmd, "EXITPH") != NULL)
    {
        modeIndex = 3;
    }
    else if (strstr(cmd, "CALPH") != NULL)
    {
        modeIndex = 2;
    }
    return modeIndex;
}

/**
 * @brief recieves a command and parses it
 * 
 * @return byte index mode
 */
byte DFRobotESPpH::cmdParse() {
    byte modeIndex = 0;
    if (strstr(this->_cmdReceivedBuffer, "ENTERPH") != NULL)
    {
        modeIndex = 1;
    }
    else if (strstr(this->_cmdReceivedBuffer, "EXITPH") != NULL)
    {
        modeIndex = 3;
    }
    else if (strstr(this->_cmdReceivedBuffer, "CALPH") != NULL)
    {
        modeIndex = 2;
    }
    return modeIndex;
}

/**
 * @brief Calibrates pH sensor based on provided mode
 * 
 * @param mode the mode from cmdparse
 */
void DFRobotESPpH::phCalibration(byte mode) {
    char *receivedBufferPtr;
    static boolean phCalibrationFinish = 0;
    static boolean enterCalibrationFlag = 0;
    switch (mode)
    {
    case 0:
        if (enterCalibrationFlag)
        {
            Serial.println(F(">>>Command Error<<<"));
        }
        break;

    case 1:
        enterCalibrationFlag = 1;
        phCalibrationFinish = 0;
        Serial.println();
        Serial.println(F(">>>Enter PH Calibration Mode<<<"));
        Serial.println(F(">>>Please put the probe into the 4.0 or 7.0 standard buffer solution<<<"));
        Serial.println();
        break;

    case 2:
        if (enterCalibrationFlag)
        {
            if ((this->_voltage > PH_8_VOLTAGE) && (this->_voltage < PH_6_VOLTAGE))
            { // buffer solution:7.0
                Serial.println();
                Serial.print(F(">>>Buffer Solution:7.0"));
                this->_neutralVoltage = this->_voltage;
                Serial.println(F(",Send EXITPH to Save and Exit<<<"));
                Serial.println();
                phCalibrationFinish = 1;
            }
            else if ((this->_voltage > PH_5_VOLTAGE) && (this->_voltage < PH_3_VOLTAGE))
            { //buffer solution:4.0
                Serial.println();
                Serial.print(F(">>>Buffer Solution:4.0"));
                this->_acidVoltage = this->_voltage;
                Serial.println(F(",Send EXITPH to Save and Exit<<<"));
                Serial.println();
                phCalibrationFinish = 1;
            }
            else
            {
                Serial.println();
                Serial.print(F(">>>Buffer Solution Error Try Again<<<"));
                Serial.println(); // not buffer solution or faulty operation
                phCalibrationFinish = 0;
            }
        }
        break;

    case 3://store calibration value in eeprom
        if (enterCalibrationFlag)
        {
            Serial.println();
			preferences.begin("pHVals", false);
            if (phCalibrationFinish)
            {
                if ((this->_voltage > PH_8_VOLTAGE) && (this->_voltage < PH_5_VOLTAGE))
                {
                    preferences.putFloat("voltage7", this->_neutralVoltage);
					Serial.print(F("PH 7 Calibration value SAVE THIS FOR LATER: "));
					Serial.print(this->_neutralVoltage);
                }
                else if ((this->_voltage > PH_5_VOLTAGE) && (this->_voltage < PH_3_VOLTAGE))
                {
                    preferences.putFloat("voltage4", this->_acidVoltage);
					Serial.print(F("PH 4 Calibration value SAVE THIS FOR LATER: "));
					Serial.print(this->_acidVoltage);
                }
                Serial.print(F(">>>Calibration Successful"));
            }
            else
            {
                Serial.print(F(">>>Calibration Failed"));
            }
			preferences.end();
            Serial.println(F(",Exit PH Calibration Mode<<<"));
            Serial.println();
            phCalibrationFinish = 0;
            enterCalibrationFlag = 0;
        }
        break;
    }
}

/**
 * @brief Manually calibrate the pH sensor 
 * 
 * @param voltage7 voltage at pH 7
 * @param voltage4 voltage at pH 4
 */
void DFRobotESPpH::manualCalibration(float voltage7, float voltage4){
	preferences.begin("pHVals", false);
	
	preferences.putFloat("voltage7", this->_neutralVoltage);
	Serial.println(F("PH 7 Calibration value saved"));
	preferences.putFloat("voltage4", this->_acidVoltage);
	Serial.println(F("PH 4 Calibration value saved"));
	
	preferences.end();
}
