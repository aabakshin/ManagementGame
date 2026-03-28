#ifndef LIST_HPP_SENTINEL
#define LIST_HPP_SENTINEL


template <class U>
class Item
{
private:
	U data;
	Item* next;
	Item* prev;
public:
	Item( U value );
	void SetData( U value );
	const U& GetData() const { return data; }
	Item* GetNext() const { return next; }
	Item* GetPrev() const { return prev; }
	void SetNext( Item* value ) { next = value; }
	void SetPrev( Item* value ) { prev = value; }
private:
	Item( const Item& ) = delete;
	Item( Item&& ) = delete;
	void operator=( const Item& ) = delete;
};

template <class T>
class List
{
private:
	T* first;
	T* last;
public:
	List() { first = nullptr; last = nullptr; }
	~List() { Clear(); }
	T* GetFirst() const { return first; }
	T* GetLast() const { return last; }
	bool IsEmpty() const { if ( !first && !last ) return true; return false; }
	void Insert( int num_value, T value ) { return; }
	void Delete( int num_value ) { return; }
	void Clear() { return; }
	void GetSize() const { return; }
	void Print() const { return; }
private:
	List( const List& ) = delete;
	List( List&& ) = delete;
	void operator=( const List& ) = delete;
	void SetFirst( T* value ) { first = value; }
	void SetLast( T* value ) { last = value; }
};

#endif
