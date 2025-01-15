#ifndef SERVER_H
#define SERVER_H
#include <memory>
#include <map>
#include <vector>

class Client;
class Server
{
public:
	Server();
  ~Server() = default;
  /**
   * @brief This function will create a new Client with the specified id. If this id already exists, the server should add a random 4 digit number at the end of it automatically. each client should be assigned with 5 coins at the begining.
   * 
   * @param id 
   * @return std::shared_ptr<Client> 
   */
	std::shared_ptr<Client> add_client(std::string id);

  /**
   * @brief Get the client objectUsing this function you can get a pointer to a Client using its id.
   * 
   * @param id 
   * @return std::shared_ptr<Client> 
   */
	std::shared_ptr<Client> get_client(std::string id) const;

  /**
   * @brief Using this function will return the wallet value of the client with username id.
   * 
   * @param id 
   * @return double 
   */
	double get_wallet(std::string id);

  /**
    * @brief Get the wallet objecti) id of the sender ii) id of the receiver iii) value of money to transfer.
    * 
    * @param id 
    * @return double 
    */
	static bool parse_trx(std::string trx, std::string& sender, std::string& receiver, double& value);
	
  /**
   * @brief  Each Client can add a pending transaction using the transaction format described in the above section.
   * 
   * @param trx 
   * @param signature 
   * @return true 
   * @return false 
   */
  bool add_pending_trx(std::string trx, std::string signature);

	size_t mine();

  std::map<std::shared_ptr<Client>, double> Clients() const {
    return clients;
  }
private:
	std::map<std::shared_ptr<Client>, double> clients;
};
#endif //SERVER_H