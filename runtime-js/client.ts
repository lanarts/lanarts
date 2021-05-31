import { bytesEqual, bytesToBase64 } from "./base64";
import { timeout } from "./jsUtils";
import {
  createSignalConnection,
  findServerPeerTryN,
  NleConnection,
  RANDOM_TOPIC,
  SignalPeer,
  TopologyPayload,
} from "./netUtils";

export async function createClientConnection() {
  const client = await createSignalConnection(false);
  const peerIds = await client.join(RANDOM_TOPIC);
  const nleConnection = new NleConnection(client);
  const serverPeer = await findServerPeerTryN(client, peerIds);
  let topology: TopologyPayload = {
    peers: [],
    handshakeIsOver: false,
  };

  client.onIncomingPeer(async (peer: SignalPeer) => {
    peer.stream.on("data", (data) => {
      if (data[0] === 123) {
        // Are we a server, getting this JSON?
        if (bytesEqual(peer.id, serverPeer.id)) {
          // Detected JSON data starting with char '{'
          const handshakeData = JSON.parse(new TextDecoder().decode(data));
          if (handshakeData.topology) {
            topology = handshakeData.topology;
          }
        }
      } else {
        nleConnection.queue(peer.id, data);
      }
    });
  });
  try {
    await serverPeer.ready();
    console.log(
      "Client connection to " + bytesToBase64(serverPeer.id) + " is ready"
    );
  } catch (err) {
    throw new Error("Connection to server rejected! " + err);
  }

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
    getMessages() {
      const msgs = nleConnection.receivedMessages;
      nleConnection.receivedMessages = [];
      return msgs.map((msg) => msg.data);
    },
  };
}
