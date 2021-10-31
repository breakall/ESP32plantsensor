const dgram = require('dgram');
const server = dgram.createSocket('udp4');

server.on('error', (err) => {
  console.log(`server error:\n${err.stack}`);
  server.close();
});

server.on('message', (msg, rinfo) => {
    process.stdout.write(`${msg}`);
});

server.on('listening', () => {
  const address = server.address();
  console.log(`server listening ${address.address}:${address.port}`);
});

if (process.argv.length > 2) {
  if (1 >= +process.argv[2] <= 65535 ) {
    console.log("Usage: node index.js <port to listen on>")
    process.exit(1);
  }
} else {
  server.bind(42434);
}