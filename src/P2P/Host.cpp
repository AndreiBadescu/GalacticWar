//#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "P2P/Host.hpp"

#include <iostream>
#include <cstdint>
#include <memory>

namespace gal {
	Host::Host() {
		GetHostExternalIpAddr();
	}

	void Host::GetHostExternalIpAddr() {
		std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen("curl http://ipinfo.io/ip", "r"), _pclose);
		if (!pipe) {
			//std::cerr << "Couldn't get the external ip adress !\n";
			m_external_ip_addr = "Unknown";
		}
		std::string result;
		char buffer[48];
		while (fgets(buffer, sizeof(buffer), pipe.get())) {
			m_external_ip_addr = buffer;
		}
		LOG_MESSAGE("Adresa ip externa a fost primita: %s", m_external_ip_addr.c_str());
	}

	bool Host::ListenForNewConnection() {
		// Create socket
		SOCKET listen_sock = socket(AF_INET, SOCK_STREAM, 0);
		if (listen_sock == INVALID_SOCKET) {
			LOG_ERROR("Couldn't initialize listening socket #%d", WSAGetLastError());
			Reset();
			return false;
		}

		// Bind the host ip adress and port to socket
		sockaddr_in hint;
		hint.sin_family = AF_INET;
		hint.sin_port = htons(PORT);
		hint.sin_addr.S_un.S_addr = INADDR_ANY;

		bind(listen_sock, (sockaddr*)&hint, sizeof(hint));
		//if (bind(listen_sock, (sockaddr*)&hint, sizeof(hint)) == SOCKET_ERROR) {
		//	std::cout << "[-] Bind failed\n";
		//	closesocket(listen_sock);
		//	//Reset();
		//	return;
		//}
		// Tell winsock the socket is for listening
		if (listen(listen_sock, SOMAXCONN) == SOCKET_ERROR) {
			closesocket(listen_sock);
			Reset();
			return false;
		}

		// Wait for a connection
		sockaddr_in client;
		int32_t client_size = sizeof(client);

		LOG_MESSAGE("Se asteapta conexiunea din partea clientului");
		m_peer_sock = accept(listen_sock, (sockaddr*)&client, &client_size);
		closesocket(listen_sock);
		if (m_peer_sock == INVALID_SOCKET) {
			LOG_ERROR("A connection could not be established");
			return false;
		}

		LOG_MESSAGE("A connection with %s has been established", GetClientName(client).c_str());
		return true;
	}

	std::string Host::GetClientName(const sockaddr_in& client) const {
		char host[NI_MAXHOST];      // Client's remote name
		char service[NI_MAXSERV];   // Service (i.e. port) the client is connected on

		ZeroMemory(host, sizeof(host));
		ZeroMemory(service, sizeof(service));

		getnameinfo((sockaddr*)&client, sizeof(client), host, NI_MAXHOST, service, NI_MAXSERV, 0);

		return std::string(host);
	}
}