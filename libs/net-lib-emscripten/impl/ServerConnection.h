/*
 * ServerConnection.h:
 *  Represents a connection made by a server to multiple clients
 */

#ifndef SERVERCONNECTION_H_
#define SERVERCONNECTION_H_

#include <vector>
#include "../NetConnection.h"
#include <emscripten/val.h>

class ServerConnection: public NetConnection {
public:
	explicit ServerConnection(int port);
	virtual ~ServerConnection();

	virtual void initialize_connection(const conn_callback &callback, int timeout);

	virtual int poll(packet_recv_callback message_handler,
			void* context = NULL, int timeout = 0);
	virtual void set_accepting_connections(bool accept);
	virtual void send_message(const char* msg, int len, receiver_t receiver =
			ALL_RECEIVERS);
private:
	std::unique_ptr<emscripten::val> js_server_connection;
};

#endif
