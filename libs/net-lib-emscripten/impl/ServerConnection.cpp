/*
 * ServerConnection.h:
 *  Represents a connection made by a server to multiple clients
 */

#include <string>
#include <vector>

#include <functional>

#include "../lanarts_net.h"

#include "ServerConnection.h"

using namespace std;
using namespace emscripten;

ServerConnection::ServerConnection(int port) {
}

ServerConnection::~ServerConnection() {
	if (js_server_connection) {
		js_server_connection->call<void>("destructor");
	}
}

void ServerConnection::initialize_connection(const conn_callback &callback, int timeout) {
	// parameters ignored
	js_server_connection = make_unique<val>(
		val::global("NetLibEmscripten").call<val>("createServerConnection")
	);
}

int ServerConnection::poll(packet_recv_callback message_handler, void* context,
		int timeout) {
	// timeout is ignored
	if (!js_server_connection) {
		__lnet_throw_connection_error(
				"ServerConnection::send_message: Connection not initialized!\n");
	}
	auto messages = js_server_connection->call<val>("getMessages");
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

void ServerConnection::set_accepting_connections(bool accept) {
	if (!js_server_connection) {
		__lnet_throw_connection_error(
				"ServerConnection::send_message: Connection not initialized!\n");
	}
	js_server_connection->call<void>("setAcceptingConnections", accept);
}

void ServerConnection::send_message(
	const char* msg,
	int len,
	receiver_t receiver
) {
	if (!js_server_connection) {
		__lnet_throw_connection_error(
				"ServerConnection::send_message: Connection not initialized!\n");
	}
	js_server_connection->call<void>("sendMessage", (int)receiver, string(msg, len));
}
