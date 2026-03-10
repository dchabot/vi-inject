import socket
import sys

# Define the IP and port to listen on
UDP_IP = "10.139.4.51"
UDP_PORT = 5555

# Create a UDP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

# Bind the socket to the IP and port to listen for incoming packets
sock.bind((UDP_IP, UDP_PORT))

print(f"UDP server listening on {UDP_IP}:{UDP_PORT}")

L = 0
try:
    F = open(sys.argv[1], 'wb');
    # Receive data and the address of the sender
    while True:
        data, addr = sock.recvfrom(1500) # buffer size is 1500 bytes
        L += len(data)
        #print(f"Received {len(data)} bytes from {addr}")
        F.write(data)
except KeyboardInterrupt:
    print('Caught Ctrl-C. Exiting...')
finally:
    F.flush()
    F.close()

print(f"Received {L} bytes")

