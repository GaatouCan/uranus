import { TcpClient } from "../core/client";
import { LoginRequest } from "../protobuf/gen/login";

const client = new TcpClient();
client.connect("127.0.0.1", 8090);

// Login Request
{
  const req: LoginRequest = {
    playerId: BigInt(1001),
    token: "Hello"
  };
  const bytes = LoginRequest.toBinary(req);
  client.sendPackage(BigInt(1001), bytes);
}

setTimeout(() => {
  client.close();
}, 2000);
