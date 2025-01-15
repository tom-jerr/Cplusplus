#include "utils.h"
#include "server.h"
#include "client.h"
#include <iostream>

void  show_pending_transactions()
{
	std::cout  <<  std::string(20, '*') <<  std::endl;
	for(const  auto& trx : pending_trxs)
		std::cout << trx <<  std::endl;
	std::cout  <<  std::string(20, '*') <<  std::endl;
}

void  show_wallets(const  Server& server)
{
	std::cout << std::string(20, '*') << std::endl;
	for(const auto& client: server.Clients())
		std::cout << client.first->get_id() <<  " : "  << client.second << std::endl;
	std::cout << std::string(20, '*') << std::endl;
}