#include "P2P/Client.hpp"

#include <string>
#include <iostream>
#include <cstdint>

namespace gal {
	std::string ReadIpAddr() {
		std::cout << "Introdu adresa ip a host-ului: ";
		std::string ip_addr;
		std::cin >> ip_addr;
		return ip_addr;
	}

	bool Client::ConnectToHost(const char* const pIPAddress) {
		//static std::string IP_ADRESS = ReadIpAddr();
		// Create socket
		m_peer_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (m_peer_sock == INVALID_SOCKET) {
			LOG_ERROR("Can't create socket #%d", WSAGetLastError());
			return false;
		}

		// Hint structure
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(PORT);
		inet_pton(AF_INET, pIPAddress, &hint.sin_addr);

		// Connect to server
		if (connect(m_peer_sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
			LOG_ERROR("Connection to the host couldn't be establish #%d", WSAGetLastError());
			return false;
		}
		LOG_MESSAGE("Connection with host succeded");
		return true;
	}
}