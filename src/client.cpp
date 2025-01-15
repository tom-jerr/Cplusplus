#include "client.h"
#include "server.h"
#include "crypto.h"
#include "utils.h"

#include <random>
#include <string>

Client::Client(std::string id, const Server& server):id(id), server(&server) {
  crypto::generate_key(public_key, private_key);
}

std::string Client::get_id() const {
  return id;
}

std::string Client::get_publickey() const{
  return public_key;
}

double Client::get_wallet() {
  return const_cast<Server*>(server)->get_wallet(id);
}

std::string Client::sign(std::string txt) const {
  return crypto::signMessage(private_key, txt);
}

bool Client::transfer_money(std::string receiver, double value) {
  std::string transaction = id + "-" + receiver + "-" + std::to_string(value);
  auto client = server->get_client(receiver);
  if(client == nullptr) {
    return false;
  }
  bool succ = const_cast<Server*>(server)->add_pending_trx(transaction, sign(transaction));
  // std::cout << pending_trxs.size() << std::endl;

  return succ;
}

size_t Client::generate_nonce() {
  // 生成随机的4位末尾数字
  std::random_device rd;
  std::mt19937 generate(rd());
  std::uniform_int_distribution<size_t> dist(generate());
  return dist(generate);
}
