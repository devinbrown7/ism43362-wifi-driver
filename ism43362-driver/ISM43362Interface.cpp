/* ISM43362 implementation of NetworkInterfaceAPI
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

#include <string.h>
#include "ISM43362Interface.h"
#include "mbed_debug.h"

// Various timeouts for different ISM43362 operations
#ifndef ISM43362_CONNECT_TIMEOUT
#define ISM43362_CONNECT_TIMEOUT 15000
#endif
#ifndef ISM43362_SEND_TIMEOUT
#define ISM43362_SEND_TIMEOUT    1000
#endif
#ifndef ISM43362_RECV_TIMEOUT
#define ISM43362_RECV_TIMEOUT    1000
#endif
#ifndef ISM43362_MISC_TIMEOUT
#define ISM43362_MISC_TIMEOUT    1000
#endif

// Firmware version
#define ISM43362_VERSION 2

// ISM43362Interface implementation
ISM43362Interface::ISM43362Interface()
    : _ism()
{
    memset(_ids, 0, sizeof(_ids));
    memset(_cbs, 0, sizeof(_cbs));

    _ism.attach(this, &ISM43362Interface::event);
}

int ISM43362Interface::connect(const char *ssid, const char *pass, nsapi_security_t security,
                                        uint8_t channel)
{
    if (channel != 0) {
        return NSAPI_ERROR_UNSUPPORTED;
    }

    set_credentials(ssid, pass, security);
    return connect();
}

int ISM43362Interface::connect()
{
    _ism.setTimeout(ISM43362_CONNECT_TIMEOUT);

    if (!_ism.dhcp(true)) {
        return NSAPI_ERROR_DHCP_FAILURE;
    }
    if (!_ism.connect(ap_ssid, ap_pass, ap_sec)) {
        return NSAPI_ERROR_NO_CONNECTION;
    }
    if (!_ism.getIPAddress()) {
        return NSAPI_ERROR_DHCP_FAILURE;
    }
    return NSAPI_ERROR_OK;
}

int ISM43362Interface::set_credentials(const char *ssid, const char *pass, nsapi_security_t security)
{
    memset(ap_ssid, 0, sizeof(ap_ssid));
    strncpy(ap_ssid, ssid, sizeof(ap_ssid));

    memset(ap_pass, 0, sizeof(ap_pass));
    strncpy(ap_pass, pass, sizeof(ap_pass));

    ap_sec = security;

    return 0;
}

int ISM43362Interface::set_channel(uint8_t channel)
{
    return NSAPI_ERROR_UNSUPPORTED;
}


int ISM43362Interface::disconnect()
{
    _ism.setTimeout(ISM43362_MISC_TIMEOUT);

    if (!_ism.disconnect()) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    return NSAPI_ERROR_OK;
}

const char *ISM43362Interface::get_ip_address()
{
    return _ism.getIPAddress();
}

const char *ISM43362Interface::get_mac_address()
{
    return _ism.getMACAddress();
}

const char *ISM43362Interface::get_gateway()
{
    return _ism.getGateway();
}

const char *ISM43362Interface::get_netmask()
{
    return _ism.getNetmask();
}

int8_t ISM43362Interface::get_rssi()
{
    return _ism.getRSSI();
}

int ISM43362Interface::scan(WiFiAccessPoint *res, unsigned count)
{
    return _ism.scan(res, count);
}

int ISM43362Interface::gethostbyname(const char *host, SocketAddress *address, nsapi_version_t version)
{
    // bool dns_lookup(const char *name, char *ip);
    char addr[16];
    int ret = _ism.dns_lookup(host, addr);
    int status = address->set_ip_address(addr);
    return ret;
}

struct ism43362_socket {
    int id;
    nsapi_protocol_t proto;
    bool connected;
    SocketAddress addr;
};

int ISM43362Interface::socket_open(void **handle, nsapi_protocol_t proto)
{
    // Look for an unused socket
    int id = -1;

    for (int i = 0; i < ISM43362_SOCKET_COUNT; i++) {
        if (!_ids[i]) {
            id = i;
            _ids[i] = true;
            break;
        }
    }

    if (id == -1) {
        return NSAPI_ERROR_NO_SOCKET;
    }

    struct ism43362_socket *socket = new struct ism43362_socket;
    if (!socket) {
        return NSAPI_ERROR_NO_SOCKET;
    }

    socket->id = id;
    socket->proto = proto;
    socket->connected = false;
    *handle = socket;
    return 0;
}

int ISM43362Interface::socket_close(void *handle)
{
    struct ism43362_socket *socket = (struct ism43362_socket *)handle;
    int err = 0;
    _ism.setTimeout(ISM43362_MISC_TIMEOUT);

    if (socket->connected && !_ism.close(socket->id)) {
        err = NSAPI_ERROR_DEVICE_ERROR;
    }

    socket->connected = false;
    _ids[socket->id] = false;
    delete socket;
    return err;
}

int ISM43362Interface::socket_bind(void *handle, const SocketAddress &address)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

// Listen for connections on a TCP socket.
int ISM43362Interface::socket_listen(void *handle, int backlog)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

// Connects TCP socket to a remote host.
int ISM43362Interface::socket_connect(void *handle, const SocketAddress &addr)
{
    struct ism43362_socket *socket = (struct ism43362_socket *)handle;
    _ism.setTimeout(ISM43362_MISC_TIMEOUT);

    if (!_ism.open(socket->proto, socket->id, addr.get_ip_address(), addr.get_port())) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }
    socket->connected = true;
    return 0;
}

// Accepts a connection on a TCP socket.
int ISM43362Interface::socket_accept(void *server, void **socket, SocketAddress *addr)
{
    return NSAPI_ERROR_UNSUPPORTED;
}

// Send data over a TCP socket.
int ISM43362Interface::socket_send(void *handle, const void *data, unsigned size)
{
    struct ism43362_socket *socket = (struct ism43362_socket *)handle;
    _ism.setTimeout(ISM43362_SEND_TIMEOUT);

    if (!_ism.send(socket->id, data, size)) {
        return NSAPI_ERROR_DEVICE_ERROR;
    }

    return size;
}

// Receive data over a TCP socket.
int ISM43362Interface::socket_recv(void *handle, void *data, unsigned size)
{
    struct ism43362_socket *socket = (struct ism43362_socket *)handle;
    _ism.setTimeout(ISM43362_RECV_TIMEOUT);
    int32_t recv = _ism.recv(socket->id, data, size);
    if (recv < 0) {
        return NSAPI_ERROR_WOULD_BLOCK;
    }
    return recv;
}

// Send a packet over a UDP socket.
int ISM43362Interface::socket_sendto(void *handle, const SocketAddress &addr, const void *data, unsigned size)
{
    struct ism43362_socket *socket = (struct ism43362_socket *)handle;

    if (socket->connected && socket->addr != addr) {
        _ism.setTimeout(ISM43362_MISC_TIMEOUT);
        if (!_ism.close(socket->id)) {
            return NSAPI_ERROR_DEVICE_ERROR;
        }
        socket->connected = false;
    }

    if (!socket->connected) {
        int err = socket_connect(socket, addr);
        if (err < 0) {
            return err;
        }
        socket->addr = addr;
    }

    return socket_send(socket, data, size);
}

// Receive a packet over a UDP socket.
int ISM43362Interface::socket_recvfrom(void *handle, SocketAddress *addr, void *data, unsigned size)
{
    struct ism43362_socket *socket = (struct ism43362_socket *)handle;
    int ret = socket_recv(socket, data, size);
    if (ret >= 0 && addr) {
        *addr = socket->addr;
    }
    return ret;
}

// Register a callback on state change of the socket.
void ISM43362Interface::socket_attach(void *handle, void (*callback)(void *), void *data)
{
    struct ism43362_socket *socket = (struct ism43362_socket *)handle;
    _cbs[socket->id].callback = callback;
    _cbs[socket->id].data = data;
}

void ISM43362Interface::event() {
    for (int i = 0; i < ISM43362_SOCKET_COUNT; i++) {
        if (_cbs[i].callback) {
            _cbs[i].callback(_cbs[i].data);
        }
    }
}
