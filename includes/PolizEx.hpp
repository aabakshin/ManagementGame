/* Модуль PolizEx содержит основные типы исключений, которые могут произойти в результате выполнения
 * действий с объектами PolizElem */

#ifndef POLIZ_EX_HPP
#define POLIZ_EX_HPP

#include "PolizElem.hpp"

class PolizEx
{
	PolizElem* addr;
public:
	PolizEx(PolizElem* a) : addr(a) {}
	virtual ~PolizEx() {}
	virtual void PrintErrorMessage() const = 0;
	PolizElem* Get() const { return addr; }
};

class PolizExNotInt : public PolizEx
{
public:
	PolizExNotInt(PolizElem* a) : PolizEx(a) {}
	virtual ~PolizExNotInt() {}
	virtual void PrintErrorMessage() const;
};

class PolizExNotLabel : public PolizEx
{
public:
	PolizExNotLabel(PolizElem* a) : PolizEx(a) {}
	virtual ~PolizExNotLabel() {}
	virtual void PrintErrorMessage() const;
};

class PolizExNotVar : public PolizEx
{
public:
	PolizExNotVar(PolizElem* a) : PolizEx(a) {}
	virtual ~PolizExNotVar() {}
	virtual void PrintErrorMessage() const;
};

class PolizExDivisionZero : public PolizEx
{
public:
	PolizExDivisionZero(PolizElem* a) : PolizEx(a) {}
	virtual ~PolizExDivisionZero() {}
	virtual void PrintErrorMessage() const;
};

#endif
