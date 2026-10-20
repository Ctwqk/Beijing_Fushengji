"""Minimal OpenAI-compatible /chat/completions server used by the HTTP tests.

The first path segment selects the behaviour:
  /ok/...        a valid random-event reply
  /echo/...      replies with the Authorization header and the request body it received
  /garbage/...   a syntactically valid chat reply whose content is not an event
  /malformedkey/...       invalid event JSON echoing a fake API credential
  /malformedendpoint/...  invalid event JSON echoing the configured base URL
  /unauthorized/ HTTP 401 with an OpenAI-style error object
  /badgateway/   HTTP 502 with an HTML body
  /slow/...      answers after 5 seconds
"""
import json
import threading
import time
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer

EVENT = {
    "title": "Mock Event",
    "description": "A mock server hands you an envelope.",
    "options": [
        {"text": "Open it", "cash": 300, "health": 0, "reputation": -5},
        {"text": "Burn it", "cash": 0, "health": -5, "reputation": 10},
        {"text": "Return it", "cash": -50, "health": 0, "reputation": 25},
    ],
}


def chat_reply(content):
    return {"id": "mock", "object": "chat.completion",
            "choices": [{"index": 0, "message": {"role": "assistant", "content": content}, "finish_reason": "stop"}]}


class Handler(BaseHTTPRequestHandler):
    def log_message(self, *args):  # keep test output quiet
        pass

    def _send(self, status, body, content_type="application/json"):
        data = body if isinstance(body, bytes) else json.dumps(body).encode()
        self.send_response(status)
        self.send_header("Content-Type", content_type)
        self.send_header("Content-Length", str(len(data)))
        self.end_headers()
        try:
            self.wfile.write(data)
        except (BrokenPipeError, ConnectionResetError):
            pass

    def do_POST(self):
        length = int(self.headers.get("Content-Length", 0))
        raw = self.rfile.read(length)
        mode = self.path.strip("/").split("/")[0]
        if not self.path.endswith("/chat/completions"):
            return self._send(404, {"error": {"message": "unknown route " + self.path}})
        if mode == "ok":
            self._send(200, chat_reply("Here you go:\n```json\n" + json.dumps(EVENT) + "\n```"))
        elif mode == "echo":
            self._send(200, chat_reply(json.dumps({
                "authorization": self.headers.get("Authorization"),
                "content_type": self.headers.get("Content-Type"),
                "body": json.loads(raw),
            })))
        elif mode == "garbage":
            self._send(200, chat_reply("I'm sorry, I can't write events."))
        elif mode in {"malformedkey", "malformedendpoint"}:
            sensitive = (self.headers.get("Authorization", "") if mode == "malformedkey" else
                         "http://" + self.headers["Host"] + self.path.removesuffix("/chat/completions"))
            # A raw newline makes the event JSON invalid after the sensitive
            # token, so a JSON parser's detailed error can echo that token.
            self._send(200, chat_reply('{"title":"' + sensitive + '\n"}'))
        elif mode == "unauthorized":
            # Some providers echo rejected credentials. Error reporting must not log them.
            self._send(401, {"error": {"message": "Incorrect API key provided: " +
                                     self.headers.get("Authorization", ""), "type": "invalid_request_error"}})
        elif mode == "error200":
            self._send(200, {"error": {"message": "Rejected: " + self.headers.get("Authorization", "")}})
        elif mode == "badgateway":
            self._send(502, b"<html><body>502 Bad Gateway</body></html>", "text/html")
        elif mode == "slow":
            time.sleep(5)
            self._send(200, chat_reply(json.dumps(EVENT)))
        else:
            self._send(404, {"error": {"message": "unknown mode " + mode}})


def start(port=0):
    server = ThreadingHTTPServer(("127.0.0.1", port), Handler)
    server.daemon_threads = True
    threading.Thread(target=server.serve_forever, daemon=True).start()
    return server


if __name__ == "__main__":
    s = start(0)
    print("listening on http://127.0.0.1:%d" % s.server_address[1], flush=True)
    threading.Event().wait()
