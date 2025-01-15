#include "server.h"
#include "client.h"
#include "utils.h"
#include "crypto.h"

#include <iostream>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <sstream>
#include <vector>

#define INIT_COINS 5
#define NULL_COINS 0

Server::Server() = default;

std::shared_ptr<Client> Server::add_client(std::string id) {
  bool is_exist = false;
  for(auto &[client, coins] : clients) {
    if (id == client->get_id()) {
      is_exist = true;
      break;
    }
  }
  if (is_exist) {
    // 生成随机的4位末尾数字
    std::random_device rd;
    std::mt19937 generate(rd());
    std::uniform_int_distribution<int> dist(generate());
    for(size_t i = 0; i < 4; ++i) {
      id = id + std::to_string(dist(generate));
    }
  }
  auto new_client{std::make_shared<Client>(Client(id, *this))};
  clients.emplace_hint(clients.end(), new_client, INIT_COINS);
  return new_client;
}

std::shared_ptr<Client> Server::get_client(std::string id) const {
  for(auto &[client, coins] : clients) {
    if (id == client->get_id()) {
      return client;  
    }
  }
  return nullptr;
}

double Server::get_wallet(std::string id){
  for(auto &[client, coins] : clients) {
    if (id == client->get_id()) {
      return coins;  
    }
  }
  return NULL_COINS;
}

bool Server::parse_trx(std::string trx, std::string& sender, std::string& receiver, double& value) {
  size_t sender_pos = trx.find("-");
  size_t receiver_pos = trx.find("-", sender_pos + 1);
  if(sender_pos == std::string::npos || receiver_pos == std::string::npos) {
    // return false;
    throw std::runtime_error("wrong -");
  }
  sender = trx.substr(0, sender_pos);
  receiver = trx.substr(sender_pos + 1, receiver_pos - sender_pos - 1);
  std::string value_str = trx.substr(receiver_pos + 1);
  if(!value_str.empty()) {
    // std::cout << value_str << std::endl;
    value = std::stod(value_str);
    if(value < 0) {
      throw std::logic_error("money num not be negative");
    }
    return true;
  }
  return false;
}

bool Server::add_pending_trx(std::string trx, std::string signature) {
  std::string sender, receiver;
  double value;
  parse_trx(trx, sender, receiver, value);
  auto client = get_client(sender);
  bool authentic = crypto::verifySignature(client->get_publickey(), trx, signature);
  // show_pending_transactions();
  if (authentic && client->get_wallet() >= value) {
    pending_trxs.emplace_back(trx);
    std::cout << pending_trxs.size() << std::endl;
    // show_pending_transactions();
    // std::cout << trx << std::endl;
    return true;
  }
  return false;
}

size_t Server::mine() {
  std::string mempool{};
  // std::cout << pending_trxs.size() << std::endl;
  for(const auto& trx : pending_trxs) {
    mempool += trx;
    std::string sender{}, receiver{};
    double value;
    parse_trx(trx, sender, receiver, value);
    // std::cout << value << std::endl;
    clients[get_client(sender)] -= value;
    clients[get_client(receiver)] += value;
  }
  show_wallets(*this);

  while(true)
  {  
    std::vector<std::tuple<std::string, double, std::string>> laststring;
    for(auto& client : clients)
      laststring.emplace_back(mempool + std::to_string(client.first->generate_nonce()), client.first->generate_nonce(), client.first->get_id());

    for(auto& [str, nonce, id] : laststring) {
      std::string hash = crypto::sha256(str).substr(0, 10);
      if(hash.find("000") != std::string::npos) {
        clients[get_client(id)] += 6.25;
        // pending_trxs.clear();
        return nonce;
      }
      // for (size_t j = 0; j <= 7; ++j) { // 只需检查到索引 7，因为索引 7 后面还能找到 3 个字符
      //   if (hash[j] == '0' && hash[j + 1] == '0' && hash[j + 2] == '0') {
      //     std::cout << id << std::endl;
      //     clients[get_client(id)] += 6.25;
          
      //     return nonce;
      //   }   
      // }
  }
}
  // return NULL_COINS;
}