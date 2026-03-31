#ifndef MESSAGE_TOKENS_HPP_SENTINEL
#define MESSAGE_TOKENS_HPP_SENTINEL


class MessageTokens
{
public:

	enum
	{
				MESSAGE_TOKENS_COUNT			=			  5,
				MESSAGE_TOKEN_SIZE				=			100
	};

private:
	const char** message_tokens;
	int max_msg_tokens_count;
	int msg_tokens_count;
public:
	MessageTokens() {}
	void MakeMessageTokens( int );
	const char*& operator[]( int );
	const char** GetValue() const { return message_tokens; }
	int GetMsgTokensCount() const { return msg_tokens_count; }
	void SetMsgTokensCount( int );
	~MessageTokens();
private:
	MessageTokens( const MessageTokens& ) = delete;
	MessageTokens( MessageTokens&& ) = delete;
	void operator=( const MessageTokens& ) = delete;
	void NullifyMessageTokens();
};

#endif
