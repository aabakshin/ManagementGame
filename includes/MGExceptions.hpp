#ifndef MGEXCEPTIONS_HPP_SENTINEL
#define MGEXCEPTIONS_HPP_SENTINEL


#include "Player.hpp"
#include <string>
#include <list>
#include <stdexcept>


class KickBankrotsException
{
public:

	typedef std::list<std::pair<int, std::string>> BankrotsList;

private:
	BankrotsList bankrots;
public:
	explicit KickBankrotsException( const BankrotsList& b_rots ) : bankrots(b_rots)
		{}
	const BankrotsList& GetBankrots() const { return bankrots; }
};

class QuitCommandException : public std::runtime_error
{
private:
	char addr[Player::ADDRESS_SIZE];
public:
	explicit QuitCommandException( const std::string& msg, const std::string& s ) : std::runtime_error(msg)
	{
		ssize_t cnt = s.copy(addr, s.length());
		addr[cnt] = '\0';
	}
	const char* GetAddr() const { return addr; }
};

class InternalCmdExecuteException : public std::runtime_error
{
private:
	char addr[Player::ADDRESS_SIZE];
public:
	explicit InternalCmdExecuteException( const std::string& msg, const std::string& s ) : std::runtime_error(msg)
	{
		ssize_t cnt = s.copy(addr, s.length());
		addr[cnt] = '\0';
	}
	const char* GetAddr() const { return addr; }
};

class PlayerLostConnectionException : public std::runtime_error
{
private:
	char addr[Player::ADDRESS_SIZE];
public:
	explicit PlayerLostConnectionException( const std::string& msg, const std::string& s ) : std::runtime_error(msg)
	{
		ssize_t cnt = s.copy(addr, s.length());
		addr[cnt] = '\0';
	}
	const char* GetAddr() const { return addr; }
};

#endif
