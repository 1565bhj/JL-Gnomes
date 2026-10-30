const dgram = require('dgram');
const net = require('net');
const http = require('http');
const fs = require('fs');
const path = require('path');
const { WebSocketServer } = require('ws');

const UDP_PORT = 5003;
const TCP_PORT = 5002;
const HTTP_PORT = 8080;
const WS_PORT = 8081;

let deviceIP = null;
let deviceMac = null;
let micChannels = 2;
let devicePort = 5003;
let tcpSocket = null;
let lastRecvTime = 0;
let currentTestMode = '';

let tcpDataBuffer = Buffer.alloc(0);

// ---------------- HTTP ----------------
const MIME = { '.html':'text/html; charset=utf-8', '.js':'application/javascript', '.css':'text/css', '.json':'application/json' };
http.createServer((req, res) => {
  let p = req.url.split('?')[0];
  if (p === '/') p = '/index.html';
  fs.readFile(path.join(__dirname, 'public', p), (err, data) => {
    if (err) { res.writeHead(404); res.end('Not found'); return; }
    res.writeHead(200, { 'Content-Type': MIME[path.extname(p).toLowerCase()] || 'application/octet-stream' });
    res.end(data);
  });
}).listen(HTTP_PORT, () => console.log(`HTTP: http://localhost:${HTTP_PORT}`));

// ---------------- WebSocket ----------------
const wss = new WebSocketServer({ port: WS_PORT });
console.log(`WS: ws://localhost:${WS_PORT}`);
const broadcast = (o) => { const s = JSON.stringify(o); wss.clients.forEach(c => c.readyState === 1 && c.send(s)); };
const broadcastBinary = (b) => { wss.clients.forEach(c => c.readyState === 1 && c.send(b)); };

// ---------------- UDP 接收 Socket ----------------
let udpListening = false;
const udpRecvSock = dgram.createSocket({ type: 'udp4', reuseAddr: true });

udpRecvSock.on('error', (e) => {
  console.error('UDP 接收错误:', e);
  if (e.code === 'EACCES') {
    console.error('⚠️ 端口 5003 被占用，请关闭其他占用该端口的程序！！！');
    process.exit(1);
  }
});

udpRecvSock.on('listening', () => {
  udpListening = true;
  try {
    udpRecvSock.setBroadcast(true);
    broadcast({ type: 'udpStatus', status: 'listening', address: '0.0.0.0', port: UDP_PORT });
    console.log(`✅ UDP 接收 Socket 监听成功: 0.0.0.0:${UDP_PORT}`);
  } catch (e) {
    console.error('setBroadcast failed:', e.message);
  }
});

udpRecvSock.bind({ port: UDP_PORT, address: '0.0.0.0', exclusive: false });

const ALLOWED_IPS = (process.env.ALLOWED_IPS || '').split(',').map(s => s.trim()).filter(Boolean);
console.log('UDP IP 过滤白名单:', ALLOWED_IPS.length > 0 ? ALLOWED_IPS.join(', ') : '（未设置，接收所有）');

udpRecvSock.on('message', (msg, rinfo) => {
  let text = msg.toString('utf8').replace(/\0/g, '').trim(); 

  if (ALLOWED_IPS.length > 0 && !ALLOWED_IPS.includes(rinfo.address)) return;

  console.log(`[UDP 接收] ${rinfo.address}:${rinfo.port} "${text}"`);
  broadcast({ type: 'udpStatus', status: 'received', from: rinfo.address, raw: text });

  if (!text.includes('wifi_pcm_connet')) return;

  deviceIP = rinfo.address;
  devicePort = rinfo.port; 

  const chMatch = text.match(/-ch:(\d+)/);
  micChannels = chMatch ? (parseInt(chMatch[1], 10) === 1 ? 1 : 2) : 2;

  let mac = null;
  const macMatch = text.match(/mac:([0-9A-Fa-f]{12})/);
  if (macMatch) mac = macMatch[1].toUpperCase();
  else {
    const all = text.match(/([0-9A-Fa-f]{12})/g);
    if (all && all.length) mac = all[all.length - 1].toUpperCase();
  }
  deviceMac = mac;

  let vol = 0;
  const volMatch = text.match(/vol:(\d+)/);
  if (volMatch) vol = parseInt(volMatch[1], 10);

  let productName = '';
  const parts = text.split('-');
  for (let i = parts.length - 1; i >= 0; i--) {
    const p = parts[i];
    if (p.startsWith('vol:') || p.startsWith('ch:') || p.startsWith('mac:') || p.startsWith('0x') || p.includes('wifi_pcm')) continue;
    productName = p.replace(/\0/g, '').trim();
    break;
  }

  broadcast({ 
    type: 'device', 
    ip: deviceIP, 
    port: devicePort, 
    mac: deviceMac, 
    micChannels: micChannels,
    vol: vol,
    productName: productName,
    raw: text
  });
});

// ---------------- UDP 发送函数 ----------------
function sendUDP(payload, targetIp, targetPort) {
  const destIp = targetIp || deviceIP;
  const destPort = targetPort || devicePort;
  if (!destIp || !payload) return;
  
  const buf = Buffer.from(payload);
  
  udpRecvSock.send(buf, 0, buf.length, destPort, destIp, (err) => {
    if (err) console.error('❌ UDP 发送失败:', err.message);
    else console.log(`✅ [UDP 发送 ->] ${destIp}:${destPort} "${payload}"`);
  });
}

// ---------------- TCP 连接 ----------------
function connectTCP(targetIp) {
  if (!targetIp) return;
  
  if (tcpSocket) { try { tcpSocket.destroy(); } catch (e) {} }
  
  tcpDataBuffer = Buffer.alloc(0);

  console.log(`[尝试连接 TCP] ${targetIp}:${TCP_PORT} ...`);
  
  tcpSocket = net.createConnection({ host: targetIp, port: TCP_PORT }, () => {
    console.log(`✅ TCP connected ${targetIp}:${TCP_PORT}`);
    lastRecvTime = Date.now();
    broadcast({ type: 'tcp', status: 'connected', ip: targetIp }); 
  });
  
  tcpSocket.setTimeout(2000); 
  tcpSocket.on('timeout', () => {
    console.error(`❌ TCP 连接超时: ${targetIp}`);
    broadcast({ type: 'tcp', status: 'error', msg: '连接超时', ip: targetIp });
    tcpSocket.destroy();
    tcpSocket = null;
  });
  
  tcpSocket.on('data', (d) => { 
    lastRecvTime = Date.now(); 
    
    tcpDataBuffer = Buffer.concat([tcpDataBuffer, d]);
    const bytesPerFrame = micChannels === 1 ? 6 : 8;
    
    const completeFrames = Math.floor(tcpDataBuffer.length / bytesPerFrame);
    const bytesToSend = completeFrames * bytesPerFrame;
    
    if (bytesToSend > 0) {
      const validData = tcpDataBuffer.subarray(0, bytesToSend);
      broadcastBinary(validData);
      tcpDataBuffer = tcpDataBuffer.subarray(bytesToSend);
    }
  });
  
  tcpSocket.on('error', (e) => { 
    console.error('❌ TCP error:', e.message); 
    broadcast({ type: 'tcp', status: 'error', msg: e.message, ip: targetIp }); 
    tcpSocket = null; 
  });
  
  tcpSocket.on('close', () => { 
    console.log('TCP closed'); 
    broadcast({ type: 'tcp', status: 'closed', ip: targetIp }); 
    tcpSocket = null; 
  });
}

// 断开 TCP 连接
function disconnectTCP() {
  if (tcpSocket) {
    console.log('主动断开 TCP');
    tcpSocket.destroy();
    tcpSocket = null;
    broadcast({ type: 'tcp', status: 'closed' });
  }
}

setInterval(() => {
  if (tcpSocket && !tcpSocket.destroyed && Date.now() - lastRecvTime > 1000) {
    if (currentTestMode) {
      console.log('TCP 1秒无数据，重发指令');
      for (let i = 0; i < 3; i++) setTimeout(() => sendUDP(`wifi_pcm_start-${currentTestMode}`, deviceIP, devicePort), i * 50);
    }
    lastRecvTime = Date.now();
  }
}, 1000);

// ---------------- WS 消息处理 ----------------
wss.on('connection', (ws) => {
  console.log('WS client connected');

  if (udpListening) {
    ws.send(JSON.stringify({ type: 'udpStatus', status: 'listening', address: '0.0.0.0', port: UDP_PORT }));
  }

  ws.on('message', (raw) => {
    let cmd; try { cmd = JSON.parse(raw.toString()); } catch { return; }

    if (cmd.type === 'connectDevice') {
      if (cmd.ip) {
        console.log(`[WS 收到指令] 准备连接设备 ${cmd.ip}`);
        connectTCP(cmd.ip);
      }
    } 
    else if (cmd.type === 'disconnectDevice') {
      console.log(`[WS 收到指令] 准备断开 TCP`);
      disconnectTCP();
    }
    else if (cmd.type === 'udpSend') {
      if (cmd.ip && cmd.payload) {
        console.log(`[准备发送 UDP] 目标: ${cmd.ip}:${cmd.port} 内容: "${cmd.payload}"`);
        sendUDP(cmd.payload, cmd.ip, cmd.port);
      }
    }
  });
});