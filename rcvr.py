import socket

# Define the IP and port to listen on
UDP_IP = "10.139.4.51"
UDP_PORT = 24742

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to the IP and port to listen for incoming packets
sock.bind((UDP_IP, UDP_PORT))

print(f"UDP server listening on {UDP_IP}:{UDP_PORT}")

while True:
    # Receive data and the address of the sender
    data, addr = sock.recvfrom(1024) # buffer size is 1024 bytes
    print(f"Received message: {data.decode()} from {addr}")

