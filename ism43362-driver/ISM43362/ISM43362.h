/* ISM43362Interface Example
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

#ifndef ISM43362_H
#define ISM43362_H

#include "mbed.h"
#include "wifi.h"
#include <cstdarg>
#include "Callback.h"
#include <stdint.h>
#include "nsapi_types.h"

#include "WiFiAccessPoint.h"

/** ISM43362Interface class.
    This is an interface to a ISM43362 radio.
 */
class ISM43362
{
public:
    ISM43362();

    /**
    * Check firmware version of ISM43362
    *
    * @return integer firmware version or -1 if firmware query command gives outdated response
    */
    int get_firmware_version(void);

    /**
    * Startup the ISM43362
    *
    * @param mode mode of WIFI 1-client, 2-host, 3-both
    * @return true only if ISM43362 was setup correctly
    */
    bool startup(int mode);

    /**
    * Reset ISM43362
    *
    * @return true only if ISM43362 resets successfully
    */
    bool reset(void);

    /**
    * Enable/Disable DHCP
    *
    * @param enabled DHCP enabled when true
    * @return true only if ISM43362 enables/disables DHCP successfully
    */
    bool dhcp(bool enabled);

    /**
    * Connect ISM43362 to AP
    *
    * @param ssid the name of the AP
    * @param passPhrase the password of AP
    * @param security the security of AP
    * @return true only if ISM43362 is connected successfully
    */
    bool connect(const char *ssid, const char *passPhrase, nsapi_security_t security);

    /**
    * Disconnect ISM43362 from AP
    *
    * @return true only if ISM43362 is disconnected successfully
    */
    bool disconnect(void);

    /**
    * Get the IP address of ISM43362
    *
    * @return null-teriminated IP address or null if no IP address is assigned
    */
    const char *getIPAddress(void);

    /**
    * Get the MAC address of ISM43362
    *
    * @return null-terminated MAC address or null if no MAC address is assigned
    */
    const char *getMACAddress(void);

     /** Get the local gateway
     *
     *  @return         Null-terminated representation of the local gateway
     *                  or null if no network mask has been recieved
     */
    const char *getGateway();

    /** Get the local network mask
     *
     *  @return         Null-terminated representation of the local network mask
     *                  or null if no network mask has been recieved
     */
    const char *getNetmask();

    /* Return RSSI for active connection
     *
     * @return      Measured RSSI
     */
    int8_t getRSSI();

    /**
    * Check if ISM43362 is conenected
    *
    * @return true only if the chip has an IP address
    */
    bool isConnected(void);

    /** Scan for available networks
     *
     * @param  ap    Pointer to allocated array to store discovered AP
     * @param  limit Size of allocated @a res array, or 0 to only count available AP
     * @return       Number of entries in @a res, or if @a count was 0 number of available networks, negative on error
     *               see @a nsapi_error
     */
    int scan(WiFiAccessPoint *res, unsigned limit);

    /**Perform a dns query
    *
    * @param name Hostname to resolve
    * @param ip   Buffer to store IP address
    * @return       Number of entries in @a res, or if @a count was 0 number of available networks, negative on error
    *               see @a nsapi_error
    */
    int dns_lookup(const char *name, char *ip);

    /**
    * Open a socketed connection
    *
    * @param type the type of socket to open "UDP" or "TCP"
    * @param id id to give the new socket, valid 0-4
    * @param port port to open connection with
    * @param addr the IP address of the destination
    * @return true only if socket opened successfully
    */
    bool open(nsapi_protocol_t type, int id, const char* addr, int port);

    /**
    * Sends data to an open socket
    *
    * @param id id of socket to send to
    * @param data data to be sent
    * @param amount amount of data to be sent - max 1024
    * @return true only if data sent successfully
    */
    bool send(int id, const void *data, uint32_t amount);

    /**
    * Receives data from an open socket
    *
    * @param id id to receive from
    * @param data placeholder for returned information
    * @param amount number of bytes to be received
    * @return the number of bytes received
    */
    int32_t recv(int id, void *data, uint32_t amount);

    /**
    * Closes a socket
    *
    * @param id id of socket to close, valid only 0-4
    * @return true only if socket is closed successfully
    */
    bool close(int id);

    /**
    * Allows timeout to be changed between commands
    *
    * @param timeout_ms timeout of the connection
    */
    void setTimeout(uint32_t timeout_ms);

    /**
    * Checks if data is available
    */
    bool readable();

    /**
    * Checks if data can be written
    */
    bool writeable();

    /**
    * Attach a function to call whenever network state has changed
    *
    * @param func A pointer to a void function, or 0 to set as none
    */
    void attach(Callback<void()> func);

    /**
    * Attach a function to call whenever network state has changed
    *
    * @param obj pointer to the object to call the member function on
    * @param method pointer to the member function to call
    */
    template <typename T, typename M>
    void attach(T *obj, M method) {
        attach(Callback<void()>(obj, method));
    }

private:
    struct packet {
        struct packet *next;
        int id;
        uint32_t len;
        // data follows
    } *_packets, **_packets_end;
    void _packet_handler();
    void wifi_ap2ns_api_wifi_ap(WIFI_AP_t *wifi_ap, nsapi_wifi_ap_t *ns_api_wifi_ap);
    nsapi_security_t wifi_ecn2nsapi_security(WIFI_Ecn_t wifi_ecn);
    WIFI_Ecn_t nsapi_security2wifi_ecn(nsapi_security_t nsapi_security);
    WIFI_Protocol_t nsapi_protocol2WIFI_Protocol(nsapi_protocol_t nsapi_protocol);

    char _ip_buffer[16];
    char _gateway_buffer[16];
    char _netmask_buffer[16];
    char _mac_buffer[18];
    uint32_t timeout;
};

#endif
