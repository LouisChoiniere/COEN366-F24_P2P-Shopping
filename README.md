# COEN366-F24_P2P-Shopping

## Introduction
Welcome to the Peer-to-Peer Shopping System (P2S2) project! This project is part of the Communication Networks and Protocols course (COEN 366) at Concordia University. The goal of this project is to create a shopping system where users can buy and sell items through a server that facilitates communication between peers.

## Project Overview
The Peer-to-Peer Shopping System (P2S2) allows users to search for items and purchase them from other users. The system consists of several peers (users) and one server. Users can register with the server, search for items, and finalize purchases. The server acts as an intermediary, ensuring that users do not communicate directly with each other.

## Features
- **User Registration and De-registration**: Users must register with the server to use the service. They can also de-register when they no longer wish to use the service.
- **Item Search**: Registered users can search for items they wish to buy. The server broadcasts the search request to all other registered users.
- **Offers and Negotiation**: Users who have the requested item can make offers. The server facilitates negotiation if the offer price is higher than the buyer's maximum price.
- **Purchase Finalization**: Once an agreement is reached, the server helps finalize the purchase by collecting payment information and providing shipping details.

## Communication
- **UDP Communication**: Used for registration, de-registration, and item search.
- **TCP Communication**: Used for finalizing purchases, including payment and shipping information.

## Getting Started
### Recomended IDE
We recomend to use the CLion IDE from JetBrains to run this project.
### Prerequisites
- CMake 3.25.1 or higher
- A C++ compiler that supports C++20
### Building the Project
To build the project, run the following command:
```cmake --build .```
### Running the Server
To start the server, execute:
```ServerExecutable```
### Running the Client
To start a client, execute:
```ClientExecutable```
### Program Arguments
Don't forget to add the program arguments when running the client/server
Example arguments to use on the client
```peer1 127.0.0.1 8080 5000 5001```
