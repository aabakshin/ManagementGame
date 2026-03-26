#ifndef PLAYER_HPP
#define PLAYER_HPP


enum
{
		TURNS_TO_BUILD		=		 5,
		MAX_PLAYERS			=		 8
};


class Player
{
private:

	enum
	{
			ADDRESS_SIZE		=		50,
			BUFSIZE				=	  1024
	};

public:
	class BuildsList
	{
	public:
		class BuildsItem
		{
		public:
			class BuildsData
			{
				int build_number;
				int turns_left;
			public:
				BuildsData( int num_value = 0, int turns_left_value = 0 );
				int GetBuildNumber() const { return build_number; }
				int GetTurnsLeft() const { return turns_left; }
				void SetBuildNumber( int num_value );
				void SetTurnsLeft( int turns_left_value );
			private:
				BuildsData( const BuildsData& ) {}
				void operator=( const BuildsData& ) {}
			};
		private:
			BuildsData data;
			BuildsItem* next;
			BuildsItem* prev;
		public:
			BuildsItem( int num_value, int turns_left_value );
			void SetData( int num_value, int turns_left_value );
			const BuildsData& GetData() const { return data; }
			BuildsItem* GetNext() const { return next; }
			BuildsItem* GetPrev() const { return prev; }
			void SetNext( BuildsItem* next_value ) { next = next_value; }
			void SetPrev( BuildsItem* prev_value ) { prev = prev_value; }
		private:
			BuildsItem( const BuildsItem& ) {}
			void operator=( const BuildsItem& ) {}
		};
	private:
		BuildsItem* first;
		BuildsItem* last;
	public:
		BuildsList() { first = nullptr; last = nullptr; }
		~BuildsList() { Clear(); }
		BuildsItem* GetFirst() const { return first; }
		BuildsItem* GetLast() const { return last; }
		bool IsEmpty() const { if ( !first && !last ) return true; return false; }
		int Insert( int num_value, int turns_left_value );
		int Delete( int build_num );
		int Clear();
		int GetSize() const;
		void Print() const;
		int GetValidNum() const { return GetMaxNum() + 1; }
	private:
		BuildsList( const BuildsList& ) {}
		void operator=( const BuildsList& ) {}
		void SetFirst( BuildsItem* first_value ) { first = first_value; }
		void SetLast( BuildsItem* last_value ) { last = last_value; }
		int GetMaxNum() const;
	};

	class AuctionReport
	{
	private:
		int sold_sources;
		int sold_price;
		int bought_products;
		int bought_price;
	public:
		AuctionReport();
		int GetSoldSources() const { return sold_sources; }
		int GetSoldPrice() const { return sold_price; }
		int GetBoughtProducts() const { return bought_products; }
		int GetBoughtPrice() const { return bought_price; }
		void SetSoldSources( int src_value );
		void SetSoldPrice( int price_value );
		void SetBoughtProducts( int prods_value );
		void SetBoughtPrice( int price_value );
	private:
		AuctionReport(const AuctionReport&) {}
		void operator=(const AuctionReport&) {}
	};
private:
	int fd;
	char addr[ADDRESS_SIZE];
	char message[BUFSIZE];
	int uid;
	int money;
	int old_money;
	int income;
	int sources;
	int products;
	int wait_factories;
	int work_factories;
	int built_factories;
	int produced_on_turn;
	bool is_free;
	bool is_bot;
	bool is_ident_message_recv;
	bool is_turn;
	//bool is_prod;
	bool is_bankrot;
	bool sent_source_request;
	bool sent_products_request;
	AuctionReport ar;
	BuildsList builds_factories;
public:
	Player( int p_fd, const char* p_addr, int p_uid );
	int GetFd() const { return fd; }
	const char* GetAddr() const { return addr; }
	const char* GetMessageBuffer() const { return message; }
	int GetUID() const { return uid; }
	int GetMoney() const { return money; }
	int GetOldMoney() const { return old_money; }
	int GetIncome() const { return income; }
	int GetSources() const { return sources; }
	int GetProducts() const { return products; }
	int GetWaitFactories() const { return wait_factories; }
	int GetWorkFactories() const { return work_factories; }
	int GetBuiltFactories() const { return built_factories; }
	int GetProduced() const { return produced_on_turn; }
	const AuctionReport& GetAuctionReport() const { return ar; }
	const BuildsList& GetBuildsFactories() const { return builds_factories; }

	void SetMessageBuffer( const char*, int );
	void SetMoney( int money_value );
	void SetOldMoney( int money_value );
	void SetIncome( int income_value );
	void SetSources( int sources_value );
	void SetProducts( int products_value );
	void SetWaitFactories( int wait_factories_value );
	void SetWorkFactories( int work_factories_value );
	void SetBuiltFactories( int built_factories_value );
	void SetProduced( int produced_value );
	void SetFree() { is_free = true; }
	void UnsetFree() { is_free = false; }
	void SetBot() { is_bot = true; }
	void UnsetBot() { is_bot = false; }
	void SetIdentMsgRecv() { is_ident_message_recv = true; }
	void UnsetIdentMsgRecv() { is_ident_message_recv = false; }
	void SetTurn() { is_turn = true; }
	void UnsetTurn() { is_turn = false; }
	//void SetProd() { is_prod = true; }
	//void UnsetProd() { is_prod = false; }
	void SetBankrot() { is_bankrot = true; }
	void UnsetBankrot() { is_bankrot = false; }
	void SetSentSourceRequest() { sent_source_request = true; }
	void UnsetSentSourceRequest() { sent_source_request = false; }
	void SetSentProductsRequest() { sent_products_request = true; }
	void UnsetSentProductsRequest() { sent_products_request = false; }

	bool IsFree() const { return is_free; }
	bool IsBot() const { return is_bot; }
	bool IsIdentMsgRecv() const { return is_ident_message_recv; }
	bool IsTurn() const { return is_turn; }
	//bool IsProd() const { return is_prod; }
	bool IsBankrot() const { return is_bankrot; }
	bool IsSentSourceRequest() const { return sent_source_request; }
	bool IsSentProductsRequest() const { return sent_products_request; }
private:
	Player() {}
	Player( const Player& ) {}
	void operator=( const Player& ) {}
	void SetFd( int p_fd );
	void SetAddr( const char* p_addr );
	void SetUID( int p_uid );
};

#endif
