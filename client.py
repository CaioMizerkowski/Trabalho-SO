import socket

s = socket.socket()
s.connect(('127.0.0.1',5193))
while True:
    msg = input("S: ")
    if msg:
        s.send(msg.encode())
        print("N:",s.recv(1024).decode())
s.close()
