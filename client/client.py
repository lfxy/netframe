#! /usr/bin/python


import socket
import time
from time import clock
 
def Client():
    sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    sock.connect(('192.168.171.128',8081))
    time.sleep(1)
    start = clock()
    for i in  range(12000):
        sock.send("ee")
        sock.recv(1024)
    end = clock()
    sock.close()
    print (end - start)# / 10000
if __name__=='__main__':
    Client()
