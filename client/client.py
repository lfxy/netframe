#! /usr/bin/python


import socket
import time
 
def Client():
    sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    sock.connect(('192.168.171.128',8081))
    time.sleep(1)
    for i in  range(1000):
        sock.send("czq")
        print sock.recv(1024)
    sock.close()
     
if __name__=='__main__':
    Client()
