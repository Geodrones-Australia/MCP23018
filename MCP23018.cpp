/******************************************************************************
MCP23018.cpp
Interface for MCP23018 16 bit IO expander
Bobby Schulz @ Northern Widget LLC
07/18/2019
https://github.com/NorthernWidget-Skunkworks/MCP23018

Allows control of all aspects of the control of the IO expander 

0.0.0

"All existing things are really one"
-Zhuangzi

Distributed as-is; no warranty is given.
******************************************************************************/

#include <Arduino.h>
#include <MCP23018.h>


const uint8_t I2C_MCP23018 = B0100000;

MCP23018::MCP23018(int _ADR, TwoWire &wire)
{
  ADR = _ADR; // 0x20 - 0x27
  _wire = &wire; 
}


int MCP23018::begin(void)  //FIX! Combine interrupt lines be default!
{

  PinModeConf[0] = 0xFF; //Default to all inputs //FIX make cleaner
  PinModeConf[1] = 0xFF; 

  if (!scan()) {
    return 0;
  } else 
  return 1;
}

bool MCP23018::scan() {
  // Scan I2C bus
  HAL_StatusTypeDef error;

  // Check if device is active on the bus
  error = HAL_I2C_IsDeviceReady(_wire->getHandle(),(uint16_t)ADR << 1, 1, 1);

  if (error == HAL_OK)
  {
    return true;
  }
  else
  {
    return false;
  } 
}

int MCP23018::PinMode(int Pin, uint8_t PinType, bool Port)
{
  if(Pin > 8 || Pin < 0)
  {
    return -1;  //Fail if pin out of range
  }

  if(PinType == INPUT)
  {
    PinModeConf[Port] = PinModeConf[Port] | (0x01 << Pin); //Set bit for input
    PullUpConf[Port] = PullUpConf[Port] & ~(0x01 << Pin);  //Clear bit for pullup 
    // Serial.print("Reg 0x03 = ");  //DEBUG!
    // Serial.println(PinModeConf, HEX); //DEBUG!
    SetDirection(PinModeConf[Port], Port);
    SetPullup(PullUpConf[Port], Port);
    return 1;
  }
  else if(PinType == OUTPUT)
  {
    PinModeConf[Port] = PinModeConf[Port] & ~(0x01 << Pin); //Clear bit for output
    PullUpConf[Port] = PullUpConf[Port] | (0x01 << Pin);  //Set pullup bit to allow for "push-pull" operation
    // Serial.print("Reg 0x03 = ");  //DEBUG!
    // Serial.println(PinModeConf[Port], HEX); //DEBUG!
    // Serial.println(PullUpConf[Port], HEX); //DEBUG!
    SetDirection(PinModeConf[Port], Port);
    SetPullup(PullUpConf[Port], Port);
    return 0;
  }

  else if(PinType == OPEN_DRAIN)
  {
    PinModeConf[Port] = PinModeConf[Port] & ~(0x01 << Pin); //Clear bit for output
    PullUpConf[Port] = PullUpConf[Port] & ~(0x01 << Pin);  //Clear pullup bit to allow for open drain operation
    // Serial.print("Reg 0x03 = ");  //DEBUG!
    // Serial.println(PinModeConf[Port], HEX); //DEBUG!
    SetDirection(PinModeConf[Port], Port);
    SetPullup(PullUpConf[Port], Port);
    return 0;
  }

  else if(PinType == INPUT_PULLUP)
  {
    PinModeConf[Port] = PinModeConf[Port] | (0x01 << Pin); //Set bit for input
    PullUpConf[Port] = PullUpConf[Port] | (0x01 << Pin);  //Set bit for pullup  
    // Serial.print("Reg 0x03 = ");  //DEBUG!
    // Serial.println(PinModeConf, HEX); //DEBUG!
    SetDirection(PinModeConf[Port], Port); 
    SetPullup(PullUpConf[Port], Port);
    return 0;
  }
  else 
    return -1; //Fail if pin type not defined 
}

int MCP23018::DigitalWrite(int Pin, bool State, bool Port)
{
  if(Pin > 8 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(State == HIGH)
  {
    PortState[Port] = PortState[Port] | (0x01 << Pin);
    // Serial.print("Reg 0x01 = ");  //DEBUG!
    // Serial.println(Port, HEX); //DEBUG!
    SetPort(PortState[Port], Port);
    return 1;
  }
  else if(State == LOW)
  {
    PortState[Port] = PortState[Port] & ~(0x01 << Pin);
    // Serial.print("Reg 0x01 = ");  //DEBUG!
    // Serial.println(Port, HEX); //DEBUG!
    SetPort(PortState[Port], Port);
    return 0;
  }
  else 
    return -1; //Fail if state is ill-defined
}

int MCP23018::SetInterrupt(int Pin, bool State, bool Port)
{
  if(Pin > 8 || Pin < 0)
  {
    return -1; //Fail if pin out of range
  }

  if(State == ON)
  {
    InterruptConf[Port] = InterruptConf[Port] | (0x01 << Pin);
    // Serial.print("Reg 0x01 = ");  //DEBUG!
    // Serial.println(Port, HEX); //DEBUG!
    SetPort(InterruptConf[Port], Port);
    return 1;
  }
  else if(State == OFF)
  {
    InterruptConf[Port] = InterruptConf[Port] & ~(0x01 << Pin);
    // Serial.print("Reg 0x01 = ");  //DEBUG!
    // Serial.println(Port, HEX); //DEBUG!
    SetPort(InterruptConf[Port], Port);
    return 0;
  }
  else 
    return -1; //Fail if state is ill-defined
}

int MCP23018::SetPort(int Config, bool Port) 
{
  _wire->beginTransmission(ADR); // transmit to device with address ADR
  _wire->write(LATA + Port);   //Send to output set register
  _wire->write(Config);   
  // Serial.println(LATA + Port, HEX); //DEBUG!
  // Serial.println(Config, HEX); //DEBUG!
  // Serial.print("\n\n"); //DEBUG!
  return _wire->endTransmission();
}

int MCP23018::SetDirection(int Config, bool Port) 
{
  // Serial.println(Config, HEX); //DEBUG!
  _wire->beginTransmission(ADR); // transmit to device with address ADR
  _wire->write(DIRA + Port);        //Send to port configuration register
  _wire->write(Config);              
  // Serial.println(DIRA + Port, HEX); //DEBUG!
  // Serial.print("\n\n"); //DEBUG!
  return _wire->endTransmission();    // stop transmitting
}

int MCP23018::SetPolarity(int Config, bool Port) 
{
  _wire->beginTransmission(ADR); // transmit to device with address ADR
  _wire->write(POLA + Port);        //Send to port configuration register
  _wire->write(Config);              
  return _wire->endTransmission();    // stop transmitting
}

int MCP23018::SetPullup(int Config, bool Port) 
{
  // Serial.println(Config, HEX); //DEBUG!
  _wire->beginTransmission(ADR); // transmit to device with address ADR
  _wire->write(PULLUPA + Port);        //Send to port configuration register
  _wire->write(Config);         
  return _wire->endTransmission();    // stop transmitting
}

int MCP23018::SetInt(int Config, bool Port) 
{
  _wire->beginTransmission(ADR); // transmit to device with address ADR
  _wire->write(PULLUPA + Port);        //Send to port configuration register
  _wire->write(Config);         
  return _wire->endTransmission();    // stop transmitting
}

//IN DEVELOPMENT
int MCP23018::ReadPort(int Config, bool Port)
{
  _wire->beginTransmission(ADR); // transmit to device with address ADR
  _wire->write(0x01);        //Send to port configuration register
  _wire->write(Config);              //Set port 1-4 as OUTPUT, all others as inputs
  return _wire->endTransmission();    // stop transmitting
}





