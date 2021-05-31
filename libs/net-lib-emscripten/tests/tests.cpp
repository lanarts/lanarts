#include <cstdio>
#include <cassert>
#include "../lanarts_net.h"
#include <vector>
#include <emscripten/emscripten.h>
#include <lcommon/Timer.h>

using namespace std;

static vector<int> NETWORK_RECEIVES;
static vector<NetConnection*> NETWORK;

static void message_received(receiver_t sender, void* context,
		const char* msg, size_t len) {
	printf("message_received %d %d %s\n", sender, (long)context, msg);
	NETWORK_RECEIVES.at((size_t)(long)context) += 1;
}
static void send_packet(NetConnection* conn) {
	printf("send_packet\n");
	conn->send_message("Hello World", sizeof "Hello World", NetConnection::ALL_RECEIVERS);
}

static void run_tick() {
	int i = 0;
	for (NetConnection* conn : NETWORK) {
		conn->poll(message_received, (void*)(long)i, -1);
		// printf("Got %d receives at %d\n", NETWORK_RECEIVES.at(i), i);
		i++;
	}
}

static int run_client_server_test(const char* hostname, bool isclient) {
}
static int run_client_server_test() {
	lanarts_net_init(true);
	NETWORK = vector<NetConnection*> {
		create_server_connection(0),
		create_client_connection("localhost", 0),
		create_client_connection("localhost", 0),
		create_client_connection("localhost", 0)
	};
	NETWORK_RECEIVES = vector<int> {0, 0, 0, 0};
	for (NetConnection* conn : NETWORK) {
		// params not used here
		conn->initialize_connection(nullptr, 0);
	}
	for (NetConnection* conn : NETWORK) {
		send_packet(conn);
	}
	// NetConnection* conn;
	// if (isclient) {
	// 	conn = create_client(hostname);
	// } else {
	// 	conn = create_server();
	// 	conn->poll(message_received, NULL, -1);
	// }
	// if (isclient) {
	// 	send_packet(conn);
	// }
	// if (!isclient) {
	// 	get_packet(conn);
	// 	send_packet(conn);
	// }
	// if (isclient) {
	// 	get_packet(conn);
	// }
	// Timer timer;
	// const int TEST_RUNS = 500;
	// for (int i = 0; i < TEST_RUNS; i++) {
	// 	send_packet(conn);
	// 	get_packet(conn);
	// }

	// printf("Average time was %.2fms\n", timer.get_microseconds()/TEST_RUNS/1000.0);
	// lanarts_net_quit();
	return 0;
}

int main(int argc, char** argv) {
	emscripten_set_main_loop(run_tick, 1, 0);
	run_client_server_test();
	return 0;
}
