/*
 *	Модуль PolizElem содержит объекты, являющиеся внутренним представленим игрового скрипта,
 *	получившиеся в результате лексического и синтаксического анализа(работа объектов LexemAnalyzer и SyntaxAnalyzer)
 *	Каждый объект является внутренней инструкцией, которая может исполняться(методы Evaluate и EvaluateFun).
 *	Всего представлено несколько видов объектов: константы(PolizConst), вычисляемые арифм. и логич. выражения, а также игровые функции(PolizFunction),
 *	операторы безусловного(PolizOpGo) и условного(PolizIfThen) переходов.
 *	Некоторые инструкции требуют для выполнения параметры, которые хранятся в спец. стеке в модуле bot_mg.
 *	Сами объекты-операторы(PolizElem) хранятся в контейнере PolizItem
 */

#ifndef POLIZ_ELEM_HPP
#define POLIZ_ELEM_HPP

#include "PolizItem.hpp"

class PolizElem
{
public:
	virtual ~PolizElem() {}
	virtual void Evaluate(PolizItem** stack, PolizItem** cur_cmd) = 0;
protected:
	static void Push(PolizItem** stack, PolizElem* elem);
	static PolizElem* Pop(PolizItem** stack);
	static int Size(PolizItem** stack);
	static int Clear(PolizItem** stack);
};

class PolizConst : public PolizElem
{
public:
	virtual ~PolizConst() {}
	virtual PolizElem* Clone() const = 0;
	virtual void Evaluate(PolizItem** stack, PolizItem** cur_cmd);
};

class PolizInt : public PolizConst
{
	int value;
public:
	PolizInt(int a) : value(a) {}
	PolizInt(const PolizInt& a);
	PolizInt& operator=(const PolizInt& a);
	virtual ~PolizInt() {}
	virtual PolizElem* Clone() const { return new PolizInt(value); }
	int Get() const { return value; }
private:
	PolizInt() {}
};

class PolizString : public PolizConst
{
	char* value;
	int size;
public:
	PolizString(char* a, int a_size);
	PolizString(const PolizString& a);
	PolizString& operator=(const PolizString& a);
	virtual ~PolizString();
	virtual PolizElem* Clone() const { return new PolizString(value, size); }
	char* Get() const { return value; }
private:
	PolizString() {}
};

class PolizVarAddr : public PolizConst
{
	int value;
public:
	PolizVarAddr(int a) : value(a) {}
	PolizVarAddr(const PolizVarAddr& a);
	PolizVarAddr& operator=(const PolizVarAddr& a);
	virtual ~PolizVarAddr() {}
	virtual PolizElem* Clone() const { return new PolizVarAddr(value); }
	int Get() const { return value; }
private:
	PolizVarAddr() {}
};

class PolizLabel : public PolizConst
{
	PolizItem* value;
public:
	PolizLabel(PolizItem* a) : value(a) {}
	PolizLabel(const PolizLabel& a);
	PolizLabel& operator=(const PolizLabel& a) { if ( !a.value ) return *this; value = a.value; return *this; }
	virtual ~PolizLabel() {}
	virtual PolizElem* Clone() const { return new PolizLabel(value); }
	PolizItem* Get() const { return value; }
private:
	PolizLabel() {}
};



class PolizOpGo : public PolizElem
{
public:
	PolizOpGo() {}
	virtual ~PolizOpGo() {}
	virtual void Evaluate(PolizItem** stack, PolizItem** cur_cmd);
};

class PolizOpIfThen : public PolizElem
{
public:
	PolizOpIfThen() {}
	virtual ~PolizOpIfThen() {}
	virtual void Evaluate(PolizItem** stack, PolizItem** cur_cmd);
};



class PolizFunction : public PolizElem
{
public:
	virtual PolizElem* EvaluateFun(PolizItem** stack) = 0;
	virtual void Evaluate(PolizItem** stack, PolizItem** cur_cmd);
protected:
	static PolizElem* EvalGameFunction(PolizItem** stack, const char* function_name);
};

class PolizVar : public PolizFunction
{
	PolizVarAddr* var_addr;
	char* var_name;
	int var_name_size;
	int var_value;
public:
	PolizVar(const char* var_name, int var_name_size, PolizInt* var_value);
	virtual ~PolizVar();
	virtual PolizElem* EvaluateFun(PolizItem** stack);
	int GetVarValue() const { return var_value; }
	void SetVarValue(int new_value) { var_value = new_value; }
	PolizVarAddr* GetVarAddr() const { return var_addr; }
	void SetVarAddr(PolizVarAddr* addr) { var_addr = addr; }
	char* GetVarName() const { return var_name; }
	void SetVarName(const char* var_name, int var_name_size);
};

class PolizAssign : public PolizFunction
{
public:
	PolizAssign() {}
	virtual ~PolizAssign() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFMyId : public PolizFunction
{
public:
	PolizGFMyId() {}
	virtual ~PolizGFMyId() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFTurn : public PolizFunction
{
public:
	PolizGFTurn() {}
	virtual ~PolizGFTurn() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFPlayers : public PolizFunction
{
public:
	PolizGFPlayers() {}
	virtual ~PolizGFPlayers() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFAPlayers : public PolizFunction
{
public:
	PolizGFAPlayers() {}
	virtual ~PolizGFAPlayers() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFSupply : public PolizFunction
{
public:
	PolizGFSupply() {}
	virtual ~PolizGFSupply() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFRawPrice : public PolizFunction
{
public:
	PolizGFRawPrice() {}
	virtual ~PolizGFRawPrice() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFDemand : public PolizFunction
{
public:
	PolizGFDemand() {}
	virtual ~PolizGFDemand() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFProdPrice : public PolizFunction
{
public:
	PolizGFProdPrice() {}
	virtual ~PolizGFProdPrice() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFMoney : public PolizFunction
{
public:
	PolizGFMoney() {}
	virtual ~PolizGFMoney() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFRaw : public PolizFunction
{
public:
	PolizGFRaw() {}
	virtual ~PolizGFRaw() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFProduction : public PolizFunction
{
public:
	PolizGFProduction() {}
	virtual ~PolizGFProduction() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFFactories : public PolizFunction
{
public:
	PolizGFFactories() {}
	virtual ~PolizGFFactories() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFManufactured : public PolizFunction
{
public:
	PolizGFManufactured() {}
	virtual ~PolizGFManufactured() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFResRawSold : public PolizFunction
{
public:
	PolizGFResRawSold() {}
	virtual ~PolizGFResRawSold() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFResRawPrice : public PolizFunction
{
public:
	PolizGFResRawPrice() {}
	virtual ~PolizGFResRawPrice() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFResProdBought : public PolizFunction
{
public:
	PolizGFResProdBought() {}
	virtual ~PolizGFResProdBought() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizGFResProdPrice : public PolizFunction
{
public:
	PolizGFResProdPrice() {}
	virtual ~PolizGFResProdPrice() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizPrint : public PolizFunction
{
public:
	PolizPrint() {}
	virtual ~PolizPrint() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizBuy : public PolizFunction
{
public:
	PolizBuy() {}
	virtual ~PolizBuy() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizSell : public PolizFunction
{
public:
	PolizSell() {}
	virtual ~PolizSell() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizProd : public PolizFunction
{
public:
	PolizProd() {}
	virtual ~PolizProd() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizBuild : public PolizFunction
{
public:
	PolizBuild() {}
	virtual ~PolizBuild() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizEndturn : public PolizFunction
{
public:
	PolizEndturn() {}
	virtual ~PolizEndturn() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizFunLess : public PolizFunction
{
public:
	PolizFunLess() {}
	virtual ~PolizFunLess() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizFunGreater : public PolizFunction
{
public:
	PolizFunGreater() {}
	virtual ~PolizFunGreater() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizFunEqual : public PolizFunction
{
public:
	PolizFunEqual() {}
	virtual ~PolizFunEqual() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizFunNotEqual : public PolizFunction
{
public:
	PolizFunNotEqual() {};
	virtual ~PolizFunNotEqual() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizFunOr : public PolizFunction
{
public:
	PolizFunOr() {}
	virtual ~PolizFunOr() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizFunAnd : public PolizFunction
{
public:
	PolizFunAnd() {}
	virtual ~PolizFunAnd() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizFunNot : public PolizFunction
{
public:
	PolizFunNot() {}
	virtual ~PolizFunNot() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizFunUnMinus : public PolizFunction
{
public:
	PolizFunUnMinus() {}
	virtual ~PolizFunUnMinus() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizFunPlus : public PolizFunction
{
public:
	PolizFunPlus() {}
	virtual ~PolizFunPlus() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizFunMinus : public PolizFunction
{
public:
	PolizFunMinus() {}
	virtual ~PolizFunMinus() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizFunDiv : public PolizFunction
{
public:
	PolizFunDiv() {}
	virtual ~PolizFunDiv() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizFunMul : public PolizFunction
{
public:
	PolizFunMul() {}
	virtual ~PolizFunMul() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

class PolizFunRem : public PolizFunction
{
public:
	PolizFunRem() {}
	virtual ~PolizFunRem() {}
	virtual PolizElem* EvaluateFun(PolizItem** stack);
};

#endif
