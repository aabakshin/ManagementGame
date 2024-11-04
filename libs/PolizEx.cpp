/* Файл реализации модуля PolizEx */

#ifndef POLIZ_EX_CPP
#define POLIZ_EX_CPP

#ifndef __CPP_FILE__
#define __CPP_FILE__
#endif

#include "../includes/PolizEx.hpp"
#include <cstdio>

void PolizExNotInt::PrintErrorMessage() const
{
	fprintf(stderr, "\n[PolizExNotInt]: Object \"PolizElem\"(%p) is not an object PolizInt\n", this->Get());
	return;
}

void PolizExNotLabel::PrintErrorMessage() const
{
	fprintf(stderr, "\n[PolizExNotLabel]: Object \"PolizElem\"(%p) is not an object PolizLabel\n", this->Get());
	return;
}

void PolizExNotVar::PrintErrorMessage() const
{
	fprintf(stderr, "\n[PolizExNotVar]: Object \"PolizElem\"(%p) is not an object PolizVar\n", this->Get());
	return;
}

void PolizExDivisionZero::PrintErrorMessage() const
{
	fprintf(stderr, "\n[PolizExDivisionZero]: Object \"PolizElem\"(%p) is ZERO!\n", this->Get());
	return;
}

#endif
