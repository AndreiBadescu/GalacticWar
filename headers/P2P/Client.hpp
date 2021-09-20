#pragma once

#include "NetworkInterface.hpp"

namespace gal {
	class Client : public NetworkInterface<Client> {
	private:
		friend class NetworkInterface<Client>;

		Client() = default;
		Client(const Client&) = delete;
		Client& operator=(const Client&) = delete;
		~Client() = default;

		bool ConnectToHost(const char* const pIPAddress);
	};
}

