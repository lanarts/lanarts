export function range(n) {
  let ret: number[] = [];
  for (var i = 0; i < n; i++) {
    ret.push(i);
  }
  return ret;
}

// const _BYTE_TO_HEX_TABLE = [];

// for (let n = 0; n <= 0xff; n++) {
//   const hexOctet = n.toString(16).padStart(2, "0");
//   _BYTE_TO_HEX_TABLE.push(hexOctet);
// }

// export function uint8ToHex(buff: Uint8Array) {
//   const hexOctets = new Array(buff.length);

//   for (let i = 0; i < buff.length; i++) {
//     hexOctets[i] = _BYTE_TO_HEX_TABLE[buff[i]];
//   }

//   return hexOctets.join("");
// }

export function countAll<T>(objs: T[], obj: T) {
  let sum = 0;
  for (const o of objs) {
    if (o === obj) {
      sum++;
    }
  }
  return sum;
}

export function hash(str: string) {
  let hash = 1;
  if (str.length === 0) {
    return hash;
  }
  for (let i = 0; i < str.length; i++) {
    const chr = str.charCodeAt(i);
    hash = (hash << 5) - hash + chr;
    hash |= 0; // Convert to 32bit integer
  }
  // Dont allow a hash of 0
  return hash || 1;
}

export function randomFrom<T>(arr: T[]): T {
  return arr[Math.floor(Math.random() * arr.length)];
}

export function randomN<T>(arr: T[], n: number): T[] {
  const ret: T[] = [];
  for (let i = 0; i < Math.min(n, arr.length); i++) {
    let elem = randomFrom(arr);
    while (ret.indexOf(elem) >= 0) {
      elem = randomFrom(arr);
    }
    ret.push(elem);
  }
  return ret;
}

export function has<T>(arr: T[], elem: T) {
  return arr.indexOf(elem) >= 0;
}

export function any<T>(arr: T[], func: (T) => boolean) {
  for (const elem of arr) {
    if (func(elem)) {
      return true;
    }
  }
  return false;
}

export function logAndReturn(obj: any) {
  console.log(obj);
  return obj;
}

export function hasProperty(obj: any, key: string) {
  return Object.prototype.hasOwnProperty.call(obj, key);
}

export function getProperty(obj: any, key: string) {
  return hasProperty(obj, key) ? obj[key] : undefined;
}

type RestParams<T extends (...args: any[]) => any> = T extends (
  dropped: any,
  ...args: infer P
) => any
  ? P
  : never;
type DispatcherAction<Func extends (...args: any[]) => any> = (
  ...args: RestParams<Func>
) => { type: string; args: any };

interface Reducable {
  reduce: <T>(state: T, action) => T;
}

export function makeActions<T extends { [s: string]: (...args: any[]) => any }>(
  defObj: T
): { [P in keyof T]: DispatcherAction<T[P]> } & Reducable {
  let actionTypes = Object.keys(defObj);
  let ret: any = {};
  for (let type of actionTypes) {
    ret[type] = (...args) => ({ type, args });
  }
  ret.reduce = (state, action) => {
    if (hasProperty(defObj, action.type)) {
      return defObj[action.type](state, ...action.args);
    }
    return state;
  };
  return ret;
}

export function makeDispatchActions(dispatch, actions: any) {
  let actionTypes = Object.keys(actions);
  let ret: any = {};
  for (let type of actionTypes) {
    ret[type] = (...args) => dispatch(actions[type](...args));
  }
  return ret;
}

export function assign<A, B extends A>(obj1: B, obj2: A): B {
  return Object.assign({}, obj1, obj2);
}

export function append<T>(list: T[], obj: T) {
  return [...list, obj];
}

export function removeOne<T>(list: T[], obj: T) {
  let found = false;
  const copy: T[] = [];
  for (const elem of list) {
    if (elem === obj && !found) {
      found = true;
      continue;
    }
    copy.push(elem);
  }
  return copy;
}

export function removeAll<T>(list: T[], obj: T) {
  const copy: T[] = [];
  for (const elem of list) {
    if (elem === obj) {
      continue;
    }
    copy.push(elem);
  }
  return copy;
}

export function interpolate(a, b, precision) {
  return a + (b - a) * precision;
}

export function anyTrue(list, f) {
  for (let elem of list) {
    if (f(elem)) {
      return true;
    }
  }
  return false;
}

export function readFile(fileName: string): Promise<string> {
  return new Promise(promiseHandler);
  function promiseHandler(resolve, reject) {
    const rawFile = new XMLHttpRequest();
    rawFile.open("GET", fileName, true);
    rawFile.onreadystatechange = () => {
      const { readyState, status } = rawFile;
      if (readyState === 4) {
        if (status === 200 || status === 0) {
          resolve(rawFile.responseText);
        } else {
          reject(new Error(`Could not load ${fileName}!`));
        }
      }
    };
    rawFile.send(null);
  }
}

export function nOf(n, elem) {
  const ret: any[] = [];
  for (let i = 0; i < n; i++) {
    ret.push(elem);
  }
  return ret;
}

export const timeout = (ms) => new Promise((res) => setTimeout(res, ms));

function isUnique(value, index, self) {
  return self.indexOf(value) === index;
}

export function toUnique<T>(data: T[]): T[] {
  return data.filter(isUnique);
}
