#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <cctype>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
using namespace std;
#endif /* __PROGTEST__ */

/* ------------------- FUN EXCEPTION -------------------- */
class thisIsTheEndOfTheProgramBecauseOutOfBoundExceptionOccuredAndILikeBigExceptionsNamesJustLikeInJava
{

};

/* ------------------- SIGNATURE HEADER ----------------- */

class CSig
{
public:
		CSig(const char *, bool, const char *, int);
		~CSig();
		char * sig;
		bool deb;
		char * opp;
		int amount;
};

CSig::CSig(const char * sig, bool deb, const char * opp, int amount)
{
	int len = strlen(sig);

	this->sig = new char[len+1];
	strcpy(this->sig, sig);
	this->sig[len] = '\0';

	this->deb = deb;

	len = strlen(opp);
	this->opp = new char[len + 1];
	strcpy(this->opp, opp);
	this->opp[len] = '\0';

	this->amount = amount;
}

CSig::~CSig()
{
	delete [] sig;
	delete [] opp;
	sig = nullptr;
	opp = nullptr;
}


/* ------------------- ACCOUNT HEADER ------------------- */
class CAccount
{
private:
		void cleanSigs();

public:
		CAccount();
		~CAccount();
		CAccount(const char *, int);
		CAccount(const char *, int, int);
		int Balance();
		friend ostream& operator<< (ostream& inOs, const CAccount& inAcc);
		ostream& getSigs(ostream &inOs) const;

		char * accID;
		long long balance;
		long long startBalance;
		CSig** sigArray;
		unsigned int sigNum;
		unsigned int sigMax;

		void addSig(const char * sig, bool deb, const char * opp, int amount);
		void trim();
};


class CAccountArray
{
	public:
		CAccountArray();

		~CAccountArray();

		CAccount **accountArray;
		int shared;
		int accountNum;
		int accountMax;

		bool find_account(const char * accID);

		bool find_account(const char * accID, CAccount *&res);

		void cleanAccounts();

		bool addAccount(const char *accID, int initialBalance);

		bool NewAccount(const char *accID, int initialBalance, int startBalance);

		void copyAccounts(const CAccountArray * inAcc);

};

CAccountArray::CAccountArray()
{
	accountNum = accountMax = shared = 0;
	accountArray = nullptr;
}

CAccountArray::~CAccountArray()
{
	cleanAccounts();
}

bool CAccountArray::find_account(const char *accID, CAccount *&res)
{
	for (int i = 0; i < accountNum; i++)
		if (strcmp(accountArray[i]->accID, accID) == 0)
		{
			res = accountArray[i];
			return true;
		}
	return false;
}


bool CAccountArray::find_account(const char *accID)
{
	for (int i = 0; i < accountNum; i++)
		if (strcmp(accountArray[i]->accID, accID) == 0)
			return true;
	return false;
}

bool CAccountArray::addAccount(const char *accID, int initialBalance)
{
	if (find_account(accID))
		return false;

	if (accountNum >= accountMax)
	{
		//accountMax += (accountMax > 1000) ? accountMax/2 : 5;
		accountMax += 5;
		accountArray = (CAccount **) realloc(accountArray, accountMax* sizeof(**accountArray));
	}

	CAccount* tmp = new CAccount(accID, initialBalance);
	accountArray[accountNum++] = tmp;
	return true;
}

void CAccountArray::cleanAccounts()
{
	for (int i = 0; i < accountNum; i++)
		delete accountArray[i];
	free(accountArray);
	accountArray = nullptr;
	accountNum = accountMax = shared = 0;
}

bool CAccountArray::NewAccount(const char *accID, int initialBalance, int startBalance)
{
	if (find_account(accID))
		return false;

	if (accountNum >= accountMax)
	{
		//accountMax += (accountMax > 96) ? accountMax/2 : 12;
		accountMax += 5;
		accountArray = (CAccount **) realloc(accountArray, accountMax* sizeof(**accountArray));
	}

	CAccount* tmp = new CAccount(accID, initialBalance, startBalance);
	accountArray[accountNum++] = tmp;
	return true;
}

void CAccountArray::copyAccounts(const CAccountArray * inAcc)
{
	for (int i = 0; i < inAcc->accountNum; i++)
	{
		NewAccount(inAcc->accountArray[i]->accID, inAcc->accountArray[i]->balance, inAcc->accountArray[i]->startBalance);
		for (unsigned int j = 0; j < inAcc->accountArray[i]->sigNum; j++)
			accountArray[i]->addSig(inAcc->accountArray[i]->sigArray[j]->sig, inAcc->accountArray[i]->sigArray[j]->deb, inAcc->accountArray[i]->sigArray[j]->opp, inAcc->accountArray[i]->sigArray[j]->amount);
	}
}

/* ----------------- BANK HEADER ----------------------- */

class CBank
{
public:
		// default constructor
		CBank();
		// copy constructor
		CBank(const CBank &);
		// destructor
		~CBank();
		// operator =
		CBank& operator=(const CBank &);

		bool   NewAccount    ( const char * accID, int initialBalance );
		bool   Transaction   ( const char * debAccID, const char * credAccID, unsigned int amount, const char * signature );
		bool   TrimAccount   ( const char * accID );
		CAccount& Account(const char * accID) const;

private:
		CAccountArray* accountArrayWrapper;
		void handleShared();
};

/* -------------- BANK IMPLEMENTATION --------------------- */

CBank::CBank()
{
	accountArrayWrapper = new CAccountArray();
}

CBank::~CBank()
{
	if (accountArrayWrapper->shared <= 0)
		delete accountArrayWrapper;
	else
		accountArrayWrapper->shared--;
}

CBank::CBank(const CBank &inBank)
{
	accountArrayWrapper = inBank.accountArrayWrapper;
	inBank.accountArrayWrapper->shared++;
}


CAccount& CBank::Account(const char * accID) const
{
	CAccount* res = nullptr;
	if (!accountArrayWrapper->find_account(accID, res))
		throw thisIsTheEndOfTheProgramBecauseOutOfBoundExceptionOccuredAndILikeBigExceptionsNamesJustLikeInJava();

	return *res;
}

bool CBank::Transaction(const char *debAccID, const char *credAccID, unsigned int amount, const char *signature)
{
	if (strcmp(debAccID, credAccID)==0)
		return false;

	handleShared();
	CAccount *toDeb = nullptr, *toCred = nullptr;
	if (!accountArrayWrapper->find_account(debAccID, toDeb) || !accountArrayWrapper->find_account(credAccID, toCred))
		return false;

	toDeb->balance -= amount;
	toCred->balance += amount;

	toDeb->addSig(signature, true, toCred->accID, amount);
	toCred->addSig(signature, false, toDeb->accID, amount);
	return true;
}

bool CBank::TrimAccount(const char *accID)
{
	handleShared();
	CAccount* res = nullptr;
	if (!accountArrayWrapper->find_account(accID, res))
		return false;


	res->trim();
	return true;
}


CBank &CBank::operator=(const CBank &inBank)
{
	if (&inBank == this) return *this;
	accountArrayWrapper->cleanAccounts();
	delete accountArrayWrapper;

	accountArrayWrapper = inBank.accountArrayWrapper;
	accountArrayWrapper->shared++;
	return *this;
}

bool CBank::NewAccount(const char *accID, int initialBalance)
{
	handleShared();
	return accountArrayWrapper->addAccount(accID, initialBalance);
}

void CBank::handleShared()
{
	if (accountArrayWrapper->shared > 0)
	{
		accountArrayWrapper->shared--;
		CAccountArray *tmp = accountArrayWrapper;
		accountArrayWrapper = new CAccountArray();
		accountArrayWrapper->copyAccounts(tmp);
	}
}

/* ------------ ACCOUNT IMPLEMENTATION ------------------- */

CAccount::CAccount(const char * accID, int initialBalance)
{
	balance = initialBalance;
	startBalance = initialBalance;
	int len = strlen(accID);
	this->accID = new char[len+1];
	strcpy(this->accID, accID);
	this->accID[len] = '\0';
	sigNum = 0;
	sigMax = 0;
	sigArray = nullptr;
}


CAccount::~CAccount()
{
	cleanSigs();
	delete [] accID;
}


int CAccount::Balance()
{
	return balance;
}

ostream& operator<< (ostream& inOs, const CAccount& inAcc)
{
	inOs << inAcc.accID << ":" << endl << "   " << inAcc.startBalance << endl;
	inAcc.getSigs(inOs) << " = " << inAcc.balance << endl;
	//1000\n - 300, opp: 987654, sign: XAbG5uKz6E=\n - 2890, opp: 987654, sign: AbG5uKz6E=\n = -2190\n" )
	return inOs;
}

CAccount::CAccount()
{
	sigNum = 0;
	sigMax = 0;
	sigArray = nullptr;
}

void CAccount::addSig(const char *sig, bool deb, const char *opp, int amount)
{
	if (sigNum >= sigMax)
	{
		//sigMax += (sigMax > 96) ? sigMax/2 : 12;
		sigMax += 5;
		sigArray = (CSig **) realloc(sigArray, sigMax*sizeof(**sigArray));
	}

	CSig *tmp = new CSig(sig, deb, opp, amount);
	sigArray[sigNum++] = tmp;
}

ostream &CAccount::getSigs(ostream &inOs) const
{
	for (unsigned int i = 0; i < sigNum; i++)
	{
		if (sigArray[i]->deb)
			inOs << " - " << sigArray[i]->amount << ", " << "to: " << sigArray[i]->opp << ", " << "sign: " << sigArray[i]->sig << endl;
		else
			inOs << " + " << sigArray[i]->amount << ", " << "from: " << sigArray[i]->opp << ", " << "sign: " << sigArray[i]->sig << endl;
	}
	return inOs;
}

void CAccount::trim()
{
	cleanSigs();
	this->startBalance = balance;
}

void CAccount::cleanSigs()
{
	for (unsigned int i = 0; i < sigNum; i++)
		delete sigArray[i];
	free(sigArray);
	sigArray = nullptr;
	sigNum = 0;
	sigMax = 0;
}

CAccount::CAccount(const char *accID, int initialBalance, int startBalance)
{
	balance = initialBalance;
	this->startBalance = startBalance;
	int len = strlen(accID);
	this->accID = new char[len+1];
	strcpy(this->accID, accID);
	this->accID[len] = '\0';
	sigNum = 0;
	sigMax = 0;
	sigArray = nullptr;
}

#ifndef __PROGTEST__
int main ( void )
{
	ostringstream os;
	char accCpy[100], debCpy[100], credCpy[100], signCpy[100];
	CBank x0;
	assert ( x0 . NewAccount ( "123456", 1000 ) );
	assert ( x0 . NewAccount ( "987654", -500 ) );
	assert ( x0 . Transaction ( "123456", "987654", 300, "XAbG5uKz6E=" ) );
	assert ( x0 . Transaction ( "123456", "987654", 2890, "AbG5uKz6E=" ) );
	assert ( x0 . NewAccount ( "111111", 5000 ) );
	assert ( x0 . Transaction ( "111111", "987654", 290, "Okh6e+8rAiuT5=" ) );
	assert ( x0 . Account ( "123456" ). Balance ( ) ==  -2190 );
	assert ( x0 . Account ( "987654" ). Balance ( ) ==  2980 );
	assert ( x0 . Account ( "111111" ). Balance ( ) ==  4710 );
	os . str ( "" );
	os << x0 . Account ( "123456" );

	assert ( ! strcmp ( os . str () . c_str (), "123456:\n   1000\n - 300, to: 987654, sign: XAbG5uKz6E=\n - 2890, to: 987654, sign: AbG5uKz6E=\n = -2190\n" ) );
	os . str ( "" );
	os << x0 . Account ( "987654" );
	assert ( ! strcmp ( os . str () . c_str (), "987654:\n   -500\n + 300, from: 123456, sign: XAbG5uKz6E=\n + 2890, from: 123456, sign: AbG5uKz6E=\n + 290, from: 111111, sign: Okh6e+8rAiuT5=\n = 2980\n" ) );
	os . str ( "" );
	os << x0 . Account ( "111111" );
	assert ( ! strcmp ( os . str () . c_str (), "111111:\n   5000\n - 290, to: 987654, sign: Okh6e+8rAiuT5=\n = 4710\n" ) );
	assert ( x0 . TrimAccount ( "987654" ) );
	assert ( x0 . Transaction ( "111111", "987654", 123, "asdf78wrnASDT3W" ) );
	os . str ( "" );
	os << x0 . Account ( "987654" );
	assert ( ! strcmp ( os . str () . c_str (), "987654:\n   2980\n + 123, from: 111111, sign: asdf78wrnASDT3W\n = 3103\n" ) );

	CBank x2;
	strncpy ( accCpy, "123456", sizeof ( accCpy ) );
	assert ( x2 . NewAccount ( accCpy, 1000 ));
	strncpy ( accCpy, "987654", sizeof ( accCpy ) );
	assert ( x2 . NewAccount ( accCpy, -500 ));
	strncpy ( debCpy, "123456", sizeof ( debCpy ) );
	strncpy ( credCpy, "987654", sizeof ( credCpy ) );
	strncpy ( signCpy, "XAbG5uKz6E=", sizeof ( signCpy ) );
	assert ( x2 . Transaction ( debCpy, credCpy, 300, signCpy ) );
	strncpy ( debCpy, "123456", sizeof ( debCpy ) );
	strncpy ( credCpy, "987654", sizeof ( credCpy ) );
	strncpy ( signCpy, "AbG5uKz6E=", sizeof ( signCpy ) );
	assert ( x2 . Transaction ( debCpy, credCpy, 2890, signCpy ) );
	strncpy ( accCpy, "111111", sizeof ( accCpy ) );
	assert ( x2 . NewAccount ( accCpy, 5000 ));
	strncpy ( debCpy, "111111", sizeof ( debCpy ) );
	strncpy ( credCpy, "987654", sizeof ( credCpy ) );
	strncpy ( signCpy, "Okh6e+8rAiuT5=", sizeof ( signCpy ) );
	assert ( x2 . Transaction ( debCpy, credCpy, 2890, signCpy ) );
	assert ( x2 . Account ( "123456" ). Balance ( ) ==  -2190 );
	assert ( x2 . Account ( "987654" ). Balance ( ) ==  5580 );
	assert ( x2 . Account ( "111111" ). Balance ( ) ==  2110 );
	os . str ( "" );
	os << x2 . Account ( "123456" );
	assert ( ! strcmp ( os . str () . c_str (), "123456:\n   1000\n - 300, to: 987654, sign: XAbG5uKz6E=\n - 2890, to: 987654, sign: AbG5uKz6E=\n = -2190\n" ) );
	os . str ( "" );
	os << x2 . Account ( "987654" );
	assert ( ! strcmp ( os . str () . c_str (), "987654:\n   -500\n + 300, from: 123456, sign: XAbG5uKz6E=\n + 2890, from: 123456, sign: AbG5uKz6E=\n + 2890, from: 111111, sign: Okh6e+8rAiuT5=\n = 5580\n" ) );
	os . str ( "" );
	os << x2 . Account ( "111111" );
	assert ( ! strcmp ( os . str () . c_str (), "111111:\n   5000\n - 2890, to: 987654, sign: Okh6e+8rAiuT5=\n = 2110\n" ) );
	assert ( x2 . TrimAccount ( "987654" ) );
	strncpy ( debCpy, "111111", sizeof ( debCpy ) );
	strncpy ( credCpy, "987654", sizeof ( credCpy ) );
	strncpy ( signCpy, "asdf78wrnASDT3W", sizeof ( signCpy ) );
	assert ( x2 . Transaction ( debCpy, credCpy, 123, signCpy ) );
	os . str ( "" );
	os << x2 . Account ( "987654" );
	assert ( ! strcmp ( os . str () . c_str (), "987654:\n   5580\n + 123, from: 111111, sign: asdf78wrnASDT3W\n = 5703\n" ) );

	CBank x4;
	assert ( x4 . NewAccount ( "123456", 1000 ) );
	assert ( x4 . NewAccount ( "987654", -500 ) );
	assert ( !x4 . NewAccount ( "123456", 3000 ) );
	assert ( !x4 . Transaction ( "123456", "666", 100, "123nr6dfqkwbv5" ) );
	assert ( !x4 . Transaction ( "666", "123456", 100, "34dGD74JsdfKGH" ) );
	assert ( !x4 . Transaction ( "123456", "123456", 100, "Juaw7Jasdkjb5" ) );
	try
	{
		x4 . Account ( "666" ). Balance ( );
		assert ( "Missing exception !!" == NULL );
	}
	catch ( ... )
	{
	}
	try
	{
		os << x4 . Account ( "666" ). Balance ( );
		assert ( "Missing exception !!" == NULL );
	}
	catch ( ... )
	{
	}
	assert ( !x4 . TrimAccount ( "666" ) );

	CBank x6;
	assert ( x6 . NewAccount ( "123456", 1000 ) );
	assert ( x6 . NewAccount ( "987654", -500 ) );
	assert ( x6 . Transaction ( "123456", "987654", 300, "XAbG5uKz6E=" ) );
	assert ( x6 . Transaction ( "123456", "987654", 2890, "AbG5uKz6E=" ) );
	assert ( x6 . NewAccount ( "111111", 5000 ) );
	assert ( x6 . Transaction ( "111111", "987654", 2890, "Okh6e+8rAiuT5=" ) );
	CBank x7 ( x6 );
	assert ( x6 . Transaction ( "111111", "987654", 123, "asdf78wrnASDT3W" ) );
	assert ( x7 . Transaction ( "111111", "987654", 789, "SGDFTYE3sdfsd3W" ) );
	assert ( x6 . NewAccount ( "99999999", 7000 ) );
	assert ( x6 . Transaction ( "111111", "99999999", 3789, "aher5asdVsAD" ) );
	assert ( x6 . TrimAccount ( "111111" ) );
	assert ( x6 . Transaction ( "123456", "111111", 221, "Q23wr234ER==" ) );
	os . str ( "" );
	os << x6 . Account ( "111111" );
	assert ( ! strcmp ( os . str () . c_str (), "111111:\n   -1802\n + 221, from: 123456, sign: Q23wr234ER==\n = -1581\n" ) );
	os . str ( "" );
	os << x6 . Account ( "99999999" );
	assert ( ! strcmp ( os . str () . c_str (), "99999999:\n   7000\n + 3789, from: 111111, sign: aher5asdVsAD\n = 10789\n" ) );
	os . str ( "" );
	os << x6 . Account ( "987654" );
	assert ( ! strcmp ( os . str () . c_str (), "987654:\n   -500\n + 300, from: 123456, sign: XAbG5uKz6E=\n + 2890, from: 123456, sign: AbG5uKz6E=\n + 2890, from: 111111, sign: Okh6e+8rAiuT5=\n + 123, from: 111111, sign: asdf78wrnASDT3W\n = 5703\n" ) );
	os . str ( "" );
	os << x7 . Account ( "111111" );
	assert ( ! strcmp ( os . str () . c_str (), "111111:\n   5000\n - 2890, to: 987654, sign: Okh6e+8rAiuT5=\n - 789, to: 987654, sign: SGDFTYE3sdfsd3W\n = 1321\n" ) );
	try
	{
		os << x7 . Account ( "99999999" ). Balance ( );
		assert ( "Missing exception !!" == NULL );
	}
	catch ( ... )
	{
	}
	os . str ( "" );
	os << x7 . Account ( "987654" );
	assert ( ! strcmp ( os . str () . c_str (), "987654:\n   -500\n + 300, from: 123456, sign: XAbG5uKz6E=\n + 2890, from: 123456, sign: AbG5uKz6E=\n + 2890, from: 111111, sign: Okh6e+8rAiuT5=\n + 789, from: 111111, sign: SGDFTYE3sdfsd3W\n = 6369\n" ) );

	CBank x8;
	CBank x9;
	assert ( x8 . NewAccount ( "123456", 1000 ) );
	assert ( x8 . NewAccount ( "987654", -500 ) );
	assert ( x8 . Transaction ( "123456", "987654", 300, "XAbG5uKz6E=" ) );
	assert ( x8 . Transaction ( "123456", "987654", 2890, "AbG5uKz6E=" ) );
	assert ( x8 . NewAccount ( "111111", 5000 ) );
	assert ( x8 . Transaction ( "111111", "987654", 2890, "Okh6e+8rAiuT5=" ) );
	x9 = x8;
	assert ( x8 . Transaction ( "111111", "987654", 123, "asdf78wrnASDT3W" ) );
	assert ( x9 . Transaction ( "111111", "987654", 789, "SGDFTYE3sdfsd3W" ) );
	assert ( x8 . NewAccount ( "99999999", 7000 ) );
	assert ( x8 . Transaction ( "111111", "99999999", 3789, "aher5asdVsAD" ) );
	assert ( x8 . TrimAccount ( "111111" ) );
	os . str ( "" );
	os << x8 . Account ( "111111" );
	assert ( ! strcmp ( os . str () . c_str (), "111111:\n   -1802\n = -1802\n" ) );
	os . str ( "" );
	os << x9 . Account ( "111111" );
	assert ( ! strcmp ( os . str () . c_str (), "111111:\n   5000\n - 2890, to: 987654, sign: Okh6e+8rAiuT5=\n - 789, to: 987654, sign: SGDFTYE3sdfsd3W\n = 1321\n" ) );

	CBank x10(x9);
	CBank x11(x9);
	CBank x12(x9);
	CBank x13(x9);

	return 0;
}
#endif /* __PROGTEST__ */
