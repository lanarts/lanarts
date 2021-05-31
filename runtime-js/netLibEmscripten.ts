import * as client from "./client";
import * as server from "./server";

function createServerConnection() {
  console.log("netLibEmscripten.createServerConnection()");

  let connection,
    taskQueue: Array<() => void> = [];
  function withConnection(f) {
    if (connection) {
      f();
    } else {
      taskQueue.push(f);
    }
  }
  (async function () {
    connection = await client.createClientConnection();
    for (const task of taskQueue) {
      task();
    }
    taskQueue = [];
  })();
  return {
    destructor() {
      withConnection(() => connection.destructor());
    },
    sendMessage(receiverPeerId: number, payload: string) {
      withConnection(() => connection.sendMessage(receiverPeerId, payload));
    },
    setAcceptingConnections(isAccepting: boolean) {
      withConnection(() => connection.setAcceptingConnections(isAccepting));
    },
    getMessages() {
      if (!connection) {
        return [];
      } else {
        return connection.getMessages();
      }
    },
  };
  // const self = {
  //   peerId: 0,
  //   nleConnection: server.createServerConnection(),
  //   messages: [],
  //   destructor() {
  //     __NETWORK.splice(__NETWORK.indexOf(this), 1);
  //   },
  //   sendMessage(receiverPeerId: number, payload: string) {
  //     if (receiverPeerId == -1) {
  //       for (const peerConn of __NETWORK) {
  //         if (peerConn !== this) {
  //           peerConn.messages.push({
  //             peer: this.peerId,
  //             payload,
  //           });
  //         }
  //       }
  //     } else {
  //       __NETWORK[receiverPeerId].messages.push({
  //         peer: this.peerId,
  //         payload,
  //       });
  //     }
  //   },
  //   setAcceptingConnections(accepting: boolean) {
  //     acceptingConnections = accepting;
  //   },
  //   getMessages() {
  //     const msgs = this.messages;
  //     this.messages = [];
  //     return msgs;
  //   },
  // };
  // __NETWORK = [self];
  // return self;
}

function createClientConnection(address: string) {
  console.log(`netLibEmscripten.createClientConnection(${address})`);
  let connection,
    taskQueue: Array<() => void> = [];
  function withConnection(f) {
    if (connection) {
      f();
    } else {
      taskQueue.push(f);
    }
  }
  (async function () {
    connection = await client.createClientConnection();
    for (const task of taskQueue) {
      task();
    }
    taskQueue = [];
  })();
  return {
    destructor() {
      withConnection(() => connection.destructor());
    },
    sendMessage(receiverPeerId: number, payload: string) {
      withConnection(() => connection.sendMessage(receiverPeerId, payload));
    },
    getMessages() {
      if (!connection) {
        return [];
      } else {
        return connection.getMessages();
      }
    },
  };
}

function initialize() {
  console.log("netLibEmscripten: initialize");
  return true;
}
function deinitialize() {
  console.log("netLibEmscripten: deinitialize");
}

(window as any).NetLibEmscripten = {
  initialize,
  deinitialize,
  createClientConnection,
  createServerConnection,
};
