import socket
import threading
import time
import random
import string

SERVER_HOST = "127.0.0.1"
SERVER_PORT = 9090
NUM_CLIENTS = 10


def stress_test(username):
    try:
        sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        sock.connect((SERVER_HOST, SERVER_PORT))

        # Send username.
        sock.sendall((username + "\n").encode())
        time.sleep(0.2)

        # Join a channel.
        sock.sendall(b"/join testroom\n")
        time.sleep(0.2)

        # Send a couple of messages.
        for _ in range(2):
            msg = ''.join(random.choices(string.ascii_letters + string.digits, k=8))
            sock.sendall((msg + "\n").encode())
            time.sleep(0.2)

        # Quit
        sock.sendall(b"/quit\n")
        sock.close()
    except Exception as e:
        print(f"{username}: {e}")


def main():
    threads = []
    for i in range(NUM_CLIENTS):
        username = f"Client{i}"
        t = threading.Thread(target=stress_test, args=(username,))
        t.start()
        threads.append(t)
        time.sleep(0.1)  # Stagger connections.

    for t in threads:
        t.join()

    print("Stress test complete.")


if __name__ == "__main__":
    main()
