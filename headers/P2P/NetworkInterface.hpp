#pragma once

#include <WinSock2.h>
#include <WS2tcpip.h>
#include "P2P/Packet.hpp"
#include "Utility/ErrorHandler.hpp"
#include <iostream>

#pragma comment (lib, "Ws2_32.lib")

#define PORT 54000

namespace gal {
	class Host;
	class Client;

	template<class T>
	class NetworkInterface {
	public:
		static bool StartSession(const char* const pIPAddress = nullptr) {
			if (Get().InitWSA()) {
				if constexpr (std::is_same_v<T, Client>) {
					return Get().ConnectToHost(pIPAddress);
				} else {
					return Get().ListenForNewConnection();
				}
			}
		}

		static void EndSession() {
			closesocket(Get().m_peer_sock);
			WSACleanup();
		}

		static bool SendPacket(const Packet& pack) {
			if (send(Get().m_peer_sock, (char*)&pack, sizeof(pack), 0) == SOCKET_ERROR) {
				LOG_ERROR("Nu s-a putut trimite packetul #%d", WSAGetLastError());
				return false;
			}
			LOG_MESSAGE("S-a trimis packetul");
			return true;
		}

		static Packet ReceivePacket() {
			Packet pack;	
			int32_t bytes_received = recv(Get().m_peer_sock, (char*)&pack, sizeof(pack), 0);
			if (bytes_received == SOCKET_ERROR) {
				LOG_ERROR("Nu s-a putut primi packetul #%d", WSAGetLastError());
				return {2, CORRUPTED_PACKET};
			}

			if (bytes_received == 0) {
				LOG_WARNING("Client disconnected");
				Get().Reset();
				return {2, CORRUPTED_PACKET};
			}

			if (bytes_received != sizeof(pack)) {
				LOG_ERROR("Packetul primit este corupt");
				return {2, CORRUPTED_PACKET};
			}

			LOG_MESSAGE("S-a primit packetul");
			return pack;
		}

	protected:
		SOCKET m_peer_sock;

		NetworkInterface() = default;
		NetworkInterface(const NetworkInterface&) = delete;
		NetworkInterface& operator=(const NetworkInterface&) = delete;
		virtual ~NetworkInterface() = default;

		static T& Get() noexcept {
			static T unique_instance;
			return unique_instance;
		}

		bool InitWSA() const {
			WSADATA ws_data;
			if (WSAStartup(MAKEWORD(2, 2), &ws_data)) {
				LOG_ERROR("Initialization of the WinSock API failed #%d", WSAGetLastError());
				return false;
			}
			return true;
		}

		void Reset() const {
			StartSession();
			EndSession();
		}
	};
}
