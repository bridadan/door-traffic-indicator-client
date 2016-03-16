/*
 * PackageLicenseDeclared: Apache-2.0
 * Copyright (c) 2015 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "mbed-drivers/mbed.h"
#include "security.h"
#include "simpleclient.h"
#include "lwipv4_init.h"
#include <string>
#include <sstream>
#include <vector>

#include "DoorIndicator.h"

DoorIndicator doorIndicator(D6, D3, D0, D1);

using namespace mbed::util;

Serial &output = get_stdio_serial();

EthernetInterface eth;

// These are example resource values for the Device Object
struct MbedClientDevice device = {
    "Manufacturer_String",      // Manufacturer
    "Type_String",              // Type
    "ModelNumber_String",       // ModelNumber
    "SerialNumber_String"       // SerialNumber
};

// Instantiate the class which implements LWM2M Client API (from simpleclient.h)
MbedClient mbed_client(device);

// Set up Hardware interrupt button.
InterruptIn obs_button(SW2);
InterruptIn unreg_button(SW3);

// LED Output
DigitalOut led1(LED1);

/*
 * The button contains one property (click count).
 * When `handle_button_click` is executed, the counter updates.
 */
class StateResource {
public:
    StateResource() {
        // create ObjectID with metadata tag of '3200', which is 'digital input'
        btn_object = M2MInterfaceFactory::create_object("3200");
        M2MObjectInstance* btn_inst = btn_object->create_object_instance();
        // create resource with ID '5501', which is digital input counter
        M2MResource* btn_res = btn_inst->create_dynamic_resource("5501", "State",
            M2MResourceInstance::INTEGER, true /* observable */);
        // we can read this value
        btn_res->set_operation(M2MBase::GET_ALLOWED);
        // set initial value (all values in mbed Client are buffers)
        // to be able to read this data easily in the Connector console, we'll use a string
        btn_res->set_value((uint8_t*)"0", 1);
    }

    M2MObject* get_object() {
        return btn_object;
    }

    /*
     * When you press the button, we read the current value of the click counter
     * from mbed Device Connector, then up the value with one.
     */
    void handle_state_change(DoorIndicator::WarnStatus warnStatus) {
        M2MObjectInstance* inst = btn_object->object_instance();
        M2MResource* res = inst->resource("5501");

        printf("handle_state_change, new value of state is %d\r\n", (int) warnStatus);

        // serialize the value of counter as a string, and tell connector
        stringstream ss;
        ss << (int) warnStatus;
        std::string stringified = ss.str();
        res->set_value((uint8_t*)stringified.c_str(), stringified.length());
    }

private:
    M2MObject* btn_object;
    uint16_t counter = 0;
};

void app_start(int /*argc*/, char* /*argv*/[]) {

    //Sets the console baud-rate
    output.baud(115200);
    output.printf("In app_start()\r\n");

    // This sets up the network interface configuration which will be used
    // by LWM2M Client API to communicate with mbed Device server.
    eth.init();     //Use DHCP
    if (eth.connect() != 0) {
        output.printf("Failed to form a connection!\r\n");
    }
    if (lwipv4_socket_init() != 0) {
        output.printf("Error on lwipv4_socket_init!\r\n");
    }
    output.printf("IP address %s\r\n", eth.getIPAddress());
    output.printf("Device name %s\r\n", MBED_ENDPOINT_NAME);

    // we create our button and LED resources
    auto state_resource = new StateResource();
    
    doorIndicator.onStateChange(mbed::util::FunctionPointer1<void, DoorIndicator::WarnStatus>(state_resource, &StateResource::handle_state_change));

    // Unregister button (SW3) press will unregister endpoint from connector.mbed.com
    unreg_button.fall(&mbed_client, &MbedClient::test_unregister);

    // Create endpoint interface to manage register and unregister
    mbed_client.create_interface();

    // Create Objects of varying types, see simpleclient.h for more details on implementation.
    M2MSecurity* register_object = mbed_client.create_register_object(); // server object specifying connector info
    M2MDevice*   device_object   = mbed_client.create_device_object();   // device resources object

    // Create list of Objects to register
    M2MObjectList object_list;

    // Add objects to list
    object_list.push_back(device_object);
    object_list.push_back(state_resource->get_object());

    // Set endpoint registration object
    mbed_client.set_register_object(register_object);
    
    mbed_client.on_object_registered(mbed::util::FunctionPointer0<void>(&doorIndicator, &DoorIndicator::init));

    // Issue register command.
    FunctionPointer2<void, M2MSecurity*, M2MObjectList> fp(&mbed_client, &MbedClient::test_register);
    minar::Scheduler::postCallback(fp.bind(register_object,object_list));
    minar::Scheduler::postCallback(&mbed_client,&MbedClient::test_update_register).period(minar::milliseconds(25000));
}