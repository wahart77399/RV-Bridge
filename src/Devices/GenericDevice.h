#ifndef GENERIC_DEVICE_H
#define GENERIC_DEVICE_H

/*********************************************************************************
 *  MIT License
 *  
 *  Copyright (c) 2023 Randy Ubillos
 *  
 *  https://github.com/rubillos/RV-Bridge
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 *  
 ********************************************************************************/
 
////////////////////////////////////////////////////////////////
//                                                            //
//    RV-Bridge: A HomeKit to RV-C interface for the ESP32    //
//                                                            //
////////////////////////////////////////////////////////////////
#include "Arduino.h"
#include "elapsedMillis.h"
#include <list>
#include "DGN.h"
#include "Packet.h"
// #include "SpanView.h"

class SpanView;

// generic


class GenericDevice {
    private:
        const uint8_t INVALID_DATA = 0xff;
        const uint8_t DC_LAST_COMMAND = 5; 
        std::list <SpanView*>   views;
        CAN_frame_t frame; // frame to be used for sending commands
        uint8_t* currentData;
        uint8_t sourceAddress;
        uint8_t instance;
        uint8_t group; // maybe?
        std::list <RVC_DGN> availableDGNs; // list of available DGN numbers for this device 

        const uint8_t getInstance(void) const { return instance; }
        void setInstance(uint8_t curInstance) { instance = curInstance; }
        uint8_t getGroup(void) { return group; }
        void setGroup(uint8_t curGroup) { group = curGroup; }

        std::list<SpanView* >::iterator getViews(void) { 
            return views.begin(); 
        }
        
        const boolean isLastView(std::list<SpanView*>::iterator it) { return (it == views.end()); }
        const boolean isViewsEmpty(void) const { return views.empty(); }
        
       

        friend class SpanView; // ::cmdSendOnOff(const char *buff);
        friend void Packet::processPacket(CAN_frame_t *packet); // allow Packet to access private members
    protected:
        
        inline uint8_t* getCurrentData(void) const {
            return currentData;
        }

        inline void setCurrentData(const uint8_t* data) {
            if ((data != currentData) && (currentData != nullptr) && (data != nullptr)) {
                //if (currentData != nullptr) {
                //    delete[] currentData; // delete old data
                //}
                for (uint8_t i = 1; i < 8; i++) {
                    currentData[i] = data[i]; // copy the data
                }
            }
            // currentData = (uint8_t* )data;
            
        }

        virtual void setData(RVC_DGN dgn, uint8_t* data) = 0;

        inline const boolean isValidData(void) const {
                boolean dataGood = false; 
                if (currentData != nullptr)
                    dataGood = true;

                return dataGood;
        }
        
        inline const uint8_t getByte(uint8_t index)  const { 
            uint8_t result = INVALID_DATA;
            if (isValidData()) {
                uint8_t* tempData = getCurrentData();
                result = tempData[index];
            }
            return result;
        }
        
        // virtual void updateModel(void) = 0;

        virtual CAN_frame_t* buildCommand(RVC_DGN dgn);
        // virtual boolean sendCommand(RVC_DGN dgn);

        virtual void updateViews();
        virtual void addView(SpanView*); // this transfers ownership of the view to the model
        virtual void deleteViews();

        GenericDevice() :
             views(), 
             currentData(nullptr), 
             instance(0), 
             group(0) {
                // all done in initializers    
        }

        GenericDevice(uint8_t address, uint8_t index, uint8_t grp, std::list <RVC_DGN> dgns) :
            views(),
            currentData(nullptr),
            sourceAddress(address),
            instance(index),
            group(grp) { 
            
            availableDGNs = dgns;
            currentData = new uint8_t[sizeof(uint8_t) * 8]; // allocate 8 bytes for the data
            currentData[0] = getInstance(); // set the first byte to the instance index
            for (uint8_t i = 1; i < 8; i++) {
                currentData[i] = INVALID_DATA; // initialize to invalid data
            }
        }


        GenericDevice(const GenericDevice& orig) :
            views(),
            frame(orig.frame),  // may need to allocate memory and deep copy - not sure yet
            currentData(nullptr),
            sourceAddress(orig.sourceAddress),
            instance(orig.instance),
            group(orig.group),
            availableDGNs(orig.availableDGNs) { 
            // Copy constructor
            if (orig.currentData) {
                // ********************** WARNING ****************************************************************
                // this wont work, it will alocate the size of the pointer - I believe it needs to be 8 bytes long
                currentData = new uint8_t[sizeof(uint8_t) * 8];
                memcpy(currentData, orig.currentData, sizeof(orig.currentData));
            }             
        }

        GenericDevice(uint8_t* data) :
             views(), 
             currentData(nullptr), 
             instance(0), 
             group(0) {
            // Constructor with parameters
            if (data != nullptr) {
                if (currentData != nullptr) {
                    delete[] currentData; // delete old data
                }
                this->instance = Packet::getIndex(data);
                this->group = Packet::getGroup(data);
                // ********************** WARNING ****************************************************************
                // this wont work, it will alocate the size of the pointer - I believe it needs to be 8 bytes long
                currentData = new uint8_t[sizeof(uint8_t) * 8];
                memcpy(currentData, data, sizeof(data));
            }  
        }

        // assign operator
        GenericDevice& operator=(const GenericDevice& orig) {
            if (this != &orig) {
                currentData = nullptr; // Clear existing data
                if (orig.currentData) {
                    // ********************** WARNING ****************************************************************
                    // this wont work, it will alocate the size of the pointer - I believe it needs to be 8 bytes long
                    currentData = new uint8_t[sizeof(orig.currentData)];
                    memcpy(currentData, orig.currentData, sizeof(orig.currentData));
                } 
                sourceAddress = orig.sourceAddress;
                instance = orig.instance;
                group = orig.group;
                availableDGNs = orig.availableDGNs;
                frame = orig.frame;
                // availableDGNs = nullptr; // Initialize to nullptr
                //if (orig.availableDGNs != nullptr) {
                //    availableDGNs = new RVC_DGN[sizeof(orig.availableDGNs)];
                //    memcpy(availableDGNs, orig.availableDGNs, sizeof(orig.availableDGNs));
                //}
            }
            return *this;
        }

        virtual void printDevice(void);
    public:

        ~GenericDevice() {
            // Destructor
            if (currentData) 
                delete[] currentData;
            // if (availableDGNs) 
            //    delete[] availableDGNs;
            deleteViews();
            
        }
        inline const uint8_t index(void) const { return getInstance(); }

        // Print packet data based on DGN and data
        // This function will print the packet data based on the DGN and data provided.
        // It will check the printPacket mode and print the data accordingly.
        // If printPacket is packetPrintIfKnown or packetPrintYes, it will print the
        // DGN, instance, group, and data.
        // virtual void printPacketData(RVC_DGN dgn, uint8_t* data, PacketPrint printPacket);
        
        // Execute command based on DGN and data
        // This function will execute the command based on the DGN and data provided.
        // It will check the data and process the command based on the DGN.
        // It will return true if the command was executed successfully, otherwise false.
        // The function will also set the currentData to the data provided.
        // It will also set the instance and group based on the data provided.
        // If the DGN is GENERIC_INDICATOR_COMMAND, it will handle the indicator
        // command. If the DGN is GENERIC_CONFIGURATION_STATUS, it will handle the
        // configuration status. For other DGN values, it will handle them accordingly.
        // If the data is null, it will return false.
        // If the command is executed successfully, it will return true.
        // If the command is not recognized, it will return false.
        // This function is virtual and can be overridden by derived classes.
        // It is expected that derived classes will implement their own command execution logic.
        virtual boolean executeCommand(RVC_DGN dgn, const uint8_t* buffer, uint8_t val=SOURCE_ADDRESS);
    
};



#endif // GENERIC_DEVICE_H