import socket

if __name__ == "__main__":
    choice = 'r'

    while(choice == 'r' or choice == 'd'):
        choice = input("Do you want to register or deregister or any other key to exit? (r/d): ")

        if choice == 'r':

            name = input("Enter your name: ")
            udp_port = input("Enter your UDP port: ")
            tcp_port = input("Enter your TCP port: ")
            rq_num = input("Enter your Request Number: ")

            client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            server_address = ('localhost', 12345)
            
            ip_address = "localhost"  # Assume the client IP is localhost for simplicity
            
            register_message = f"REGISTER\n{rq_num}\n{name}\n{ip_address}\n{udp_port}\n{tcp_port}"
            client_socket.sendto(register_message.encode(), server_address)
            
            response, _ = client_socket.recvfrom(1024)
            print("Response from server:", response.decode())

        elif choice == 'd':

            name = input("Enter your name to deregister: ")
            client_socket = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            server_address = ('localhost', 12345)

            rq_num = "1"
            deregister_message = f"DE-REGISTER\n{rq_num}\n{name}"
            client_socket.sendto(deregister_message.encode(), server_address)
            
            response, _ = client_socket.recvfrom(1024)
            print("Response from server:", response.decode())
