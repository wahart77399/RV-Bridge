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

#include "SpanView.h"
#include "HomeSpan.h"
#include "GenericDevice.h"

boolean SpanView::prepped = false;

void SpanView::setCallBack(char c, const char* s, void (*f)(const char *)) {
    new SpanUserCommand(c, s, f);
}

const char* SpanView::getValuePair(const char* buff, int16_t& val1, int16_t& val2) {
	val1 = -1;
	val2 = -2;

	const char* equals = strchr(buff, '=');

	if (equals) {
		equals += 1;
		val1 = atoi(buff);
		val2 = atoi(equals);

		buff = strchr(equals, ',');
		if (buff) {
			buff += 1;
		}
	}
	else {
		buff = NULL;
	}

	return buff;
}

uint8_t SpanView::indexOfModel(void) const {
	uint8_t index = -1; 
    if (model != nullptr) 
		index = model->getInstance();
	return index; 
}

void SpanView::prepHomeSpan(void) {
	// This method can be used to prepare HomeSpan for the view
	// For example, setting up characteristics or services
	if (!SpanView::prepped) {
		printf("SpanView::prepHomeSpan called\n");
		// Set up HomeSpan characteristics or services here
		// Example: HomeSpan.addAccessory(new SpanAccessory());
		// This is just a placeholder, actual implementation may vary
		SPAN_ACCESSORY(); // creates bridge for HomeSpan - called only once.
		// new SpanAccessory();
		// new Service::AccessoryInformation();
		// new Characteristic::Identify();
		SpanView::prepped = true;
		printf("SpanView::prepHomeSpan - completed Preparing HomeSpan for SpanView...\n");
	}
}

SpanView::SpanView() : model(nullptr) { 
	SpanView::prepHomeSpan();
}

SpanView::SpanView(GenericDevice* refModel) : model(refModel)  { 
	SpanView::prepHomeSpan(); 
	if (model != nullptr)
		model->addView(this); 
}