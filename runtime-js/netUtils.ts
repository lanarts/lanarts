import * as crypto from "crypto";
import { SocketSignalWebsocketClient } from "socket-signal-websocket";
import { AsyncLoop } from "./AsyncLoop";
import { timeout } from "./jsUtils";
import { bytesEqual, bytesToBase64 } from "./base64";
import * as simplePeer from "simple-peer";

const ALL_RECEIVERS = -1;
const SERVER_RECEIVER = 0;

export const RANDOM_TOPIC = crypto.randomBytes(32);

export interface NlePeer {
  peerId: Uint8Array;
}

export interface NleMessage {
  peerId: string;
  data: string;
}

export interface SignalPeer {
  id: Uint8Array;
  stream: simplePeer.Instance;
  ready(): Promise<void>;
}

export class NleConnection {
  connectedPeers: {
    [peerIdBase64: string]: SignalPeer | "connecting" | "rejected";
  } = {};
  closed: boolean = false;
  receivedMessages: NleMessage[] = [];
  constructor(public client: SocketSignalWebsocketClient) {}
  queue(peerId: Uint8Array, data: string) {
    this.receivedMessages.push({
      peerId: bytesToBase64(peerId),
      data,
    });
  }
  destroy() {
    for (const conn of Object.values(this.connectedPeers)) {
      if (conn && typeof conn !== "string") {
        conn.stream.destroy();
      }
    }
    this.connectedPeers = {};
    this.closed = true;
  }
  // Returns a connection, 'rejected', or undefined if no new connection
  // to be made
  async connect(
    peerId: Uint8Array
  ): Promise<SignalPeer | "rejected" | "already-connected"> {
    if (bytesEqual(this.client.id, peerId)) {
      return;
    }
    if (!peerId) {
      throw new Error();
    }
    const peerIdBase64 = bytesToBase64(peerId);
    const status = this.connectedPeers[peerIdBase64];
    if (status === undefined || status === "rejected") {
      // placeholder
      this.connectedPeers[peerIdBase64] = "connecting";
      const remotePeer = await this.client.connect(RANDOM_TOPIC, peerId);

      remotePeer.stream.on("signal", (data) => {
        if (typeof data === "string") {
          console.log(`CLIENT ${peerIdBase64} SAYS`, data);
        }
      });
      try {
        await remotePeer.ready();
        console.log(`Connection to ${peerIdBase64} is ready`);
        // SimplePeer connected
        this.connectedPeers[peerIdBase64] = remotePeer;
        return remotePeer;
      } catch (err) {
        // SimplePeer rejected
        this.connectedPeers[peerIdBase64] = "rejected";
        return "rejected";
      }
    }
    return "already-connected";
  }
}

// The server simply sends continuously to all peers
// Network/game topology is first broadcast at a 10fps rate
// and then throttled as we enter a real game
export interface TopologyPayload {
  peers: Array<{ netId: number; peerId: string }>;
  handshakeIsOver: boolean;
}

// async function handlerThread(connection: NleConnection) {
//   const { client } = connection;
//   await client.open();

// connection.availablePeerIds = [];
// // join the swarm for the given topic and get the current peers for that topic
// for (const peerId of await client.join(topic)) {
//   connection.availablePeerIds.push(peerId);
//   considerPeer(peerId);
// }
//   }
//   // peerQueryThread(connection);

//   // // Helpers
//   // async function peerQueryThread(connection: NleConnection) {
//   //   while (connection.isOpen) {
//   //     const peers = await connection.client.lookup(topic);
//   //     connection.availablePeerIds = [];
//   //     for (const peer of peers) {
//   //       considerPeer(peer);
//   //     }
//   //     // Update our peer list every 5 seconds
//   //     await timeout(5000);
//   //   }
//   // }
// }
export async function createSignalConnection(
  isServer: boolean
): Promise<SocketSignalWebsocketClient> {
  const id = crypto.randomBytes(32);
  id[31] = isServer ? 1 : 0;
  const client = new SocketSignalWebsocketClient(
    // ["wss://geut-webrtc-signal-v3.glitch.me"],
    ["ws://localhost:4000"],
    {
      id,
      heartbeat: {
        interval: 10 * 1000,
        timeout: 5 * 1000,
      },
      simpleWebsocket: {}, // https://github.com/feross/simple-websocket options
      reconnectingWebsocket: {}, // https://github.com/pladaria/reconnecting-websocket options
      simplePeer: {}, // https://github.com/feross/simple-peer options
    }
  );
  await client.open();
  return client;
}

export async function findServerPeerTryN(
  connection: SocketSignalWebsocketClient,
  peerIds: number[]
): Promise<SignalPeer> {
  for (let i = 0; i < 100; i++) {
    const conn = await findServerPeer(connection, peerIds);
    if (!conn) {
      await timeout(1000);
      peerIds = await connection.client.lookup(RANDOM_TOPIC);
      continue;
    }
    return conn;
  }
  throw new Error("Could not find server peer!");
}

export async function findServerPeer(
  connection: SocketSignalWebsocketClient,
  peerIds: number[]
): Promise<SignalPeer | undefined> {
  for (const peerId of peerIds) {
    console.log({ peerId });
    if (peerId[31] === 1) {
      // Found server
      return await connection.connect(RANDOM_TOPIC, peerId);
    }
  }
  return undefined;
}
