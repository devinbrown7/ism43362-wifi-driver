#include "mbed.h"
#include "TCPSocket.h"

#include "ISM43362Interface.h"
#include "ntp-client/NTPClient.h"
#include "Websocket.h"

using namespace std;

ISM43362Interface wifi;

Serial pc(USBTX, USBRX);
DigitalOut led1(LED1);

Thread blinkThread;
Thread wifiThread;

#define logMessage printf
#define MQTTCLIENT_QOS2 1

#include "MQTTNetwork.h"
#include "MQTTmbed.h"
#include "MQTTClient.h"

typedef struct {
    uint32_t secs;         // Transmit Time-stamp seconds.
}ntp_packet;

void ntpDemo();
void wsDemo();
void udpServerDemo();
void udpClientDemo();
void mqttDemo();

uint32_t ntohl(uint32_t n)
{
  return ((n & 0xff) << 24) |
    ((n & 0xff00) << 8) |
    ((n & 0xff0000UL) >> 8) |
    ((n & 0xff000000UL) >> 24);
}

int arrivedcount = 0;

void messageArrived(MQTT::MessageData& md)
{
    MQTT::Message &message = md.message;
    logMessage("Message arrived: qos %d, retained %d, dup %d, packetid %d\r\n", message.qos, message.retained, message.dup, message.id);
    logMessage("Payload %.*s\r\n", message.payloadlen, (char*)message.payload);
    ++arrivedcount;
}

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
    response = socket.connect("10.10.1.197", 8080);
    if(0 != response) {
        printf("Error connecting: %d\n", response);
        socket.close();
        return;
    }

    // Send a simple http request
    // char sbuffer[] = "GET / HTTP/1.1\r\nHost: www.arm.com\r\n\r\n";
    char sbuffer[] = "GET / HTTP/1.1\r\nHost: 10.10.1.197\r\n\r\n";
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
        printf("WiFi example\n\n");

        // int count = 0;
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

        // CHOOSE ONE!

        // ntpDemo();
        // wsDemo();
        // udpServerDemo();
        // udpClientDemo();
        // mqttDemo();
}

void ntpDemo() {
    printf("NTP Client example (using Ethernet)\r\n");
    printf("Client IP Address is %s\r\n", wifi.get_ip_address());

    NTPClient ntp(&wifi);

    while(1) {
        time_t timestamp = ntp.get_timestamp();

        if (timestamp < 0) {
            printf("An error occurred when getting the time. Code: %ld\r\n", timestamp);
        } else {
            printf("Current time is %s\r\n", ctime(&timestamp));
        }

        printf("Waiting 10 seconds before trying again...\r\n");
        wait(10.0);
    }
}

void wsDemo() {

    printf("IP Address is %s\n\r", wifi.get_ip_address());

    // Websocket ws("ws://sockets.mbed.org:443/ws/demo/rw");
    Websocket ws((char *)"ws://demos.kaazing.com/echo", &wifi);
    ws.connect();

    char recv[100];

    while (1) {
        ws.send((char *)"WebSocket Hello World!");
        if (ws.read(recv)) {
            printf("rcv: %s\r\n", recv);
        }
        wait(0.1);
    }
}

void udpServerDemo() {
     printf("IP Address is %s\n\r", wifi.get_ip_address());

     UDPSocket socket;
     const uint buffer_size = 256;
     char buffer[buffer_size];

     if (socket.open(&wifi) < 0) {
         printf("Failed to open UDP Socket\n\r");
         return;
     }

     if (socket.bind(42) < 0) {
         printf("Failed to bind UDP Socket to port 42\n\r");
     }

     while (true) {
         Thread::wait(2000);
         SocketAddress s_addr;
         int result = socket.recvfrom(&s_addr, buffer, buffer_size);

         switch (result) {
             case -1:
                 printf("Failed to read from UDP Socket\n\r");
                 return;

             case 0:
                 printf("Nothing received...?\n\r");
                 continue;

             default:
                 printf("Received %d bytes from %s:%d\n\r", result,
                     s_addr.get_ip_address(),
                     s_addr.get_port());

                 printf("Message: %s\n\r", (char *)buffer);

                 if (strlen(s_addr.get_ip_address()) > 0) {
                     socket.sendto(s_addr.get_ip_address(), s_addr.get_port(), buffer, result);
                 }
                 continue;
        }
     }
}

void udpClientDemo() {
    // Bring up the ethernet interface
    printf("UDP Socket example\n");

    // Show the network address
    const char *ip = wifi.get_ip_address();
    printf("IP address is: %s\n", ip ? ip : "No IP");

    UDPSocket sock(&wifi);
    SocketAddress sockAddr;

    char out_buffer[] = "time";
    if(0 > sock.sendto("time.nist.gov", 37, out_buffer, sizeof(out_buffer))) {
        printf("Error sending data\n");
        return;
    }

    ntp_packet in_data;
    sock.recvfrom(&sockAddr, &in_data, sizeof(ntp_packet));
    in_data.secs = ntohl(in_data.secs) - (uint32_t)2208988800;    // 1900-1970
    printf("Time Received %lu seconds since 1/01/1900 00:00 GMT\n",
                        (uint32_t)in_data.secs);
    printf("Time = %s", ctime(( const time_t* )&in_data.secs));

    printf("Time Server Address: %s Port: %d\n\r",
                               sockAddr.get_ip_address(), sockAddr.get_port());

    // Close the socket and bring down the network interface
    sock.close();
    wifi.disconnect();
    return;
}

void mqttDemo() {
    float version = 0.6;
    char* topic = (char *)"mbed-sample-db";

    logMessage("HelloMQTT: version is %.2f\r\n", version);

    MQTTNetwork mqttNetwork(&wifi);

    MQTT::Client<MQTTNetwork, Countdown> client(mqttNetwork);

    //const char* hostname = "m2m.eclipse.org";
    const char* hostname = "broker.hivemq.com";
    int port = 1883;
    logMessage("Connecting to %s:%d\r\n", hostname, port);
    int rc = mqttNetwork.connect(hostname, port);
    if (rc != 0)
        logMessage("rc from TCP connect is %d\r\n", rc);

    MQTTPacket_connectData data = MQTTPacket_connectData_initializer;
    data.MQTTVersion = 3;
    data.clientID.cstring = (char *)"clientId-lySQ9Xr7MG";
    data.username.cstring = (char *)"testuser";
    data.password.cstring = (char *)"testpassword";
    if ((rc = client.connect(data)) != 0)
        logMessage("rc from MQTT connect is %d\r\n", rc);

    if ((rc = client.subscribe(topic, MQTT::QOS2, messageArrived)) != 0)
        logMessage("rc from MQTT subscribe is %d\r\n", rc);

    MQTT::Message message;

    // QoS 0
    char buf[100];
    sprintf(buf, "Hello World!  QoS 0 message from app version %f\r\n", version);
    message.qos = MQTT::QOS0;
    message.retained = false;
    message.dup = false;
    message.payload = (void*)buf;
    message.payloadlen = strlen(buf)+1;
    rc = client.publish(topic, message);
    while (arrivedcount < 1)
        client.yield(100);

    // QoS 1
    sprintf(buf, "Hello World!  QoS 1 message from app version %f\r\n", version);
    message.qos = MQTT::QOS1;
    message.payloadlen = strlen(buf)+1;
    rc = client.publish(topic, message);
    while (arrivedcount < 2)
        client.yield(100);

    // QoS 2
    sprintf(buf, "Hello World!  QoS 2 message from app version %f\r\n", version);
    message.qos = MQTT::QOS2;
    message.payloadlen = strlen(buf)+1;
    rc = client.publish(topic, message);
    while (arrivedcount < 3)
        client.yield(100);

    if ((rc = client.unsubscribe(topic)) != 0)
        logMessage("rc from unsubscribe was %d\r\n", rc);

    if ((rc = client.disconnect()) != 0)
        logMessage("rc from disconnect was %d\r\n", rc);

    mqttNetwork.disconnect();

    logMessage("Version %.2f: finish %d msgs\r\n", version, arrivedcount);

    return;
}

int main()
{
    pc.baud(115200);
    printf("\n ----------------- \n|  ISM43362 WiFi  |\n ----------------- \n\n");

    blinkThread.start(blink);
    wifiThread.start(wifiDemo);
    wifiThread.join();
}
