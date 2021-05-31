import { base64ToBytes, bytesToBase64 } from "./base64";
import { timeout } from "./jsUtils";
import {
  createSignalConnection,
  NleConnection,
  RANDOM_TOPIC,
  SignalPeer,
  TopologyPayload,
} from "./netUtils";

export async function createServerConnection() {
  const server = await createSignalConnection(true);
  const peerIds = await server.join(RANDOM_TOPIC);
  const nleConnection = new NleConnection(server);
  const topology: TopologyPayload = {
    peers: [{ netId: 0, peerId: bytesToBase64(server.id) }],
    handshakeIsOver: false,
  };

  // Wait for clients to join us
  server.onIncomingPeer(async (peer: SignalPeer) => {
    peer.stream.on("data", (data) => {
      nleConnection.queue(peer.id, data);
    });
  });
  // Start handshake phase
  handshakeThread();
  return {
    destructor() {
      nleConnection.destroy();
    },
    sendMessage(netId: number, payload: string) {
      if (netId == -1) {
        for (const peer of topology.peers) {
          this.sendMessage(peer.netId, payload);
        }
        return;
      }
      const receiver = topology.peers[netId];
      if (!receiver) {
        console.warn(`No peer found with for netID=${netId}!`);
        return;
      }
      const peer = nleConnection.connectedPeers[receiver.peerId];
      if (!receiver || typeof peer === "string") {
        console.warn(
          `No active connection found for peer ${receiver.peerId} with for netID=${netId}!`
        );
        return;
      }
      peer.stream.send(payload);
    },
    setAcceptingConnections(accepting: boolean) {
      topology.handshakeIsOver = accepting;
    },
    getMessages() {
      const msgs = nleConnection.receivedMessages;
      nleConnection.receivedMessages = [];
      return msgs.map((msg) => msg.data);
    },
  };
  // Helpers
  async function connect(peerId: Uint8Array) {
    if (nleConnection.closed) {
      return;
    }
    const peer = await nleConnection.connect(peerId);
    if (!peer || typeof peer === "string") {
      return;
    }
    peer.stream.on("data", (data) => {
      nleConnection.queue(peer.id, data);
    });
    topology.peers.push({
      netId: topology.peers.length,
      peerId: bytesToBase64(peerId),
    });
  }
  async function handshakeThread() {
    // Try to join all clients??
    for (const peerId of peerIds) {
      connect(peerId);
    }

    while (!nleConnection.closed && !topology.handshakeIsOver) {
      const peerIds = await server.lookup(RANDOM_TOPIC);
      for (const peerId of peerIds) {
        await connect(peerId);
      }
      for (const conn of Object.values(nleConnection.connectedPeers)) {
        if (typeof conn === "string") {
          continue;
        }
        conn.stream.send(JSON.stringify({ topology }));
      }
      await timeout(100);
    }
  }
}
