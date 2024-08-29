import socket
import threading

"""

- The server listens on port 9999 for incoming connections.

F- or each client connection, a new thread is created to handle the client’s requests.

- The handle_client function processes the put and get commands. For put, it receives 
the file data and writes it to a file. For get, it reads the file and 
sends the data to the client.

"""

def handle_client(client_socket):
    """
    Handles client requests for file upload (put) and download (get).

    Parameters:
    client_socket (socket.socket): The socket object representing the client connection.

    Returns:
    None
    """
    while True:
        command = client_socket.recv(1024).decode()
        if not command:
            break
        command, filename = command.split()

        if command == 'put':
            with open(filename, 'wb') as f:
                while True:
                    data = client_socket.recv(1024)
                    if not data:
                        break
                    f.write(data)
            print(f"Received file: {filename}")

        elif command == 'get':
            try:
                with open(filename, 'rb') as f:
                    while (data := f.read(1024)):
                        client_socket.send(data)
                print(f"Sent file: {filename}")
            except FileNotFoundError:
                client_socket.send(b'File not found')

        client_socket.close()
        break


def start_server():
    """
    Starts the server to listen for incoming client connections on port 9999.

    This function sets up a socket to listen for incoming connections on all available
    network interfaces (0.0.0.0) and port 9999. For each client connection, it spawns
    a new thread to handle the client's requests.

    Parameters:
    None

    Returns:
    None
    """
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind(('0.0.0.0', 9999))
    server.listen(5)
    print("Server listening on port 9999")

    while True:
        client_socket, addr = server.accept()
        print(f"Accepted connection from {addr}")
        client_handler = threading.Thread(target=handle_client, args=(client_socket,))
        client_handler.start()


if __name__ == "__main__":
    start_server()
