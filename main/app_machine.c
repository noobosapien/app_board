#include "app_machine.h"

static APP_STATE current_state = APP_INITIALIZE;

void run_app_state_machine(){
    while(1){
        switch(current_state){
            case APP_INITIALIZE:
            // Display initialize screen
            // Initialize WIFI
            // Initialize BLE
            // Change state to READY
            break;

            case APP_READY:
            // Display ready screen
            // Wait until next is pressed
            // Change state to WIFI_SCANNING
            break;

            case APP_WIFI_SCANNING:
            // Display Wifi scanning screen
            // Scan for Wifi APs 
            // Change state to WIFI_FOUND/WIFI_NOT_FOUND depending on return value
            break;

            case APP_WIFI_FOUND:
            // Display Wifi found screen with the APs
            // Wait for next/back button press 
            // Change state to BLE_SCANNING/READY depending on the button press
            break;

            case APP_WIFI_NOT_FOUND:
            // Display Wifi not found screen
            // Wait for next/back button press 
            // Change state to BLE_SCANNING/READY depending on the button press
            break;
        
            case APP_BLE_SCANNING:
            // Display BLE scanning screen
            // Scan for BLE devices 
            // Change state to BLE_FOUND/BLE_NOT_FOUND depending on return value
            break;

            case APP_BLE_FOUND:
            // Display BLE found screen with the devices
            // Wait for back button press 
            // Change state to WIFI_SCANNING if back is pressed
            break;
        
            case APP_BLE_NOT_FOUND:
            // Display BLE not found screen with the devices
            // Wait for back button press 
            // Change state to WIFI_SCANNING if back is pressed
            break;

            default:
            // Wrong state change back to initializing
            current_state = APP_INITIALIZE;
            break;
        }
    }  
}