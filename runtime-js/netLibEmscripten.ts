const ALL_RECEIVERS = -1;
const SERVER_RECEIVER = 0;

let __NETWORK = [];

function createServerConnection() {
  let acceptingConnections = true;
  console.log("netLibEmscripten.createServerConnection()");
  const self = {
    peerId: 0,
    messages: [],
    destructor() {
      __NETWORK.splice(__NETWORK.indexOf(this), 1);
    },
    sendMessage(receiverPeerId: number, payload: string) {
      if (receiverPeerId == -1) {
        for (const peerConn of __NETWORK) {
          if (peerConn !== this) {
            peerConn.messages.push({
              peer: this.peerId,
              payload,
            });
          }
        }
      } else {
        __NETWORK[receiverPeerId].messages.push({
          peer: this.peerId,
          payload,
        });
      }
    },
    setAcceptingConnections(accepting: boolean) {
      acceptingConnections = accepting;
    },
    getMessages() {
      const msgs = this.messages;
      this.messages = [];
      return msgs;
    },
  };
  __NETWORK = [self];
  return self;
}

function createClientConnection(address: string) {
  console.log(`netLibEmscripten.createClientConnection(${address})`);
  const self = {
    peerId: __NETWORK.length,
    messages: [],
    destructor() {
      __NETWORK.splice(__NETWORK.indexOf(this), 1);
    },
    sendMessage(receiverPeerId: number, payload: string) {
      if (receiverPeerId == -1) {
        for (const peerConn of __NETWORK) {
          if (peerConn !== this) {
            peerConn.messages.push({
              peer: this.peerId,
              payload,
            });
          }
        }
      } else {
        __NETWORK[receiverPeerId].messages.push({
          peer: this.peerId,
          payload,
        });
      }
    },
    getMessages() {
      const msgs = this.messages;
      this.messages = [];
      return msgs;
    },
  };
  __NETWORK.push(self);
  return self;
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
