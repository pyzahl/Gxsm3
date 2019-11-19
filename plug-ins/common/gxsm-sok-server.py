# Load scan support object from library
#GXSM_USE_LIBRARY <gxsm3-lib-scan>

import selectors
import socket
import sys
import os
import fcntl

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 65432        # Port to listen on (non-privileged ports are > 1023)

gxsm.echo ("GXSM Py Socket Server is listening on "+HOST+":"+str(PORT))
sys.stdout.write ("************************************************************\n")
sys.stdout.write ("* GXSM Py Socket Server is listening on "+HOST+":"+str(PORT)+"\n")
sc=gxsm.get ("script-control")
sys.stdout.write ('* Script Control is set to {} currently.\n'.format(int(sc)))
sys.stdout.write ('* Set Script Control >  0 to keep server alife! 0 will exit.\n')
sys.stdout.write ('* Set Script Control == 1 for idle markings...\n')
sys.stdout.write ('* Set Script Control >  1 for silence.\n')
sys.stdout.write ("************************************************************\n\n")

def process_message(msg):
    if msg == b'gxsm-action-start-scan':
        gxsm.startscan ()
        return 'ok'
    elif msg == b'gxsm-action-stop-scan':
        gxsm.stopscan ()
        return 'ok'
    elif msg[0:16] == b'gxsm-set-OffsetX':
        print ('[')
        print (msg[0:16], '],[', msg[17:], ']\n')
        x=float(msg[17:])
        print(x)
        gxsm.set('OffsetX','%f'%x)
        return msg[17:]
    else:
        return '?'
        
m_selector = selectors.DefaultSelector ()

# set sys.stdin non-blocking
def set_input_nonblocking():
    orig_fl = fcntl.fcntl (sys.stdin, fcntl.F_GETFL)
    fcntl.fcntl (sys.stdin, fcntl.F_SETFL, orig_fl | os.O_NONBLOCK)

def create_socket (host, port, max_conn):
    server_addr = (host, port)
    server = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt (socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.setblocking (False)
    server.bind (server_addr)
    server.listen (max_conn)
    return server

def read(conn, mask):
    global keep_alive
    client_address = conn.getpeername ()
    data = conn.recv (1024)
    print ('Got {} from {}'.format(data, client_address))
    ret = process_message (data)
    b = 'Echo:  <{}> -> {}'.format(data,ret)
    conn.sendall (b.encode('utf-8'))
    if not data:
         keep_alive = False

def accept(sock, mask):
    new_conn, addr = sock.accept ()
    new_conn.setblocking (False)
    print ('Accepting connection from {}'.format(addr))
    m_selector.register(new_conn, selectors.EVENT_READ, read)

def quit ():
    global keep_alive
    print ('Exiting...')
    keep_alive = False

def from_keyboard(arg1, arg2):
    line = arg1.read()
    if line == 'quit\n':
        quit()
    else:
        print('User input: {}'.format(line))

keep_alive = True
set_input_nonblocking()

# listen to port 10000, at most 10 connections
server = create_socket (HOST, PORT, 10)

m_selector.register (server, selectors.EVENT_READ, accept)
m_selector.register (sys.stdin, selectors.EVENT_READ, from_keyboard)

while keep_alive:
    sc=gxsm.get ("script-control")
    if sc == 1:
        sys.stdout.write ('.')
    # sys.stdout.flush ()
    for key, mask in m_selector.select(0):
        callback = key.data
        callback (key.fileobj, mask)
    gxsm.sleep (1)
    if sc == 0:
        sys.stdout.write ('\nScript Control is 0:  Closing server down now.\n')
        quit ()

# unregister events
m_selector.unregister (sys.stdin)

# close connection
server.shutdown (socket.SHUT_RDWR)
server.close ()

#  close select
m_selector.close ()
