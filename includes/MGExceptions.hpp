#ifndef MGEXCEPTIONS_HPP_SENTINEL
#define MGEXCEPTIONS_HPP_SENTINEL


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
public:
	explicit QuitCommandException( const std::string& msg ) : std::runtime_error(msg)
		{}
};

class InternalCmdExecuteException : public std::runtime_error
{
public:
	explicit InternalCmdExecuteException( const std::string& msg ) : std::runtime_error(msg)
		{}
};

class PlayerLostConnectionException : public std::runtime_error
{
public:
	explicit PlayerLostConnectionException( const std::string& msg ) : std::runtime_error(msg)
		{}
};

#endif
