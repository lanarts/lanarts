#include <memory>

/*
 * ClientConnection.h:
 *  Represents a client connected to a server
 */

#include <functional>
#include "ClientConnection.h"

#include <emscripten/bind.h>

#include "../lanarts_net.h"

using namespace std;
using namespace emscripten;

ClientConnection::ClientConnection(const char* addr, int port) {
    address = addr;
}

ClientConnection::~ClientConnection() {
    if (js_client_connection) {
        js_client_connection->call<void>("destructor");
    }
}

/*!
 * Try to initialize connection
 * @param callback on every connection timeout
 * @param timeout in ms
 */
void ClientConnection::initialize_connection(const conn_callback &callback, int timeout) {
	// both parameters are ignored
	js_client_connection = make_unique<val>(
        val::global("NetLibEmscripten").call<val>("createClientConnection", address)
    );
}

int ClientConnection::poll(packet_recv_callback message_handler, void* context,
		int timeout) {
    // timeout is ignored
    if (!js_client_connection) {
        __lnet_throw_connection_error(
                "ClientConnection::poll: Connection not initialized!\n");
    }
	auto messages = js_client_connection->call<val>("getMessages");
	size_t messages_length = messages["length"].as<size_t>();
	for (int i = 0; i < messages_length; i++) {
		auto message = messages[i];
        auto payload = message["payload"].as<string>();
        (*message_handler)(
            message["peer"].as<int>(),
            context,
            payload.c_str(),
            payload.size()
        );
    }
	return messages_length > 0;
}

void ClientConnection::send_message(const char* msg, int len,
		receiver_t receiver) {
    if (!js_client_connection) {
        __lnet_throw_connection_error(
                "ServerConnection::send_message: Connection not initialized!\n");
    }
    js_client_connection->call<void>("sendMessage", (int)receiver, string(msg, len));
}
