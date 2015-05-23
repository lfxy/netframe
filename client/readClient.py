#! /usr/bin/python


import socket
import time
from time import clock
 
def Client():
    sock=socket.socket(socket.AF_INET,socket.SOCK_STREAM)
    sock.connect(('127.0.0.1',8086))
    #sock.connect(('172.16.31.59',8085))
    time.sleep(1)
#    start = clock()
    for i in  range(1000000):
#        sock.send("abee")
        print sock.recv(1024)
#    end = clock()
    sock.close()
#    print (end - start)# / 10000
if __name__=='__main__':
    Client()


#tail -f time.log | uniq -c
