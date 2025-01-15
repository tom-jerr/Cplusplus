#pragma once
#include <vector>
#include <string>
class Server;
class Client;

static std::vector<std::string> pending_trxs;;
void  show_pending_transactions();
void  show_wallets(const  Server& server);