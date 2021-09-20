#pragma once

#include <string>
#include "NetworkInterface.hpp"

namespace gal {
	class Host : public NetworkInterface<Host> {
	private:
		friend class NetworkInterface<Host>;

		std::string m_external_ip_addr;

		Host();
		Host(const Host&) = delete;
		Host& operator=(const Host&) = delete;
		~Host() = default;

		void GetHostExternalIpAddr();
		bool ListenForNewConnection();
		std::string GetClientName(const sockaddr_in& client) const;
	};
}

