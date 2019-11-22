# Load scan support object from library
# GXSM_USE_LIBRARY <gxsm3-lib-scan>

import selectors
import socket
import sys
import os
import fcntl
import json

## CONNECTION CONFIG

HOST = '127.0.0.1'  # Standard loopback interface address (localhost)
PORT = 65432        # Port to listen on (non-privileged ports are > 1023)

##

sys.stdout.write ("************************************************************\n")
sys.stdout.write ("* GXSM Py Socket Server is listening on "+HOST+":"+str(PORT)+"\n")
sc=gxsm.get ("script-control")
sys.stdout.write ('* Script Control is set to {} currently.\n'.format(int(sc)))
sys.stdout.write ('* Set Script Control >  0 to keep server alife! 0 will exit.\n')
sys.stdout.write ('* Set Script Control == 1 for idle markings...\n')
sys.stdout.write ('* Set Script Control >  1 for silence.\n')
sys.stdout.write ("************************************************************\n\n")

## message processing

def process_message(jmsg):
    print (jmsg)
    for cmd in jmsg.items():
        if cmd == 'command':
            if jmsg['command'] == 'set':
                gxsm.set(jmsg['command']['set'], jmsg['command']['value'])
                return {'result': [{jmsg['command']}]}
            elif jmsg['command'] == 'get':
                value=gxsm.get(jmsg['command']['get'])
                return {'result': [{'get':jmsg['command']['get'], 'value':value}]}
            else:
                return {'result': 'invalid command'}
        elif cmd == 'action':
            return {'result': 'ok'}
        elif cmd == 'echo':
            return {'result': [{'echo': jmsg['echo']['message']}]}
        else: return {'result': 'invalid request'}
            
## socket server 
    
# set sys.stdin non-blocking
def set_input_nonblocking():
    orig_fl = fcntl.fcntl (sys.stdin, fcntl.F_GETFL)
    fcntl.fcntl (sys.stdin, fcntl.F_SETFL, orig_fl | os.O_NONBLOCK)

def create_socket (server_addr, max_conn):
    sys.stdout.write ("*** GXSM Py Socket Server is now listening on "+HOST+":"+str(PORT)+"\n")
    server = socket.socket (socket.AF_INET, socket.SOCK_STREAM)
    server.setsockopt (socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    server.setblocking (False)
    server.bind (server_addr)
    server.listen (max_conn)
    return server

def try_connect (server, server_addr):
    try:
        server.connect (server_addr)
        return True
    except:
        pass
    return False
        
def read(conn, mask):
    global keep_alive
    try:
        client_address = conn.getpeername ()
        jdata = receive_json(conn)
        print ('Got {} from {}'.format(jdata, client_address))
        send_json(conn, process_message (jdata))
        if not jdata:
            keep_alive = True
    except:
        pass
            
def send_as_json(server, data):
    try:
        serialized = json.dumps(data)
    except (TypeError, ValueError):
        raise Exception('You can only send JSON-serializable data')
    # send the length of the serialized data first
    server.socket.send(b'%d\n' % len(serialized))
    # send the serialized data
    socket.sendall(serialized)

def receive_json(conn):
    # read the length of the data, letter by letter until we reach EOL
    length_str = b''
    char = conn.recv(1)
    while char != '\n':
        length_str += char
        char = conn.recv(1)
    total = int(length_str)
    # use a memoryview to receive the data chunk by chunk efficiently
    view = memoryview(bytearray(total))
    next_offset = 0
    while total - next_offset > 0:
        recv_size = conn.recv_into(view[next_offset:], total - next_offset)
        next_offset += recv_size
        try:
            deserialized = json.loads(view.tobytes())
        except (TypeError, ValueError):
            raise Exception('Data received was not in JSON format')
    return deserialized
    
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

## MAIN ##

keep_alive = True

m_selector = selectors.DefaultSelector ()

set_input_nonblocking()
m_selector.register (sys.stdin, selectors.EVENT_READ, from_keyboard)

sys.stdout.write ("*** GXSM Py Socket Server is (re)starting...\n")
# listen to port 10000, at most 10 connections

server_addr = (HOST, PORT)
server = create_socket (server_addr, 10)
m_selector.register (server, selectors.EVENT_READ, accept)

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

sys.stdout.write ("*** GXSM Py Socket Server connection shutting down...\n")
m_selector.unregister (server)

# close connection
server.shutdown (socket.SHUT_RDWR)
server.close ()

# unregister events
m_selector.unregister (sys.stdin)

#  close select
m_selector.close ()

sys.stdout.write ("*** GXSM Py Socket Server Finished. ***\n")
