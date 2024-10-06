import socket

# {Name: (IP, UDP Socket#, TCP Socket#)}
registered_users = {}

if __name__ == "__main__":
    #AF_INET is used for IPv4 addresses
    #SOCK_DGRAM is used for Datagram-based communication
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM) 
    server_socket.bind(('localhost', 12345)) 
    print("Server Started")

    while True:
        message, addr = server_socket.recvfrom(1024)
        request = message.decode().split('\n')

        if request[0] == "REGISTER":
            print("request recieved:",request)
            
            # rq_num  request[1]
            # name request[2]
            # ip request[3]
            # udp_port request[4]
            # tcp_port request[5]

            if request[2] in registered_users:
                response = f"REGISTER-DENIED\n{request[1]}\nName already in use"
                server_socket.sendto(response.encode(), addr)
            elif len(registered_users) >= 100:  # Arbitrary max clients, modify as needed
                response = f"REGISTER-DENIED\n{request[1]}\nServer cannot handle more clients"
                server_socket.sendto(response.encode(), addr)
            else:
                registered_users[request[2]] = (request[3], request[4], request[5])
                response = f"REGISTER-ACCEPTED\n{request[1]}"
                server_socket.sendto(response.encode(), addr)
            print("sending response: ", response)
            
        elif request[0] == "DE-REGISTER":
            # rq_num request[1], 
            # name request[2]

            if request[2] in registered_users:
                del registered_users[request[2]]
                response = f"DE-REGISTER-ACCEPTED\n{request[1]}"
                server_socket.sendto(response.encode(), addr)
            else:
                # No action is taken if the name is not registered
                response = f"DE-REGISTER-IGNORED\n{request[1]}"
                server_socket.sendto(response.encode(), addr)   
