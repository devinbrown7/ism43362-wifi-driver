#include "mbed.h"

#include "ISM43362Interface.h"

using namespace std;

ISM43362Interface wifi(MBED_CONF_APP_WIFI_TX, MBED_CONF_APP_WIFI_RX);

Serial pc(USBTX, USBRX);

int main() {
    pc.baud(115200);
    printf("\n ----------------- \n|  ISM43362 WiFi  |\n ----------------- \n\n");
}
