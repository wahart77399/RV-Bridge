
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
#ifndef SPAN_VIEW_H
#define SPAN_VIEW_H
#include "Arduino.h"
#include <map>
// #include <mutex>

class GenericDevice;
class SpanUserCommand;

class SpanView {
    private:
        GenericDevice* model;

        SpanView& operator=(SpanView& v) = delete;

        static boolean prepped;

    protected:
        const float HomeKitPercentMax = (100.0F);

        void setCallBack(char c, const char *s, void (*f)(const char *)); // call back from HomeSpan to tell the model what needs to be done next

        const char* getValuePair(const char* buff, int16_t& val1, int16_t& val2);
             

        uint8_t indexOfModel(void) const;
        GenericDevice* getModel(void)  { return model; }

        

    public:

        SpanView(); // : model(nullptr) { SpanView::prepHomeSpan();}
        SpanView(GenericDevice* refModel); // : model(refModel)  { SpanView::prepHomeSpan(); model->addView(this); }
        SpanView(SpanView& copy) { model = copy.model; }

        ~SpanView() {}


        virtual bool updateView(void) = 0;
        static void prepHomeSpan(void);
};

#endif // span view