/* Idea
0x0. => 
0x1. => get
0x2. => set 
0x3. => flash

cmd flags:
0b01000000 => reserved, Maybe to set diffrent packet structure?
0b10000000 => error occur
*/

/*
Examples:
command:
<cmd> <len> <some> <crc>

good response:
<cmd> <len> <some> <crc>

packet error response:
<0x80 | cmd> <len=4> <packet_error_code> <crc>

command error response:
<0x80 | cmd> <len> <packet_error_code> <command_error_code> <crc>
*/


//ESP packet error responses:
#define USB_ERROR_BAD_CRC 0x01
#define USB_ERROR_BAD_LENGTH 0x02
#define USB_ERROR_NO_COMMAND 0x03
#define USB_ERROR_COMMAND_ERROR 0x04

//ESP commad error responses:
#define USB_COMMAND_ERROR_BAD_LENGTH 0xc4

#define USB_COMMAND_ERROR_NULL_FILE 0x02
#define USB_COMMAND_ERROR_FILE_TOO_BIG 0x03

// Initialize ESP to send ladder diagram
// PC send:
// <cmd> <len=7> <crc>
// ESP response:
// <cmd> <len=7> <ldLength 4B> <crc>
#define USB_COMMAND_INIT_READ_LD 0x30

// ESP send data from flash
// PC send:
// <cmd> <len> <crc> 
// ESP response:
// <cmd> <len> <data (max 61 bytes)> <crc> 
#define USB_COMMAND_READ_LD 0x31

// End ESP sending data from flash
// <cmd> <len> <crc> 
// ESP response:
// <cmd> <len> <crc> 
#define USB_COMMAND_END_READ_LD 0x32


// Initialize ESP to receive ladder diagram, PC sends
// PC send:
// <cmd> <len=7> <ldLength 4B> <crc>
// ESP response:
// <cmd> <len=7> <crc>
#define USB_COMMAND_INIT_WRITE_LD 0x33

// ESP receive chunk of data write to flash, PC sends
// PC send:
// <cmd> <len> <data (max 61 bytes)> <crc> 
// ESP response:
// <cmd> <len> <chunkLength 4B> <crc> 
#define USB_COMMAND_WRITE_LD 0x34

// End ESP receiving data, PC sends
// PC send:
// <cmd> <len> <crc> 
// ESP response:
// <cmd> <len> <ldLength 4B> <crc> 
#define USB_COMMAND_END_WRITE_LD 0x35


// Get information about devices: 
// deviceIndex = 0 => controller, 
// deviceIndex > 0 => extension modules
// PC send:
// <cmd> <len=4> <deviceIndex 1B> <crc>
// ESP response:
// <cmd> <len=8> <deviceType 1B> <firmwareVersion 1B> <numberOfAnalogInputs 1B> <deviceInitTime 4B> <crc>
#define USB_COMMAND_GET_DEVICE_INFO 0x10 

// Get count of extension modules
// PC send:
// <cmd> <len> <crc>
// ESP response:
// <cmd> <len> <deviceCount 1B> <crc>
#define USB_COMMAND_GET_EXTENSION_MODULES_COUNT 0x11

//#define USB_COMMAND_GET_DEVICE_STATUS 0x21 

// ESP sends digital outputs of device:
// deviceIndex = 0 => controller
// deviceIndex > 0 => extension modules
// PC send:
// <cmd> <len> <deviceIndex 1B> <crc> 
// ESP response:
// <cmd> <len> <digitalOutputs 2B> <crc> 
#define USB_COMMAND_GET_DIGITAL_OUTPUTS 0x17

// ESP sends digital inputs of device:
// deviceIndex = 0 => controller
// deviceIndex > 0 => extension modules
// PC send:
// <cmd> <len> <deviceIndex 1B> <crc> 
// ESP response:
// <cmd> <len> <digitalInputs 2B> <crc> 
#define USB_COMMAND_GET_DIGITAL_INPUTS 0x18

// ESP sends analog inputs of device:
// deviceIndex = 0 => controller
// deviceIndex > 0 => extension modules
// PC send:
// <cmd> <len> <deviceIndex 1B> <crc> 
// ESP response:
// <cmd> <len> <analogInputs 8B> <analogUpdateTime 16B> <crc> 
// #define USB_COMMAND_GET_ANALOG_INPUTS 0x29 

//#define USB_COMMAND_GET_DIGITAL_INPUTS_ALL 0x2a
//#define USB_COMMAND_GET_DIGITAL_OUTPUTS_ALL 0x2b

// PC send data and ESP response with the same data
// PC send:
// <cmd> <len> <data> <crc>
// ESP response:
// <cmd> <len> <data> <crc> 
#define USB_COMMAND_ECHO 0x01 


// Get ESP temperature. Received value has to be calculated to receive temeprature in *C: (0.4386 * deviceTemp - 27.88)
// PC send:
// <cmd> <len> <crc> 
// ESP response:
// <cmd> <len> <deviceTemp 1B> <crc> 
#define USB_COMMAND_GET_DEVICE_TEMPERATURE 0x14

// Receive ESP time from start clock
// PC send:
// <cmd> <len> <crc> 
// ESP response:
// <cmd> <len> <time 8> <crc> 
#define USB_COMMAND_GET_DEVICE_TIME 0x12

// Set ESP neutral mode
// PC send:
// <cmd> <len> <crc> 
// ESP response:
// <cmd> <len> <crc> 
#define USB_COMMAND_SET_NEUTRAL_MODE 0x20 

// Unset ESP neutral mode
// PC send:
// <cmd> <len> <crc> 
// ESP response:
// <cmd> <len> <crc> 
#define USB_COMMAND_UNSET_NEUTRAL_MODE 0x21 

// Get errors occurred on the ESP
// PC send:
// <cmd> <len> <crc> 
// ESP response:
// <cmd> <len> <crc> 
#define USB_COMMAND_GET_ERRORS 0x1f

#define USB_NO_COMMAND 0x7a 
