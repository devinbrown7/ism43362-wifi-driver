#include "mbed.h"
#include "TCPSocket.h"

#include "ISM43362Interface.h"

using namespace std;

ISM43362Interface wifi;

Serial pc(USBTX, USBRX);
DigitalOut led1(LED1);

Thread blinkThread;
Thread wifiThread;

// Double blink
void blink() {
    led1 = 0;
    while (true) {
        led1 = 1;
        Thread::wait(100);
        led1 = 0;
        Thread::wait(100);
        led1 = 1;
        Thread::wait(100);
        led1 = 0;
        Thread::wait(700);
    }
}

const char *sec2str(nsapi_security_t sec)
{
    switch (sec) {
        case NSAPI_SECURITY_NONE:
            return "None";
        case NSAPI_SECURITY_WEP:
            return "WEP";
        case NSAPI_SECURITY_WPA:
            return "WPA";
        case NSAPI_SECURITY_WPA2:
            return "WPA2";
        case NSAPI_SECURITY_WPA_WPA2:
            return "WPA/WPA2";
        case NSAPI_SECURITY_UNKNOWN:
        default:
            return "Unknown";
    }
}

int scan_demo(WiFiInterface *wifi)
{
    WiFiAccessPoint *ap;
    uint8_t count = 20;
    ap = new WiFiAccessPoint[count];

    printf("Scan:\n");

    count = wifi->scan(ap, count);
    for (int i = 0; i < count; i++) {
        printf("Network: %s secured: %s BSSID: %hhX:%hhX:%hhX:%hhx:%hhx:%hhx RSSI: %hhd Ch: %hhd\n", ap[i].get_ssid(),
               sec2str(ap[i].get_security()), ap[i].get_bssid()[0], ap[i].get_bssid()[1], ap[i].get_bssid()[2],
               ap[i].get_bssid()[3], ap[i].get_bssid()[4], ap[i].get_bssid()[5], ap[i].get_rssi(), ap[i].get_channel());
    }
    printf("%d networks available.\n", count);

    delete[] ap;
    return count;
}

void http_demo(NetworkInterface *net)
{
    TCPSocket socket;
    nsapi_error_t response;

    printf("Sending HTTP request to www.arm.com...\n");

    // Open a socket on the network interface, and create a TCP connection to www.arm.com
    socket.open(net);
    // response = socket.connect("www.arm.com", 80);
    response = socket.connect("10.10.1.185", 80);
    if(0 != response) {
        printf("Error connecting: %d\n", response);
        socket.close();
        return;
    }

    // Send a simple http request
    // char sbuffer[] = "GET / HTTP/1.1\r\nHost: www.arm.com\r\n\r\n";
    char sbuffer[] = "GET / HTTP/1.1\r\nHost: www.google.com\r\n\r\n";
    nsapi_size_t size = strlen(sbuffer);
    response = 0;
    while(size)
    {
        response = socket.send(sbuffer+response, size);
        if (response < 0) {
            printf("Error sending data: %d\n", response);
            socket.close();
            return;
        } else {
            size -= response;
            // Check if entire message was sent or not
            printf("sent %d [%.*s]\n", response, strstr(sbuffer, "\r\n")-sbuffer, sbuffer);
        }
    }

    // Recieve a simple http response and print out the response line
    char rbuffer[64];
    response = socket.recv(rbuffer, sizeof rbuffer);
    if (response < 0) {
        printf("Error receiving data: %d\n", response);
    } else {
        printf("recv %d [%.*s]\n", response, strstr(rbuffer, "\r\n")-rbuffer, rbuffer);
    }

    // Close the socket to return its memory and bring down the network interface
    socket.close();
}

void wifiDemo() {

        int count = 0;

        printf("WiFi example\n\n");

        // count = scan_demo(&wifi);
        // if (count == 0) {
        //     printf("No WIFI APNs found - can't continue further.\n");
        //     return;
        // }

        printf("\nConnecting to %s...\n", MBED_CONF_APP_WIFI_SSID);
        int ret = wifi.connect(MBED_CONF_APP_WIFI_SSID, MBED_CONF_APP_WIFI_PASSWORD, NSAPI_SECURITY_WPA2);
        if (ret != 0) {
            printf("\nConnection error\n");
            return;
        }

        printf("Success\n\n");
        printf("MAC: %s\n", wifi.get_mac_address());
        printf("IP: %s\n", wifi.get_ip_address());
        printf("Netmask: %s\n", wifi.get_netmask());
        printf("Gateway: %s\n", wifi.get_gateway());
        printf("RSSI: %d\n\n", wifi.get_rssi());

        http_demo(&wifi);

        wifi.disconnect();

        printf("\nDone\n");
}

int main()
{
    pc.baud(115200);
    printf("\n ----------------- \n|  ISM43362 WiFi  |\n ----------------- \n\n");

    blinkThread.start(blink);
    wifiThread.start(wifiDemo);
    wifiThread.join();
}
