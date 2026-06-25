#include "RVConstants.h"
#ifdef HOME_KIT_2

#ifndef FLOOR_HEAT_H
#define FLOOR_HEAT_H 
#include "GenericDevice.h"
#include "FloorHeatDefinitions.h"

class FloorHeat : public GenericDevice {
    private:
        uint8_t*        commandData;
        uint8_t*        getCommandData(void) const { return commandData; } 

        friend class FloorHeatView;

        bool isTurnedOn(void) const {
            bool result = false;
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                uint8_t status = data[FLOOR_HEAT_GEN_STATUS_INDEX];
                if ((status & (FLOOR_HEAT_GEN_STATUS_HEATING_ON | FLOOR_HEAT_ELEMENT_STATUS_ON)) == (FLOOR_HEAT_GEN_STATUS_HEATING_ON | FLOOR_HEAT_ELEMENT_STATUS_ON)) {
                    result = true;
                    //printf("FloorHeat::isTurnedOn - heating is ON\n");
                    // keep it simple and make sure that only these bits are on
                    // data[FLOOR_HEAT_GEN_STATUS_INDEX] = FLOOR_HEAT_GEN_STATUS_HEATING_ON | FLOOR_HEAT_ELEMENT_STATUS_ON;
                }
            }
            return result;
        }   

        void turnFloorHeatOff(void) {
            uint8_t* cData = getCommandData();
            if (cData != nullptr) {
                cData[FLOOR_HEAT_GEN_STATUS_INDEX] = FLOOR_HEAT_COMMAND_OFF;
                cData[FLOOR_HEAT_COMMAND_SET_TEMPERATURE_MSB_INDEX] = 0x00;
                cData[FLOOR_HEAT_COMMAND_SET_TEMPERATURE_LSB_INDEX] = 0x00;
                executeCommand(FLOOR_HEAT_COMMAND, cData);
            }
        }

        void turnFloorHeatOn(float_t temperatureC) {
            uint8_t* cData = getCommandData();
            if (cData != nullptr) {
                // printf("FloorHeat::turnFloorHeatOn for index = %d - setting temperature to %f C\n",index(), temperatureC);
                uint16_t tempValue = convFromTempC(static_cast<double>(temperatureC));
                // printf("FloorHeat::turnFloorHeatOn for index = %d - converted temperature to raw int value %d\n",index(), tempValue);
                // cData[FLOOR_HEAT_GEN_STATUS_INDEX] = FLOOR_HEAT_GEN_STATUS_HEATING_ON | FLOOR_HEAT_ELEMENT_STATUS_ON;
                cData[FLOOR_HEAT_GEN_STATUS_INDEX] = FLOOR_HEAT_COMMAND_ON;

                setLilEndian(cData,
                             FLOOR_HEAT_COMMAND_SET_TEMPERATURE_MSB_INDEX, FLOOR_HEAT_COMMAND_SET_TEMPERATURE_LSB_INDEX, 
                             tempValue);
                // printf("FloorHeat::turnFloorHeatOn for index = %d - on/off = %x, command data msb: %02x, lsb: %02x\n", 
                //    index(), cData[FLOOR_HEAT_GEN_STATUS_INDEX], cData[FLOOR_HEAT_COMMAND_SET_TEMPERATURE_MSB_INDEX], cData[FLOOR_HEAT_COMMAND_SET_TEMPERATURE_LSB_INDEX]);
                executeCommand(FLOOR_HEAT_COMMAND, cData);
            }
        }

        void storeFloorHeat(void) {
            uint8_t* cData = getCommandData();
            if (cData != nullptr) {
                cData[FLOOR_HEAT_GEN_STATUS_INDEX] = FLOOR_HEAT_COMMAND_ON;
                uint16_t tempValue = convFromTempC(static_cast<double>(FLOOR_HEAT_TEMP_STORE));
                setLilEndian(cData,
                             FLOOR_HEAT_COMMAND_SET_TEMPERATURE_MSB_INDEX, FLOOR_HEAT_COMMAND_SET_TEMPERATURE_LSB_INDEX, 
                             tempValue);
                executeCommand(FLOOR_HEAT_COMMAND, cData);
                // cData[FLOOR_HEAT_COMMAND_SET_TEMPERATURE_MSB_INDEX] = 0x00;
                // cData[FLOOR_HEAT_COMMAND_SET_TEMPERATURE_LSB_INDEX] = 0x00;
            }
        }   

        float_t currentFloorHeatTemperature(void) const {
            float_t result = INVALID_TEMPERATURE; // invalid temperature
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                uint8_t msb = data[FLOOR_HEAT_STATUS_MEASURED_TEMPERATURE_MSB_INDEX];
                uint8_t lsb = data[FLOOR_HEAT_STATUS_MEASURED_TEMPERATURE_LSB_INDEX];
                // printf("FloorHeat::currentFloorHeatTemperature - raw msb: %02x, lsb: %02x\n", msb, lsb);
                uint16_t tempValue = getLilEndian(msb, lsb);
                // printf("FloorHeat::currentFloorHeatTemperature - combined raw value: %d\n", tempValue);
                result = convToTempC(tempValue);
                // printf("FloorHeat::currentFloorHeatTemperature - converted temperature: %f C\n", result);

                // uint16_t tempValue = (static_cast<uint16_t>(data[FLOOR_HEAT_STATUS_MEASURED_TEMPERATURE_MSB_INDEX]) << 8) |
                //                      static_cast<uint16_t>(data[FLOOR_HEAT_STATUS_MEASURED_TEMPERATURE_LSB_INDEX]);
                // result = static_cast<float_t>(tempValue) / 100.0F; // assuming the value is in hundredths of a degree Celsius
            }
            return result;
        }

        float_t currentFloorHeatSetting(void) const {
            float_t result = INVALID_TEMPERATURE; // invalid temperature
            uint8_t* data = getCurrentData();
            if (data != nullptr) {
                uint8_t msb = data[FLOOR_HEAT_STATUS_SET_TEMPERATURE_MSB_INDEX];
                uint8_t lsb = data[FLOOR_HEAT_STATUS_SET_TEMPERATURE_LSB_INDEX];
                uint16_t tempValue = getLilEndian(msb, lsb);
                result = convToTempC(tempValue);
                // uint16_t tempValue = (static_cast<uint16_t>(data[FLOOR_HEAT_STATUS_SET_TEMPERATURE_MSB_INDEX]) << 8) |
                //                     static_cast<uint16_t>(data[FLOOR_HEAT_STATUS_SET_TEMPERATURE_LSB_INDEX]);
                // result = static_cast<float_t>(tempValue) / 100.0F; // assuming the value is in hundredths of a degree Celsius
            }
            return result;
        }

    protected:
        virtual CAN_frame_t* buildCommand(RVC_DGN dgn);
        virtual void setData(RVC_DGN dgn, uint8_t* data) {
            if (data != nullptr) {
                uint8_t* rawData = getCurrentData();
                uint8_t* cData = getCommandData();
                if ((rawData != nullptr) && (cData != nullptr))// set the current data to the new data
                switch (dgn) {

                    case FLOOR_HEAT_COMMAND:
                        // printf("FloorHeat::setData: FLOOR_HEAT_COMMAND received\n");
                        for (uint8_t i = 1; i < 8; i++) {
                            cData[i] = data[i]; // copy the command data
                        }
                        break;
                    case FLOOR_HEAT_STATUS:
                        // printf("FloorHeat::setData: FLOOR_HEAT STATUS received for index %d: ", index());
                        for (uint8_t i = 1; i < 8; i++) {
                            rawData[i] = data[i]; // copy the status data
                            // printf(" - byte %d: %02x", i, data[i]);
                        }
                        // printf("\n");
                        break;
                    default:
                        // do nothing
                        break;
                }
            }
        }

    public:
        FloorHeat() : GenericDevice() {
            // Constructor implementation
            // Constructor implementation
            commandData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            commandData[0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                commandData[i] = INVALID_DATA; // initialize to invalid data
            }
        }
        FloorHeat(const FloorHeat& orig) : GenericDevice(orig) {
            // Copy constructor implementation
            commandData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            for (uint8_t i = 0; i < 8; i++) {
                commandData[i] = orig.commandData[i]; // initialize to invalid data
            }
        }
        FloorHeat(uint8_t address, uint8_t instance) : GenericDevice(address, instance) {     
            // Constructor with parameters implementation
            commandData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            commandData[0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                commandData[i] = INVALID_DATA; // initialize to invalid data
            }
        }
        FloorHeat(uint8_t* data) : GenericDevice(data) {
            // Constructor with parameters implementation
            commandData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            commandData[0] = index(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                commandData[i] = INVALID_DATA; // initialize to invalid data
            }
        }
        virtual ~FloorHeat() {
            // Destructor implementation
            if (commandData != nullptr) {
                delete[] commandData;
                commandData = nullptr;
            }
        }   


        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* sendData = nullptr, uint8_t sAddress = SOURCE_ADDRESS); // execute command based on DGN and data received from the controller

    
};
#endif  // ifndef FLOOR_HEAT_H
#endif // ifdef HOME_KIT_2