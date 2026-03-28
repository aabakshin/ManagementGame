#ifndef SERVER_CORE_CPP
#define SERVER_CORE_CPP


#include "Server.hpp"
#include "Banker.hpp"
#include "List.hpp"
#include "MGLib.h"
#include "Player.hpp"
#include <signal.h>
#include <errno.h>
#include <unistd.h>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <sys/socket.h>
#include <netdb.h>
#include <ctime>


enum
{
					LISTEN_QUEUE_LEN			=						  5
};


// Описаны в модуле Banker
extern const double amount_multiplier_table[MARKET_LEVEL_NUMBER][2];
extern const int price_table[MARKET_LEVEL_NUMBER][2];
extern const int states_market_chance[MARKET_LEVEL_NUMBER][MARKET_LEVEL_NUMBER];

// Описаны в модуле MGLib
extern const char* info_game_messages[];
extern const char* error_game_messages[];

/* Список сообщений для идентификации бот-клиента */
static const char* const bot_identity_messages[] = {
				"./bot_mg_debug4",
				"./bot_mg_debug4\n",
				"./bot_mg_release4",
				"./bot_mg_release4\n",
				nullptr
};

/* Ф-я-обработчик сигнала SIGALRM */
void alrm_handler( int sig_no )
{
	int save_errno = errno;
	signal(SIGALRM, alrm_handler);

	Server::SetSignalNum( sig_no );
	Server::SetAlrmFlag();
	alarm(0);

	errno = save_errno;
}

/* Ф-я-обработчик сигнала SIGINT */
void exit_handler( int sig_no )
{
	int save_errno = errno;
	signal(SIGINT, exit_handler);

	Server::SetSignalNum( sig_no );
	Server::SetExitFlag();

	errno = save_errno;
}


void Server::MessageTokens::NullifyMessageTokens()
{
	for ( int i = 0; i < max_msg_tokens_count; ++i )
	{
		if ( message_tokens[i] != nullptr )
		{
			delete[] message_tokens[i];
			message_tokens[i] = nullptr;
		}
	}
}

void Server::MessageTokens::MakeMessageTokens( int tokens_count )
{
	message_tokens = new const char*[tokens_count];

	max_msg_tokens_count = tokens_count;
	msg_tokens_count = tokens_count;

	for ( int i = 0; i < max_msg_tokens_count; ++i )
	{
		message_tokens[i] = new char[MESSAGE_TOKEN_SIZE];
		memset(const_cast<char*>(message_tokens[i]), 0, MESSAGE_TOKEN_SIZE);
	}
}

const char*& Server::MessageTokens::operator[]( int idx )
{
	if ( ( idx < 0 ) || ( idx > max_msg_tokens_count-1 ) )
	{
		return message_tokens[0];
		//throw IncorrectMsgTokensIdxException();
	}

	return message_tokens[idx];
}

void Server::MessageTokens::SetMsgTokensCount( int tokens_value )
{
	if ( ( tokens_value < 1 ) || ( tokens_value > max_msg_tokens_count ) )
	{
		return;
		// throw IncorrectMsgTokensValueException();
	}

	msg_tokens_count = tokens_value;
}

Server::MessageTokens::~MessageTokens()
{
	NullifyMessageTokens();

	delete[] message_tokens;
}


void Server::Sender::ShowSendingMessage() const
{
	Server::SetSentMsgsCount( Server::GetSentMsgsCount() + 1 );
	printf("\n==================== (%d) ====================\n", Server::GetSentMsgsCount());

	for ( int i = 0; ( i < BUFSIZE ) && ( i < message_length ); ++i )
	{
		printf("%3d ", message[i]);
		if ( ( (i+1) % 10 ) == 0 )
			putchar('\n');
	}
	putchar('\n');

	printf("\nmessage: <[ %s ]>\n"
			"Sent to [%s] %d\\%d bytes\n"
			"==================== (%d) ====================\n\n", message, target_address, sent_bytes, message_length, Server::GetSentMsgsCount());
}

void Server::Sender::SendMessage( const char* const* message_tokens, int tokens_count, int cs, const char* address )
{
	bool overflow = false;

	Reset();

	for ( int j = 0; j < tokens_count; ++j )
	{
		for ( int k = 0; ( cur_pos < BUFSIZE-2 ) && message_tokens[j][k]; ++k, ++cur_pos )
			message[cur_pos] = message_tokens[j][k];

		if ( cur_pos >= BUFSIZE-2 )
		{
			message[BUFSIZE-2] = '\n';
			message[BUFSIZE-1] = '\0';
			message_length = BUFSIZE;
			overflow = true;
			break;
		}

		message[cur_pos] = '|';
		++cur_pos;
	}

	if ( !overflow )
	{
		message[cur_pos] = '\n';
		message[cur_pos+1] = '\0';
		message_length = cur_pos + 2;
	}

	target_socket = cs;

	strncpy(target_address, address, ADDRESS_SIZE-1);

	sent_bytes = send( target_socket, message, message_length, 0 );

	if ( sent_bytes < 0 )
	{
		return;
		// throw UnableSendDataException();
	}

	ShowSendingMessage();
}

void Server::Sender::SendMessage( const char* msg, int cs, const char* address )
{
	Reset();

	for ( ; ( cur_pos < BUFSIZE-2 ) && msg[cur_pos]; ++cur_pos )
		message[cur_pos] = msg[cur_pos];

	message[cur_pos] = '\n';
	message[cur_pos+1] = '\0';
	message_length = cur_pos + 2;

	target_socket = cs;

	strncpy(target_address, address, ADDRESS_SIZE-1);

	sent_bytes = send( target_socket, message, message_length, 0 );

	if ( !IsSentMessage() )
	{
		return;
		// throw UnableSendDataException();
	}

	ShowSendingMessage();
}

bool Server::Sender::IsSentMessage() const
{
	if ( ( sent_bytes >= 0 ) && ( sent_bytes <= BUFSIZE-1 ) )
		return true;

	return false;
}

void Server::Sender::Reset()
{
	ResetMessage();
	ResetMessageLength();
	ResetCurPos();
	ResetTargetSocket();
	ResetTargetAddress();
	ResetSentBytes();
}

void Server::Sender::ResetMessage()
{
	memset(message, 0x00, BUFSIZE);
}

void Server::Sender::ResetMessageLength()
{
	message_length = 0;
}

void Server::Sender::ResetCurPos()
{
	cur_pos = 0;
}

void Server::Sender::ResetTargetSocket()
{
	target_socket = -1;
}

void Server::Sender::ResetTargetAddress()
{
	memset(target_address, 0x00, ADDRESS_SIZE);
}

void Server::Sender::ResetSentBytes()
{
	sent_bytes = 0;
}


void Server::Receiver::RecvMessage( int cs, const char* address )
{
	Reset();

	target_socket = cs;

	strncpy(target_address, address, ADDRESS_SIZE );

	recv_bytes = readline( target_socket, message, BUFSIZE-1 );

	if ( !IsRecvMessage() )
	{
		return;
		// throw UnableRecvDataException();
	}

	message[recv_bytes] = '\0';
	cut_str(message, recv_bytes, '\n');

	int message_size = strlen(message) + 1;
	delete_spaces(message, &message_size);

	message_length = message_size - 1;

	ShowReceivedMessage();
}

bool Server::Receiver::IsRecvMessage() const
{
	if ( ( recv_bytes >= 0 ) && ( recv_bytes <= BUFSIZE-1 ) )
		return true;

	return false;
}

void Server::Receiver::Reset()
{
	ResetMessage();
	ResetMessageLength();
	ResetCurPos();
	ResetTargetSocket();
	ResetTargetAddress();
	ResetSentBytes();
}

void Server::Receiver::ResetMessage()
{
	memset(message, 0x00, BUFSIZE);
}

void Server::Receiver::ResetMessageLength()
{
	message_length = 0;
}

void Server::Receiver::ResetCurPos()
{
	cur_pos = 0;
}

void Server::Receiver::ResetTargetSocket()
{
	target_socket = -1;
}

void Server::Receiver::ResetTargetAddress()
{
	memset(target_address, 0x00, ADDRESS_SIZE);
}

void Server::Receiver::ResetSentBytes()
{
	recv_bytes = 0;
}

void Server::Receiver::ShowReceivedMessage() const
{
	Server::SetRecvMsgsCount( Server::GetRecvMsgsCount() + 1 );
	printf("\n==================== (%d) ====================\n", Server::GetRecvMsgsCount());

	for ( int i = 0; ( i < BUFSIZE ) && ( i < message_length ); ++i )
	{
		printf("%3d ", message[i]);
		if ( ( (i+1) % 10 ) == 0 )
			putchar('\n');
	}
	putchar('\n');

	printf("\nmessage: <[ %s ]>\n"
			"Received from [%s] %d\\%d bytes\n"
			"==================== (%d) ====================\n\n", message, target_address, recv_bytes, message_length, Server::GetRecvMsgsCount());
}

template <class T>
void Server::EncapsulatedBrokerMessages<T>::MakeBroker( const Banker& game_session )
{
	brokerPTR = new T( game_session );
}

template <class T>
const T& Server::EncapsulatedBrokerMessages<T>::GetBroker() const
{
	return const_cast<const T&>(*brokerPTR);
}

template <class T>
Server::EncapsulatedBrokerMessages<T>::~EncapsulatedBrokerMessages()
{
	delete brokerPTR;
}


void Server::SetSignalNum( int value )
{
	if ( value < 0 )
	{
		return;
		// throw InvalidValueException();
	}

	Server::sig_number = value;
}

void Server::SetSentMsgsCount( int msgs_value )
{
	if ( msgs_value < 0 )
	{
		return;
		// throw InvalidValueException();
	}

	sent_msgs_count = msgs_value;
}

void Server::SetRecvMsgsCount( int msgs_value )
{
	if ( msgs_value < 0 )
	{
		return;
		// throw InvalidValueException();
	}

	recv_msgs_count = msgs_value;
}

void Server::SetListenSocket( int socket_value )
{
	if ( socket_value < -1 )
	{
		return;
		// throw InvalidValueException();
	}

	ls = socket_value;
}

void Server::SetAddrBuffer( const char* addr, const char* port )
{
	printf("%s\n", "Configuring local address...");

	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ( getaddrinfo(addr, port, &hints, &GetBindAddress()) != 0 )
	{
		fprintf(stderr, "An error has occured with \"getaddrinfo\" (errno code = %d)\n", errno);
		return;
		// throw InvalidGetAddrInfoException();
	}

	getnameinfo(
			GetBindAddress()->ai_addr,
			GetBindAddress()->ai_addrlen,
			address_buffer,
			sizeof(address_buffer),
			service_buffer,
			sizeof(service_buffer),
			NI_NUMERICHOST | NI_NUMERICSERV );

	ConcatAddrPort();
}

void Server::SetMaxFd( int max_value )
{
	if ( max_value < 0 )
	{
		return;
		// throw InvalidValueException();
	}

	max_fd = max_value;
}

void Server::ListenSocketInit()
{
	printf("%s\n", "Creating listening socket...");
	SetListenSocket( socket(GetBindAddress()->ai_family, GetBindAddress()->ai_socktype, GetBindAddress()->ai_protocol) );
	if ( GetListenSocket() == -1 )
	{
		fprintf(stderr, "socket() failed. (errno code = %d)\n", errno);
		return;
		// throw InvalidSocketException();
	}

	printf("%s\n", "Setting socket options...");
	int opt_value = 1;
	setsockopt(GetListenSocket(), SOL_SOCKET, SO_REUSEADDR, &opt_value, sizeof(opt_value));

	printf("%s\n", "Binding socket to address...");
	if ( bind(GetListenSocket(), GetBindAddress()->ai_addr, GetBindAddress()->ai_addrlen) )
	{
		fprintf(stderr, "bind() failed. (errno code = %d)\n", errno);
		return;
		// throw BindSocketException();
	}

	printf("%s\n", "Enabling listen mode...");
	if ( listen(GetListenSocket(), LISTEN_QUEUE_LEN) < 0 )
	{
		fprintf(stderr, "listen() failed. (errno code = %d\n)", errno);
		return;
		// throw ListenSocketException();
	}
}

Server::Server( const char* addr, const char* port )
{
	EBCbroker.MakeBroker( GetBanker() );
	msg_tokens.MakeMessageTokens( MESSAGE_TOKENS_COUNT );

	UnsetAlrmFlag();
	UnsetExitFlag();
	UnsetTimerFlag();
	SetSignalNum( 0 );
	SetSentMsgsCount( 0 );
	SetAddrBuffer( addr, port );
	ListenSocketInit();
	SetMaxFd( 0 );

	printf("Waiting connections to %s port...\n", port);
}

Server::~Server()
{
	freeaddrinfo(GetBindAddress());
}


bool Server::IsCorrectIdentityMsg( const char* identity_msg )
{
	for ( int i = 0; bot_identity_messages[i] != nullptr; ++i )
		if ( strcmp(identity_msg, bot_identity_messages[i]) == 0 )
			return true;

	return false;
}

void Server::ConcatAddrPort()
{
	int addr_len = strlen(address_buffer);
	address_buffer[addr_len] = ':';

	int i = addr_len + 1;
	for ( int j = 0; ( j < SERVICE_SIZE-1 ) && service_buffer[j]; ++j, ++i )
		address_buffer[i] = service_buffer[j];
	address_buffer[i] = '\0';
}


void Server::CloseConnection( int player_number )
{
	int p_fd = GetBanker().GetPlayers().GetPlayerByUID(player_number)->GetFd();

	char ip[100] = { 0 };
	strcpy(ip, GetBanker().GetPlayers().GetPlayerByUID(player_number)->GetAddr());

	const_cast<Banker&>(GetBanker()).CleanPlayer(player_number);
	close(p_fd);
	FD_CLR(p_fd, &readfds);
	printf("[-] Lost connection from [%s]\n", ip);
}

void Server::Stop( int forcely )
{
	if ( forcely )
		printf("%s", "\n\n========== SERVER IS STOPPING WORK FORCELY ==========\n");

	for ( int i = 0; i < MAX_PLAYERS; ++i )
		CloseConnection( GetBanker().GetPlayers().GetUIDByIdx( i ) );

	if ( forcely )
		printf("%s", "========== SERVER IS STOPPING WORK FORCELY ==========\n\n");

	exit(0);
}

void Server::PayCharges()
{
	int total_charges = 0;

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = banker.GetPlayers()[i];

		if ( p->IsFree() )
		{
			return;
			// throw PlayerRecordIsFreeException();
		}

		total_charges = p->GetSources() * SOURCE_UNIT_CHARGE;
		total_charges += p->GetProducts() * PRODUCT_UNIT_CHARGE;
		total_charges += p->GetWaitFactories() * FACTORY_UNIT_CHARGE;
		total_charges += p->GetWorkFactories() * FACTORY_UNIT_CHARGE;

		itoa( total_charges, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::TOTAL_CHARGES_PARAM_TOKEN]), MESSAGE_TOKEN_SIZE-1 );
		const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::TOTAL_CHARGES_PARAM_TOKEN+1 );

		int remains = p->GetMoney() - total_charges;
		if ( remains > 0 )
		{
			const_cast<Player*>(p)->SetMoney( remains );
			sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::SUCCESS_CHARGES_PAY_TOKEN ), p->GetFd(), p->GetAddr() );
		}
		else
		{
			sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::PLAYER_BANKROT_TOKEN ), p->GetFd(), p->GetAddr() );
			const_cast<Player*>(p)->SetBankrot();
		}
	}
}

void Server::ShowAuctionInfo( const char* auction_type_msg, const Item<MarketData>* node )
{
	printf("\n%s\n", auction_type_msg);

	for ( ; node != nullptr; node = node->GetNext() )
	{
		const Player* p = banker.GetPlayers().GetPlayerByUID(node->GetData().GetPlayerNum());
		printf("Request of Player #%d:\n", p->GetUID());
		printf("\tPrice: %d\n\tAmount: %d\n\tIs proceed: %s\n\n",
				node->GetData().GetPrice(),
				(node->GetData().IsSuccess()) ? p->GetAuctionReport().GetSoldSources() : node->GetData().GetAmount(),
				node->GetData().IsSuccess() ? "yes" : "no");
	}
}

void Server::ReportOnTurn()
{
	printf("\n\n\n<<<<<<<<<< Report on Month #%d >>>>>>>>>>\n", banker.GetTurnNumber());
	printf("\n%s\n", "Players statistics:");
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = banker.GetPlayers()[i];
		if ( !p->IsFree() )
			printf("\tPlayer #%d:   money: %dР   produced products: %d\n", p->GetUID(), p->GetMoney(), p->GetProduced());
	}

	ShowAuctionInfo("Sources auction", banker.GetSourcesRequests().GetFirst());
	ShowAuctionInfo("Products auction", banker.GetProductsRequests().GetFirst());

	printf("\n<<<<<<<<<< Report on Month #%d >>>>>>>>>>\n", banker.GetTurnNumber());
}

void Server::ChangeMarketState()
{
	int r = 1 + (int)( 12.0 * rand() / (RAND_MAX + 1.0) );

	int sum = 0;
	int i;
	for ( i = 0; i < MARKET_LEVEL_NUMBER; i++ )
	{
		sum += states_market_chance[banker.GetCurrentMarketLvl()-1][i];
		if ( sum >= r )
			break;
	}

	if ( i < MARKET_LEVEL_NUMBER )
		banker.SetCurrentMarketLvl( i+1 );

	banker.GetCurrentMarketState().SetSourcesAmount( amount_multiplier_table[banker.GetCurrentMarketLvl()-1][0] * banker.GetAlivePlayers() );
	banker.GetCurrentMarketState().SetSourceMinPrice( price_table[banker.GetCurrentMarketLvl()-1][0] );
	banker.GetCurrentMarketState().SetProductsAmount( amount_multiplier_table[banker.GetCurrentMarketLvl()-1][1] * banker.GetAlivePlayers() );
	banker.GetCurrentMarketState().SetProductMaxPrice( price_table[banker.GetCurrentMarketLvl()-1][1] );
}

bool Server::CheckPlayersReports( List<Item<MarketData>>& requests )
{
	// если список заявок на аукцион пуст, то не нужно его проводить
	if ( requests.IsEmpty() )
		return false;

	// Если игрок не заявился на аукцион, добавить его пустую заявку
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = banker.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			Item<MarketData>* node;
			for ( node = requests.GetFirst(); node != nullptr; node = node->GetNext() )
				if ( node->GetData().GetPlayerNum() == p->GetUID() )
					break;

			if ( node == nullptr )
			{
				MarketData data;
				data.MakeData( p->GetUID(), 0, 0 );
				requests.Insert( data );
			}
		}
	}

	return true;
}

void Server::SortRequestsByPrice( const List<Item<MarketData>>& requests, List<Item<MarketData>>& sorted_requests, int auction_type )
{
	const int ready_players = banker.GetReadyPlayers();

	Item<MarketData>* arr_reqs[ready_players];
	int prices[ready_players];
	bool reqs_checked[ready_players];


	Item<MarketData>* request = requests.GetFirst();
	for ( int i = 0; request != nullptr; request = request->GetNext(), ++i )
	{
		arr_reqs[i] = request;
		prices[i] = arr_reqs[i]->GetData().GetPrice();
		reqs_checked[i] = false;
	}

	heap_sort(prices, ready_players, ( auction_type == SOURCE_AUCTION ) ? 1 : 0 );

	int j = 0;
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		if ( (arr_reqs[i]->GetData().GetPrice() == prices[j]) && !reqs_checked[i] )
		{
			MarketData data;
			data.MakeData( arr_reqs[i]->GetData().GetPlayerNum(), arr_reqs[i]->GetData().GetAmount(), arr_reqs[i]->GetData().GetPrice() );

			sorted_requests.Insert( data );
			reqs_checked[i] = true;
			i = 0;
			++j;
			if ( j == ready_players )
				break;

			continue;
		}
	}
}

void Server::StartAuction( const List<Item<MarketData>>& requests, int auction_type )
{
	if ( !CheckPlayersReports( const_cast<List<Item<MarketData>>&>(requests) ) )
		return;

	List<Item<MarketData>> sorted_requests;
	SortRequestsByPrice( requests, sorted_requests, auction_type );

	int max_sources = banker.GetCurrentMarketState().GetSourcesAmount();
	int max_products = banker.GetCurrentMarketState().GetProductsAmount();

	for ( Item<MarketData>* node = sorted_requests.GetFirst(); node != nullptr; node = node->GetNext() )
	{
		if ( node->GetData().GetPrice() < 1 )
			continue;

		const Player* cur_p = banker.GetPlayers().GetPlayerByUID( node->GetData().GetPlayerNum() );

		if ( cur_p->IsFree() )
			continue;

		if ( node->GetData().GetAmount() <= ( ( auction_type == SOURCE_AUCTION ) ? max_sources : max_products ) )
		{
			if ( node->GetData().GetAmount() > 0 )
			{
				if ( auction_type == SOURCE_AUCTION )
				{
					const_cast<Player*>(cur_p)->SetMoney( cur_p->GetMoney() - node->GetData().GetAmount() * node->GetData().GetPrice() );
					const_cast<Player*>(cur_p)->SetSources( cur_p->GetSources() + node->GetData().GetAmount() );
					max_sources -= node->GetData().GetAmount();
				}
				else
				{
					const_cast<Player*>(cur_p)->SetMoney( cur_p->GetMoney() + node->GetData().GetAmount() * node->GetData().GetPrice() );
					const_cast<Player*>(cur_p)->SetProducts( cur_p->GetProducts() - node->GetData().GetAmount() );
					max_products -= node->GetData().GetAmount();
				}

				const_cast<MarketData&>(node->GetData()).SetSuccess();

				if ( auction_type == SOURCE_AUCTION )
				{
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetSoldSources(node->GetData().GetAmount());
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetSoldPrice(node->GetData().GetPrice());
				}
				else
				{
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetBoughtProducts(node->GetData().GetAmount());
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetBoughtPrice(node->GetData().GetPrice());
				}
			}
		}
		else
		{
			int saved_max_sources = 0;
			int saved_max_products = 0;

			if ( ( ( auction_type == SOURCE_AUCTION ) ? max_sources : max_products) > 0 )
			{
				if ( auction_type == SOURCE_AUCTION )
				{
					const_cast<Player*>(cur_p)->SetMoney( cur_p->GetMoney() - max_sources * node->GetData().GetPrice() );
					const_cast<Player*>(cur_p)->SetSources( cur_p->GetSources() + max_sources );
				}
				else
				{
					const_cast<Player*>(cur_p)->SetMoney( cur_p->GetMoney() + max_products * node->GetData().GetPrice() );
					const_cast<Player*>(cur_p)->SetProducts( cur_p->GetProducts() - max_products );
				}

				const_cast<MarketData&>(node->GetData()).SetSuccess();

				if ( auction_type == SOURCE_AUCTION )
				{
					saved_max_sources = max_sources;
					max_sources = 0;
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetSoldSources(saved_max_sources);
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetSoldPrice(node->GetData().GetPrice());
				}
				else
				{
					saved_max_products = max_products;
					max_products = 0;
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetBoughtProducts(saved_max_products);
					const_cast<Player::AuctionReport&>(cur_p->GetAuctionReport()).SetBoughtPrice(node->GetData().GetPrice());
				}
			}
		}
	}
}

void Server::CheckBuildingFactories()
{
	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = banker.GetPlayers()[i];
		if ( !p->IsFree() )
		{
			for ( Item<BuildsData>* node = p->GetBuildsFactories().GetFirst(); node != nullptr; )
			{
				if ( node->GetData().GetTurnsLeft() == 1 )
				{
					int total_charges = NEW_FACTORY_UNIT_COST / 2;
					const_cast<Player*>(p)->SetMoney( p->GetMoney() - total_charges );
					if ( p->GetMoney() > 0 )
					{
						sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::PAY_FACTORY_SUCCESS_TOKEN ), p->GetFd(), p->GetAddr() );
					}
					else
					{
						itoa( total_charges, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::TOTAL_CHARGES_PARAM_TOKEN]), MESSAGE_TOKEN_SIZE-1 );
						const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::TOTAL_CHARGES_PARAM_TOKEN+1 );

						sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::PLAYER_BANKROT_TOKEN ), p->GetFd(), p->GetAddr() );
						const_cast<Player*>(p)->SetBankrot();
						break;
					}
				}
				else if ( node->GetData().GetTurnsLeft() == 0 )
				{
					const_cast<List<Item<BuildsData>>&>(p->GetBuildsFactories()).Delete( node->GetData().GetBuildNumber() );
					const_cast<Player*>(p)->SetBuiltFactories( p->GetBuiltFactories() - 1 );
					const_cast<Player*>(p)->SetWaitFactories( p->GetWaitFactories() + 1 );

					sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::FACTORY_BUILT_TOKEN ), p->GetFd(), p->GetAddr() );
					node = p->GetBuildsFactories().GetFirst();
					continue;
				}

				const_cast<BuildsData&>(node->GetData()).SetTurnsLeft( node->GetData().GetTurnsLeft() - 1 );
				node = node->GetNext();
			}
		}
	}
}

void Server::QuitPlayer( int player_number )
{
	CloseConnection( player_number );

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = GetBanker().GetPlayers()[i];
		if ( !p->IsFree() )
		{
			if ( !GetBanker().IsGameStarted() )
			{
				sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::LOST_LOBBY_PLAYER_TOKEN ), p->GetFd(), p->GetAddr() );
			}
			else
			{
				if ( GetBanker().GetAlivePlayers() <= 1 )
				{
					sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::VICTORY_MESSAGE_TOKEN ), p->GetFd(), p->GetAddr() );
					printf("\n\n<<<<< GAME IS FINISHED. PLAYER #%d IS WINNER! >>>>>\n\n", p->GetUID());
					Server::Stop( 0 );
				}
				else
				{
					itoa( player_number, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::LEFT_PLAYER_ID_PARAM_TOKEN]), MESSAGE_TOKEN_SIZE-1 );
					const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::LEFT_PLAYER_ID_PARAM_TOKEN+1 );

					sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::LOST_ALIVE_PLAYER_TOKEN ), p->GetFd(), p->GetAddr() );
				}
			}
		}
	}
}

void Server::FillReadfds()
{
	FD_ZERO(&readfds);

	FD_SET(ls, &readfds);

	max_fd = ls;

	for ( int i = 0; i < MAX_PLAYERS; ++i )
	{
		const Player* p = GetBanker().GetPlayers()[i];
		if ( !p->IsFree() )
		{
			int p_fd = p->GetFd();

			FD_SET( p_fd, &readfds );
			if ( p_fd > max_fd )
				max_fd = p_fd;
		}
	}
}

int Server::Run()
{
	signal(SIGINT, exit_handler);
	signal(SIGALRM, alrm_handler);

	srand(time(0));


	while ( 1 )
	{
		if ( exit_flag )
			Server::Stop( 1 );

		Server::FillReadfds();

		if ( !GetBanker().IsGameStarted() )
		{
			if ( !IsTimerFlag() && ( GetBanker().GetLobbyPlayers() >= MIN_PLAYERS_TO_START ) && ( GetBanker().GetLobbyPlayers() < MAX_PLAYERS ) )
			{
				alarm(TIME_TO_START);

				SetTimerFlag();

				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					const Player* p = GetBanker().GetPlayers()[i];
					if ( !p->IsFree() )
					{
						itoa( TIME_TO_START, const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::TIME_TO_START_PARAM_TOKEN]), MESSAGE_TOKEN_SIZE-1 );
						const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::TIME_TO_START_PARAM_TOKEN+1 );

						sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::STARTINSECONDS_TOKEN ), p->GetFd(), p->GetAddr() );
					}
				}
			}
			else if ( GetBanker().GetLobbyPlayers() == MAX_PLAYERS )
			{
				alarm(0);
				UnsetAlrmFlag();
				UnsetTimerFlag();

				/*FORCE START THE GAME*/
				const_cast<Banker&>(GetBanker()).SetGameStarted();
				/*FORCE START THE GAME*/

				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					const Player* p = GetBanker().GetPlayers()[i];
					if ( !p->IsFree() )
					{
						sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::GAME_STARTED_TOKEN ), p->GetFd(), p->GetAddr() );
					}
				}
				continue;
			}
		}
		else
		{
			if ( !GetBanker().IsPlayersPrepared() )
			{
				const_cast<Banker&>(GetBanker()).SetAlivePlayers(GetBanker().GetLobbyPlayers());
				const_cast<Banker&>(GetBanker()).SetLobbyPlayers( 0 );
				const_cast<Banker&>(GetBanker()).SetTurnNumber( 1 );
				const_cast<Banker&>(GetBanker()).SetCurrentMarketLvl( START_MARKET_LEVEL );
				const_cast<Banker&>(GetBanker()).GetCurrentMarketState().SetSourcesAmount( amount_multiplier_table[START_MARKET_LEVEL-1][0] * GetBanker().GetAlivePlayers() );
				const_cast<Banker&>(GetBanker()).GetCurrentMarketState().SetSourceMinPrice( price_table[START_MARKET_LEVEL-1][0] );
				const_cast<Banker&>(GetBanker()).GetCurrentMarketState().SetProductsAmount( amount_multiplier_table[START_MARKET_LEVEL-1][1] * GetBanker().GetAlivePlayers() );
				const_cast<Banker&>(GetBanker()).GetCurrentMarketState().SetProductMaxPrice( price_table[START_MARKET_LEVEL-1][1] );

				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					const Player* p = GetBanker().GetPlayers()[i];
					if ( !p->IsFree() )
					{
						const_cast<Player*>(p)->SetMoney( START_MONEY );
						const_cast<Player*>(p)->SetOldMoney( p->GetMoney() );
						const_cast<Player*>(p)->SetSources( START_SOURCES );
						const_cast<Player*>(p)->SetProducts( START_PRODUCTS );
						const_cast<Player*>(p)->SetWaitFactories( START_FACTORIES );

						itoa( p->GetUID(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::SENDER_ID_PARAM_TOKEN]), MESSAGE_TOKEN_SIZE-1 );
						const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::SENDER_ID_PARAM_TOKEN+1 );

						sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::STARTING_GAME_INFORMATION_TOKEN ), p->GetFd(), p->GetAddr() );
					}
				}

				const_cast<Banker&>(GetBanker()).SetPlayersPrepared();
			}
		}


		int res = select(max_fd+1, &readfds, nullptr, nullptr, nullptr);
		if ( res == -1 )
		{
			if ( errno == EINTR )
			{
				if ( ( sig_number == SIGALRM ) && alrm_flag && ( !GetBanker().IsGameStarted() ) )
				{
					UnsetAlrmFlag();
					UnsetTimerFlag();

					if ( GetBanker().GetLobbyPlayers() < MIN_PLAYERS_TO_START )
					{
						for ( int i = 0; i < MAX_PLAYERS; ++i )
						{
							const Player* p = GetBanker().GetPlayers()[i];
							if ( !p->IsFree() )
							{
								sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::STARTCANCELLED_TOKEN ), p->GetFd(), p->GetAddr() );
							}
						}
					}
					else
					{
						if ( GetBanker().GetLobbyPlayers() < MAX_PLAYERS )
						{
							/* START THE GAME */
							const_cast<Banker&>(GetBanker()).SetGameStarted();
							/* START THE GAME */

							for ( int i = 0; i < MAX_PLAYERS; ++i )
							{
								const Player* p = GetBanker().GetPlayers()[i];
								if ( !p->IsFree() )
								{
									sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::GAME_STARTED_TOKEN ), p->GetFd(), p->GetAddr() );
								}
							}
						}
						continue;
					}
				}
				fprintf(stderr, "\nGot some signal (#%d).\n", sig_number);
			}
			else
			{
				fprintf(stderr, "\nselect() failed. (errno code = %d)\n", errno);
			}
			continue;
		}


		if ( FD_ISSET(ls, &readfds) )
		{
			char new_client_addr[Server::ADDRESS_SIZE];
			char new_client_serv[Server::SERVICE_SIZE];
			struct sockaddr_storage client_address;
			socklen_t client_address_len = sizeof(client_address);

			int cs = accept(ls, (struct sockaddr*) &client_address, &client_address_len);
			if ( cs == -1 )
			{
				fprintf(stderr, "accept() failed. (errno code = %d)\n", errno);
				return 1;
			}

			getnameinfo(
					(struct sockaddr*) &client_address,
					client_address_len,
					new_client_addr,
					sizeof(new_client_addr),
					new_client_serv,
					sizeof(new_client_serv),
					NI_NUMERICHOST | NI_NUMERICSERV);

			ConcatAddrPort();

			printf("New connection from (%s)\n", new_client_addr);

			if ( GetBanker().IsGameStarted() )
			{
				sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::GAME_ALREADY_STARTED_TOKEN ), cs, new_client_addr );
				close(cs);
				printf("Lost connection from (%s)\n", new_client_addr);
			}
			else
			{
				const Player* new_player = nullptr;
				int i;
				for ( i = 0; i < MAX_PLAYERS; ++i )
				{
					new_player = GetBanker().GetPlayers()[i];
					if ( new_player->IsFree() )
						break;
				}

				if ( i >= MAX_PLAYERS )
				{
					sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::SERVER_FULL_TOKEN ), cs, new_client_addr );
					close(cs);
					printf("Lost connection from (%s)\n", new_client_addr);
				}
				else
				{
					try
					{
						const_cast<Player*>(new_player)->SetNewPlayer( cs, address_buffer );
					}
					catch ( ... )
					{
						sender.SendMessage( error_game_messages[INTERNAL_SERVER_ERROR], cs, new_client_addr );
						close(cs);
						printf("Lost connection from [%s]\n", new_client_addr);
						continue;
					}

					const_cast<Banker&>(GetBanker()).SetLobbyPlayers( GetBanker().GetLobbyPlayers() + 1 );

					for ( i = 0; i < MAX_PLAYERS; ++i )
					{
						const Player* p = GetBanker().GetPlayers()[i];
						if ( !p->IsFree() )
						{
							sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::NEW_PLAYER_CONNECT_TOKEN ), cs, new_client_addr );
						}
					}
				}
			}
		}

		for ( int i = 0; i < MAX_PLAYERS; ++i )
		{
			const Player* sender_p = GetBanker().GetPlayers()[i];
			if ( !sender_p->IsFree() )
			{
				int sender_p_fd = sender_p->GetFd();
				const char* sender_p_addr = sender_p->GetAddr();

				if ( FD_ISSET(sender_p_fd, &readfds) )
				{
					receiver.RecvMessage( sender_p_fd, sender_p_addr );
					if ( receiver.GetRecvBytes() > 0 )
					{
						const_cast<Player*>(sender_p)->SetMessageBuffer( receiver.GetMessage(), receiver.GetMessageLength() );
						if ( !banker.IsGameStarted() )
						{
							if ( !sender_p->IsIdentMsgRecv() )
							{
								if ( IsCorrectIdentityMsg( sender_p->GetMessageBuffer() ) )
									const_cast<Player*>(sender_p)->SetBot();
								else
									const_cast<Player*>(sender_p)->UnsetBot();

								const_cast<Player*>(sender_p)->SetIdentMsgRecv();
							}
							else
							{
								sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::GAME_NOT_STARTED_TOKEN ), sender_p_fd, sender_p_addr );
							}
							continue;
						}

						// Обработка данных от игрока, когда игра началась
						const_cast<CommandExecutor&>(GetCmdsHndl()).MakeCmdTokens( sender_p->GetMessageBuffer() );
						const_cast<CommandExecutor&>(GetCmdsHndl()).ProcessCommand( sender_p->GetUID(), EBCbroker.GetBroker());
						sender.SendMessage( GetCmdsHndl().GetCmdResultTokens(), GetCmdsHndl().GetCmdResultTokensAmount(), sender_p_fd, sender_p_addr );

						const char* info_token = GetCmdsHndl().GetCmdToken( 0 );
						if ( strcmp(info_token, info_game_messages[QUIT_COMMAND_SUCCESS]) == 0 )
						{
							QuitPlayer( sender_p->GetUID() );
						}
						else if ( strcmp(info_token, error_game_messages[INTERNAL_SERVER_ERROR]) == 0 )
						{
							QuitPlayer( sender_p->GetUID() );
						}
					}
					else
					{
						QuitPlayer( sender_p->GetUID() );
					}
				}
			}
		}

		// Действия, происходящие в конце игрового месяца
		if ( GetBanker().IsGameStarted() )
		{
			if ( GetBanker().GetReadyPlayers() == GetBanker().GetAlivePlayers() )
			{
				StartAuction( const_cast<Banker&>(GetBanker()).GetSourcesRequests(), SOURCE_AUCTION );
				StartAuction( const_cast<Banker&>(GetBanker()).GetProductsRequests(), PRODUCTION_AUCTION );

				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					const Player* p = GetBanker().GetPlayers()[i];
					if ( !p->IsFree() )
					{
						sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::AUCTION_RESULTS_TOKEN ), p->GetFd(), p->GetAddr() );
					}
				}

				CheckBuildingFactories();
				PayCharges();
				//ReportOnTurn();

				ChangeMarketState();
				const_cast<Banker&>(GetBanker()).SetTurnNumber( GetBanker().GetTurnNumber() + 1 );
				const_cast<Banker&>(GetBanker()).SetReadyPlayers( 0 );

				for ( int i = 0; i < MAX_PLAYERS; ++i )
				{
					const Player* p = GetBanker().GetPlayers()[i];
					if ( !p->IsFree() )
					{
						const_cast<Player*>(p)->UnsetSentSourceRequest();
						const_cast<Player*>(p)->UnsetSentProductsRequest();
						const_cast<Player*>(p)->SetProduced( 0 );
						const_cast<Player*>(p)->UnsetTurn();
						const_cast<Player*>(p)->SetIncome( p->GetMoney() - p->GetOldMoney() );
						const_cast<Player*>(p)->SetOldMoney( p->GetMoney() );

						itoa( p->GetUID(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::SENDER_ID_PARAM_TOKEN]), MESSAGE_TOKEN_SIZE-1 );
						const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::SENDER_ID_PARAM_TOKEN+1 );

						sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::NEW_TURN_TOKEN ), p->GetFd(), p->GetAddr() );

						while ( p->GetWorkFactories() > 0 )
						{
							const_cast<Player*>(p)->SetProduced( p->GetProduced() + 1 );
							const_cast<Player*>(p)->SetProducts( p->GetProducts() + 1 );
							const_cast<Player*>(p)->SetWorkFactories( p->GetWorkFactories() - 1 );
							const_cast<Player*>(p)->SetWaitFactories( p->GetWaitFactories() + 1 );
						}

						if ( p->GetProduced() > 0 )
						{
							itoa( p->GetProduced(), const_cast<char*>(const_cast<MessageTokens&>(msg_tokens).GetValue()[GameMessages::PRODUCED_AMOUNT_PARAM_TOKEN]), MESSAGE_TOKEN_SIZE-1 );
							const_cast<GameMessages&>(EGameMessages.GetBroker()).PutMessage( msg_tokens.GetValue(), GameMessages::PRODUCED_AMOUNT_PARAM_TOKEN+1 );

							sender.SendMessage( const_cast<GameMessages&>(EGameMessages.GetBroker()).TakeMessage( GameMessages::PRODUCED_TOKEN ), p->GetFd(), p->GetAddr() );
						}
					}
				}
			}
		}
	} /* end while ( 1 ) */
}

#endif
