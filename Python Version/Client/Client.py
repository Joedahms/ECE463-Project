import socket

"""
- The client connects to the server and sends the put or get command along with the filename.

- For put, it reads the file and sends the data to the server.

- For get, it receives the file data from the server and writes it to a file.
"""

def put_file(filename: str) -> None:
    """
    This function establishes a TCP connection to a server, sends a 'put' command along with the filename,
    and then reads the file in chunks of 1024 bytes, sending each chunk to the server.

    Parameters:
    filename (str): The name of the file to be sent to the server.

    Returns:
    None: The function does not return any value.
    """
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(('3.139.61.201', 9999))
    client.send(f'put {filename}'.encode())

    with open(filename, 'rb') as f:
        while (data := f.read(1024)):
            client.send(data)
    client.close()


def get_file(filename):
    """
    This function establishes a TCP connection to a server, sends a 'get' command along with the filename,
    and then receives the file data from the server in chunks of 1024 bytes, writing each chunk to a file.

    Parameters:
    filename (str): The name of the file to be received from the server.

    Returns:
    None: The function does not return any value.
    """
    client = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    client.connect(('127.0.0.1', 9999))
    client.send(f'get {filename}'.encode())

    with open(filename, 'wb') as f:
        while True:
            data = client.recv(1024)
            if not data:
                break
            f.write(data)
    client.close()


if __name__ == "__main__":
    x = True
    while x:
        command = input("Enter command (put/get filename): ")
        parts = command.split()
        
        if len(parts) == 2:
            cmd, filename = parts
            if cmd == 'put':
                put_file(filename)
            elif cmd == 'get':
                get_file(filename)
            else:
                print("Invalid command. Please enter 'put' or 'get' followed by the filename.")
        elif command.lower() == 'exit':
            x = False
        else:
            print("Invalid input. Please enter a command in the format 'put/get filename' or type 'exit' to quit.")
