import tls from "tls";
import net from "net";
import fs from "fs";
import { LoginSuccess } from "../protobuf/gen/login";

export class TcpClient {
  private socket?: tls.TLSSocket;

  connect(host: string, port: number) {
    this.socket = tls.connect(
      {
        host: host,
        port: port,
        ca: fs.readFileSync("config/server.crt"),
        rejectUnauthorized: false
      },
      () => {
        console.log("Connect to %s", host);
      }
    );

    this.socket.on("data", this.onPackage);
  }

  send(buf: Buffer) {
    this.socket?.write(buf);
  }

  close() {
    this.socket?.end();
  }

  sendPackage(id: bigint, payload: Buffer | Uint8Array): void {
    const length = BigInt(payload.length);

    const buf = Buffer.allocUnsafe(16 + payload.length);
    let offset = 0;

    buf.writeBigInt64BE(id, offset);
    offset += 8;

    buf.writeBigInt64BE(length, offset);
    offset += 8;

    if (payload instanceof Buffer) {
      payload.copy(buf, offset);
    } else if (payload instanceof Uint8Array) {
      buf.set(payload, offset);
    }

    this.socket?.write(buf);
  }

  onPackage(data: Buffer) {
    if (data.length < 16) return;

    let offset = 0;

    const id = data.readBigInt64BE(offset);
    offset += 8;

    const length = data.readBigInt64BE(offset);
    offset += 8;

    const payload = data.subarray(offset, offset + Number(length));
    console.log("Receive: id[%d], payload: %s", id, payload.toString());

    // TODO: handle payload
    if (Number(id) === 1002) {
      const res: LoginSuccess = LoginSuccess.fromBinary(payload);
      console.log("Login Success: pid =%d", res.playerId);
    }
  }
}
