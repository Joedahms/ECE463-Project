import socket


SERVER_HOST = '127.0.0.1'  # localhost
SERVER_PORT = 5000         # Server port
BUFFER_SIZE = 4096         # message buffer
SEPARATOR = "<SEPARATOR>"  # delimiter

# Function to upload a file to the server
def put_file(client_socket, filename):
    try:
        with open(filename,"rb") as f:
            file_content=f.read()

        
        # this is message to send to the server to read
        message = f"put{SEPARATOR}{filename}{SEPARATOR}".encode() + file_content
        
        # Send the message to the server
        client_socket.sendall(message)
        print(f"File '{filename}' uploaded successfully.")


        #error handeling
    except FileNotFoundError:
        print(f"File '{filename}' not found.")
    except Exception as e:
        print(f"Error uploading file: {e}")

