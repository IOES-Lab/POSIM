import http from 'node:http';
import { stat } from 'node:fs/promises';
import { createReadStream } from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
const root = fileURLToPath(new URL('../dist/', import.meta.url));
const port = Number(process.env.PORT || 4174);
const types = { '.html': 'text/html; charset=utf-8', '.css': 'text/css', '.js': 'text/javascript', '.json': 'application/json', '.svg': 'image/svg+xml', '.xml': 'application/xml', '.txt': 'text/plain', '.png': 'image/png', '.jpg': 'image/jpeg', '.jpeg': 'image/jpeg', '.gif': 'image/gif', '.mp4': 'video/mp4' };
http.createServer(async (request, response) => {
  try {
    if (!['GET', 'HEAD'].includes(request.method)) {
      response.writeHead(405, { Allow: 'GET, HEAD' }); response.end(); return;
    }
    let url = decodeURIComponent(new URL(request.url, 'http://localhost').pathname);
    if (url.endsWith('/')) url += 'index.html';
    const file = path.resolve(root, '.' + url);
    if (!file.startsWith(root)) throw new Error('Invalid path');
    const metadata = await stat(file);
    if (!metadata.isFile()) throw new Error('Not a file');
    const headers = { 'Content-Type': types[path.extname(file)] || 'application/octet-stream', 'Accept-Ranges': 'bytes' };
    let start = 0, end = metadata.size - 1, status = 200;
    if (request.headers.range) {
      const range = /^bytes=(\d*)-(\d*)$/.exec(request.headers.range);
      if (range && (range[1] || range[2])) {
        if (range[1]) {
          start = Number(range[1]);
          if (range[2]) end = Math.min(end, Number(range[2]));
        } else start = Math.max(0, metadata.size - Number(range[2]));
      }
      if (!range || !(range[1] || range[2]) || !Number.isSafeInteger(start) || !Number.isSafeInteger(end) || start > end || start >= metadata.size) {
        response.writeHead(416, { 'Content-Range': `bytes */${metadata.size}` }); response.end(); return;
      }
      status = 206; headers['Content-Range'] = `bytes ${start}-${end}/${metadata.size}`;
    }
    headers['Content-Length'] = Math.max(0, end - start + 1);
    response.writeHead(status, headers);
    if (request.method === 'HEAD' || metadata.size === 0) { response.end(); return; }
    const stream = createReadStream(file, { start, end });
    response.on('close', () => stream.destroy());
    stream.on('error', () => response.destroy());
    stream.pipe(response);
  } catch { response.writeHead(404); response.end('Page not found'); }
}).listen(port, '127.0.0.1', () => console.log(`POSIM documentation: http://127.0.0.1:${port}`));
