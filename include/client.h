#ifndef CLIENT_H
#define CLIENT_H
#include <string>

class Server;
class Client
{
public:
/**
 * @brief Construct a new Client object Creates an object of Client and assigning the specified variables using the inputs. Also generate RSA keys for the client (public and private keys).
 * 
 * @param id 
 * @param server 
 */
	Client(std::string id, const Server& server);

  ~Client() = default;

  /**
   * @brief Get the id objectReturns the Client's id.
   * 
   * @return std::string 
   */
	std::string get_id() const;

  /**
   * @brief Get the publickey object Returns the Client's public key.
   * 
   * @return std::string 
   */
	std::string get_publickey() const;

  /**
   * @brief Get the wallet objectReturns the amount of money the client has.
   * 
   * @return double 
   */
	double get_wallet();

  /**
   * @brief  sign signs the input with the private key and returns the signature.
   * 
   * @param txt 
   * @return std::string 
   */
	std::string sign(std::string txt) const ;

  /**
   * @brief Creates a transaction in the server according to its inputs. To create a transaction use the specified string format described in above sections and sign the final transaction string with your private key. use both your signature and your transaction signature to create a pending transaction in the Server using add_pending_trx function.
   * 
   * @param receiver 
   * @param value 
   * @return true 
   * @return false 
   */
	bool transfer_money(std::string receiver, double value);

  /**
   * @brief Returns a random number as a nonce so the server uses it for mining
   * 
   * @return size_t 
   */
	size_t generate_nonce();

private:
	Server const* const server;
	const std::string id;
	std::string public_key;
	std::string private_key;
};
#endif //CLIENT_H