/* ISM43362 Example
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

#include "ISM43362.h"
#include "wifi.h"

#define ISM43362_DEFAULT_BAUD_RATE 115200

ISM43362::ISM43362()
{
    WIFI_Init();
}

int ISM43362::get_firmware_version()
{
    // _parser.send("AT+GMR");
    // int version;
    // if(_parser.recv("SDK version:%d", &version) && _parser.recv("OK")) {
    //     return version;
    // } else {
    //     // Older firmware versions do not prefix the version with "SDK version: "
    //     return -1;
    // }

    // TODO

    return 0;

}

bool ISM43362::startup(int mode)
{
    // //only 3 valid modes
    // if(mode < 1 || mode > 3) {
    //     return false;
    // }
    //
    // bool success = _parser.send("AT+CWMODE_CUR=%d", mode)
    //     && _parser.recv("OK")
    //     && _parser.send("AT+CIPMUX=1")
    //     && _parser.recv("OK");
    //
    // _parser.oob("+IPD", callback(this, &ISM43362::_packet_handler));
    //
    // return success;

    // TODO

    return false;
}

bool ISM43362::reset(void)
{
    return WIFI_ResetModule() == WIFI_STATUS_OK;
}

bool ISM43362::dhcp(bool enabled)
{
    return WIFI_SetDhcp((uint8_t)enabled) == WIFI_STATUS_OK;;
}

bool ISM43362::connect(const char *ssid, const char *passPhrase, nsapi_security_t security)
{
    WIFI_Ecn_t wifi_ecn = nsapi_security2wifi_ecn(security);
    WIFI_Status_t status = WIFI_Connect(ssid, passPhrase, wifi_ecn);
    return status == WIFI_STATUS_OK;
}

bool ISM43362::disconnect(void)
{
    // return _parser.send("AT+CWQAP") && _parser.recv("OK");

    // TODO

    return false;
}

const char *ISM43362::getIPAddress(void)
{
    if (WIFI_GetIP_Address((uint8_t *)_ip_buffer) == WIFI_STATUS_OK) {
        sprintf(_ip_buffer, "%d.%d.%d.%d", _ip_buffer[0], _ip_buffer[1], _ip_buffer[2], _ip_buffer[3]);
        return _ip_buffer;
    } else {
        return NULL;
    }
}

const char *ISM43362::getMACAddress(void)
{
    if (WIFI_GetMAC_Address((uint8_t *)_mac_buffer) == WIFI_STATUS_OK) {
        sprintf(_mac_buffer, "%02X:%02X:%02X:%02X:%02X:%02X", _mac_buffer[0], _mac_buffer[1], _mac_buffer[2], _mac_buffer[3], _mac_buffer[4], _mac_buffer[5]);
        return _mac_buffer;
    } else {
        return NULL;
    }
}

const char *ISM43362::getGateway()
{
    if (WIFI_GetGateway((uint8_t *)_gateway_buffer) == WIFI_STATUS_OK) {
        sprintf(_gateway_buffer, "%d.%d.%d.%d", _gateway_buffer[0], _gateway_buffer[1], _gateway_buffer[2], _gateway_buffer[3]);
        return _gateway_buffer;
    } else {
        return NULL;
    }
}

const char *ISM43362::getNetmask()
{
    if (WIFI_GetNetmask((uint8_t *)_netmask_buffer) == WIFI_STATUS_OK) {
        sprintf(_netmask_buffer, "%d.%d.%d.%d", _netmask_buffer[0], _netmask_buffer[1], _netmask_buffer[2], _netmask_buffer[3]);
        return _netmask_buffer;
    } else {
        return NULL;
    }
}

int8_t ISM43362::getRSSI()
{
    int8_t rssi;
    if (WIFI_GetRssi(&rssi) == WIFI_STATUS_OK) {
        return rssi;
    } else {
        return -1;
    }
}

bool ISM43362::isConnected(void)
{
    // return getIPAddress() != 0;

    // TODO

    return false;
}

int ISM43362::scan(WiFiAccessPoint *res, unsigned limit)
{
    WIFI_APs_t *aps = (WIFI_APs_t*)malloc(sizeof(WIFI_APs_t));
    // WIFI_APs_t apss;
    // WIFI_APs_t *aps = &apss;
    uint8_t count = 0;
    WIFI_Status_t status;
    status = WIFI_ListAccessPoints(aps, limit);
    for (count = 0; count < aps->count; count++) {
        nsapi_wifi_ap_t ap;
        wifi_ap2ns_api_wifi_ap(&aps->ap[count], &ap);
        res[count] = WiFiAccessPoint(ap);
    }
    free(aps);
    return count;
}

bool ISM43362::open(nsapi_protocol_t type, int id, const char* addr, int port)
{
    printf("ISM43362::open socket %d - %s:%d\n", id, addr, port);
    WIFI_Protocol_t proto = nsapi_protocol2WIFI_Protocol(type);
    SocketAddress s_addr(addr, (uint16_t)port);
    //return  == WIFI_STATUS_OK;

    printf("ISM43362::open: AAA\n");
    uint8_t status = WIFI_OpenClientConnection(id, proto, "Socket Name", (uint8_t*)s_addr.get_ip_bytes(), (uint16_t)port, (uint16_t)6000);
    printf("ISM43362::open: BBB status %d\n", status);
    //  == WIFI_STATUS_OK;
    return status == WIFI_STATUS_OK;
}

int ISM43362::dns_lookup(const char* name, char* ip)
{
    int ret = NSAPI_ERROR_DNS_FAILURE;
    uint8_t ipAddr[4];
    if (WIFI_DNS_LookUp(name, ipAddr) == WIFI_STATUS_OK) {
        ret = NSAPI_ERROR_OK;
        sprintf(ip, "%d.%d.%d.%d", ipAddr[0], ipAddr[1], ipAddr[2], ipAddr[3]);
    }
    return ret;
}

bool ISM43362::send(int id, const void *data, uint32_t amount)
{
    // //May take a second try if device is busy
    // for (unsigned i = 0; i < 2; i++) {
    //     if (_parser.send("AT+CIPSEND=%d,%d", id, amount)
    //         && _parser.recv(">")
    //         && _parser.write((char*)data, (int)amount) >= 0) {
    //         return true;
    //     }
    // }
    //
    // return false;

    // TODO

    // WIFI_SendData(uint8_t socket, uint8_t *pdata, uint16_t Reqlen, uint16_t *SentDatalen, uint32_t Timeout)
    printf("ISM43362::send: AAA\n");
    uint8_t status = WIFI_SendData((uint8_t)id, (uint8_t *)data, 0, (uint16_t *)&amount, timeout);
    printf("ISM43362::send: BBB status %d\n", status);
    //  == WIFI_STATUS_OK;
    return status == WIFI_STATUS_OK;
}

void ISM43362::_packet_handler()
{
    // int id;
    // uint32_t amount;
    //
    // // parse out the packet
    // if (!_parser.recv(",%d,%d:", &id, &amount)) {
    //     return;
    // }
    //
    // struct packet *packet = (struct packet*)malloc(
    //         sizeof(struct packet) + amount);
    // if (!packet) {
    //     return;
    // }
    //
    // packet->id = id;
    // packet->len = amount;
    // packet->next = 0;
    //
    // if (!(_parser.read((char*)(packet + 1), amount))) {
    //     free(packet);
    //     return;
    // }
    //
    // // append to packet list
    // *_packets_end = packet;
    // _packets_end = &packet->next;

    // TODO
}

int32_t ISM43362::recv(int id, void *data, uint32_t amount)
{
    // while (true) {
    //     // check if any packets are ready for us
    //     for (struct packet **p = &_packets; *p; p = &(*p)->next) {
    //         if ((*p)->id == id) {
    //             struct packet *q = *p;
    //
    //             if (q->len <= amount) { // Return and remove full packet
    //                 memcpy(data, q+1, q->len);
    //
    //                 if (_packets_end == &(*p)->next) {
    //                     _packets_end = p;
    //                 }
    //                 *p = (*p)->next;
    //
    //                 uint32_t len = q->len;
    //                 free(q);
    //                 return len;
    //             } else { // return only partial packet
    //                 memcpy(data, q+1, amount);
    //
    //                 q->len -= amount;
    //                 memmove(q+1, (uint8_t*)(q+1) + amount, q->len);
    //
    //                 return amount;
    //             }
    //         }
    //     }
    //
    //     // Check for inbound packets
    //     if (!_parser.process_oob()) {
    //         return -1;
    //     }
    // }

    // TODO

    // WIFI_Status_t       WIFI_ReceiveData(uint8_t socket, uint8_t *pdata, uint16_t Reqlen, uint16_t *RcvDatalen, uint32_t Timeout);
    uint16_t readLength;
    WIFI_ReceiveData((uint8_t)id, (uint8_t *)data, (uint16_t) amount, &readLength, timeout);
    printf("Socket: %d    Read size: %d    Read length: %ld    Timeout: %ld\n", id, amount, readLength, timeout);
    return readLength;
}

bool ISM43362::close(int id)
{
    // //May take a second try if device is busy
    // for (unsigned i = 0; i < 2; i++) {
    //     if (_parser.send("AT+CIPCLOSE=%d", id)
    //         && _parser.recv("OK")) {
    //         return true;
    //     }
    // }
    //
    // return false;

    // TODO

    return false;
}

void ISM43362::setTimeout(uint32_t timeout_ms)
{
    timeout = timeout_ms;
}

bool ISM43362::readable()
{
    // return _serial.FileHandle::readable();

    // TODO

    return false;
}

bool ISM43362::writeable()
{
    // return _serial.FileHandle::writable();

    // TODO

    return false;
}

void ISM43362::attach(Callback<void()> func)
{
    // _serial.sigio(func);

    // TODO
}

void ISM43362::wifi_ap2ns_api_wifi_ap(WIFI_AP_t *wifi_ap, nsapi_wifi_ap_t *ns_api_wifi_ap)
{
    strcpy(ns_api_wifi_ap->ssid, wifi_ap->SSID);
    memcpy(ns_api_wifi_ap->bssid, wifi_ap->MAC, 6);
    ns_api_wifi_ap->rssi = (int8_t) wifi_ap->RSSI;
    ns_api_wifi_ap->channel = wifi_ap->Channel;
    memcpy(ns_api_wifi_ap->bssid, wifi_ap->MAC, 6);
    ns_api_wifi_ap->security = wifi_ecn2nsapi_security(wifi_ap->Ecn);
}

nsapi_security_t ISM43362::wifi_ecn2nsapi_security(WIFI_Ecn_t wifi_ecn)
{
    switch (wifi_ecn) {
        case WIFI_ECN_OPEN:
            return NSAPI_SECURITY_NONE;
        case WIFI_ECN_WEP:
            return NSAPI_SECURITY_WEP;
        case WIFI_ECN_WPA_PSK:
            return NSAPI_SECURITY_WPA;
        case WIFI_ECN_WPA2_PSK:
            return NSAPI_SECURITY_WPA2;
        case WIFI_ECN_WPA_WPA2_PSK:
            return NSAPI_SECURITY_WPA_WPA2;
        default:
            return NSAPI_SECURITY_UNKNOWN;
    }
}

WIFI_Ecn_t ISM43362::nsapi_security2wifi_ecn(nsapi_security_t nsapi_security)
{
    switch (nsapi_security) {
        case NSAPI_SECURITY_NONE:
            return WIFI_ECN_OPEN;
        case NSAPI_SECURITY_WEP:
            return WIFI_ECN_WEP;
        case NSAPI_SECURITY_WPA:
            return WIFI_ECN_WPA_PSK;
        case NSAPI_SECURITY_WPA2:
            return WIFI_ECN_WPA2_PSK;
        case NSAPI_SECURITY_WPA_WPA2:
            return WIFI_ECN_WPA_WPA2_PSK;
        default:
            return WIFI_ECN_OPEN;
    }
}

WIFI_Protocol_t ISM43362::nsapi_protocol2WIFI_Protocol(nsapi_protocol_t nsapi_protocol)
{
    switch (nsapi_protocol) {
        case NSAPI_UDP:
            return WIFI_UDP_PROTOCOL;
        case NSAPI_TCP:
        default:
            return WIFI_TCP_PROTOCOL;
    }
}
