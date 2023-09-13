#ifndef __PROGTEST__
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <cctype>
#include <climits>
#include <cstdint>
#include <cassert>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>
#include <functional>
#include <stdexcept>
using namespace std;
#endif /* __PROGTEST__ */

class CBigInt
{
public:
		// default constructor
		CBigInt();

		// copying/assignment/destruction
		CBigInt(const CBigInt&);
		CBigInt& operator=(CBigInt);
		~CBigInt();

		// int constructor
		CBigInt(int);

		// string constructor
		CBigInt(const char* in);

		// operator +, any combination {CBigInt/int/string} + {CBigInt/int/string}
		friend CBigInt operator+(const CBigInt&, const CBigInt&);

		// operator *, any combination {CBigInt/int/string} * {CBigInt/int/string}
		friend CBigInt operator*(const CBigInt&, const CBigInt&);

		// operator +=, any of {CBigInt/int/string}
		CBigInt& operator+=(const CBigInt);

		// operator *=, any of {CBigInt/int/string}
		CBigInt& operator*=(const CBigInt);

		// comparison operators, any combination {CBigInt/int/string} {<,<=,>,>=,==,!=} {CBigInt/int/string}
		friend bool operator==(const CBigInt&, const CBigInt&);
		friend bool operator!=(const CBigInt&, const CBigInt&);
		friend bool operator>=(const CBigInt&, const CBigInt&);
		friend bool operator<=(const CBigInt&, const CBigInt&);
		friend bool operator>(const CBigInt&, const CBigInt&);
		friend bool operator<(const CBigInt&, const CBigInt&);

		// output operator <<
		friend ostream& operator<<(std::ostream& out, const CBigInt& x);
		// input operator >>
		friend istream& operator>>(std::istream& in, CBigInt& x);
private:
		bool compByDigit(const string& a, const string& b);
		bool checkStr(const string& a);
		string num;
		char mark;
};

bool higher(const string& a, const string& b);
bool compByDigit(const string& a, const string& b);
string sum(string a, string b);
string subtr(string a, string b);
void shrinkNulls(string& a);
string multi(string a, string b);
string multiOne(string a, char b);

/* ------------------ CONSTRUCTOR, ASIGMENTS, DESTRUCTOR --------------------- */
CBigInt::CBigInt()
{
	num = "0";
	mark = '+';
}

CBigInt::CBigInt(int in)
{
	if (in < 0)
	{
		mark='-';
		num = to_string(-in);
	}
	else
	{
		mark='+';
		num = to_string(in);
	}
}

CBigInt::CBigInt(const char* in)
{
	string inStr = in;
	if (inStr[0] == '-')
	{
		mark = '-';
		inStr.erase(0,1);
	}
	else
		mark = '+';

	if (!checkStr(inStr))
		throw invalid_argument("");
	shrinkNulls(inStr);
	num = inStr;
}

CBigInt::CBigInt(const CBigInt& in)
{
	string tmp(in.num);
	num = tmp;
	mark = in.mark;
}

CBigInt::~CBigInt()
{
}

CBigInt& CBigInt::operator=(CBigInt in)
{
	swap(num, in.num);
	swap(mark, in.mark);
	return *this;
}

/* ------------------------------------ +=, *= ------------------------ */
CBigInt& CBigInt::operator+=(const CBigInt toAdd)
{
	if ((mark=='+' && toAdd.mark=='+') || (mark=='-' && toAdd.mark=='-'))
	{
		num = sum(toAdd.num, num);
	}

	else
	{
		num = subtr(toAdd.num, num);
		if (!compByDigit(num, toAdd.num))
		{
			if (mark == '-') mark = '+';
			else mark = '-';
		}
	}
	if (this->num[0] == '0')
		this->mark = '+';

	return *this;
}

CBigInt& CBigInt::operator*=(const CBigInt toMulti)
{
	if ((mark=='+' && toMulti.mark=='+') || (mark=='-' && toMulti.mark=='-'))
		mark = '+';
	else
		mark = '-';

	num = multi(toMulti.num, num);
	if (this->num[0] == '0')
		this->mark = '+';
	return *this;
}

/* ------------------------------------ +, * ------------------------ */

CBigInt operator+(const CBigInt& a, const CBigInt& b)
{
	CBigInt res(a);
	if ((res.mark=='+' && b.mark=='+') || (res.mark=='-' && b.mark=='-'))
		res.num = sum(b.num, res.num);
	else
	{
		if (!compByDigit(res.num, b.num))
		{
			if (res.mark == '-') res.mark = '+';
			else res.mark = '-';
		}
		res.num = subtr(b.num, res.num);
	}

	if (res.num[0] == '0')
		res.mark = '+';
	return res;
}

CBigInt operator*(const CBigInt& a, const CBigInt& b)
{
	CBigInt res(a);

	if ((res.mark =='+' && b.mark=='+') || (res.mark=='-' && b.mark=='-'))
		res.mark = '+';
	else
		res.mark = '-';

	res.num = multi(b.num, res.num);
	if (res.num[0] == '0')
		res.mark = '+';
	return res;
}

/* -------------------------------- STREAM -------------------------- */
ostream &operator<<(std::ostream& out, const CBigInt& x)
{
	if (x.mark=='-' && x.num[0] != '0')
		out << x.mark;
	out << x.num;
	return out;
}

istream &operator>>(std::istream& in, CBigInt& x)
{
	string tmp = "";
	char c;
	char tmpMark = '+';

	bool start = true;
	while((c = in.peek()) != EOF)
	{
		if (c == ' ' && start)
		{
			in.get();
			continue;
		}
		if (!isdigit(c))
		{
			if (start && c == '-')
			{
				tmpMark = '-';
			}
			else if (start)
			{
				in.setstate(ios::failbit);
				return in;
			}
			break;
		}
		tmp.push_back(in.get());
		start = false;
	}
	if (start)
	{
		in.setstate(ios::failbit);
		return in;
	}

	x = "0";
	if (tmpMark == '-')
		tmp.insert(0, "-");
	x += tmp.c_str();
	return in;
}

/* ------------------------------- PRIVATE ------------------------- */

/* ------------------------------- CALCULATIONS -------------------- */
string sum(string a, string b)
{
	if (a.size() < b.size())
		swap(a, b);

	int len = a.size()-1;
	int j = len;
	int next = 0;
	for (int i = b.size()-1; i >= 0; i--, j--)
	{
		a[j] += (b[i]-'0')+next;
		if (a[j] > '9')
		{
			int tmp = a[j]-'0';
			a[j] = '0'+(tmp%10);
			next = tmp/10;
		}
		else
			next = 0;
	}

	for (; j>=0; j--)
	{
		a[j] += next;
		if (a[j] > '9')
		{
			int tmp = a[j]-'0';
			a[j] = '0'+(tmp%10);
			next = tmp/10;
		}
		else
			next = 0;
	}

	if (next > 0)
		a.insert(0, to_string(next));

	if (a.size() == 0)
		a = "0";
	return a;
}

string subtr(string a, string b)
{
	if (a.size() < b.size())
		swap(a, b);

	int len = a.size()-1;
	int j = len;
	int next = 0;

	for (int i = b.size()-1; i >= 0; i--, j--)
	{
		int tmp = a[j];
		tmp -= (b[i]-'0')+next;
		if (tmp < '0')
		{
			a[j] = (10+a[j])-(b[i]-'0')-next;
			next = 1;
		}
		else
		{
			a[j] = tmp;
			next = 0;
		}
	}

	for (; j>=0; j--)
	{
		int tmp = a[j];
		tmp -= next;
		if (tmp < '0')
		{
			a[j] = (10+a[j])-next;
			next = 1;
		}
		else
		{
			a[j] = tmp;
			next = 0;
		}
	}


	shrinkNulls(a);
	if (a.size() == 0)
		a = "0";
	return a;
}

void shrinkNulls(string& a)
{
	while (a[0] == '0')
		a.erase(0, 1);
}

string multiOne(string a, char b)
{
	int next = 0;
	for (int i = a.size()-1; i >= 0; i--)
	{
		a[i] = (a[i]-'0') * (b-'0')+next;
		if (a[i] > 9)
		{
			int tmp = a[i];
			a[i] = tmp%10;
			next = tmp/10;
		}
		else
			next = 0;
		a[i] += '0';
	}

	if (next > 0)
		a.insert(0, to_string(next));

	return a;
}

string multi(string a, string b)
{
	if ((a.size() == 1 && a[0] == '0') ||(b.size()==1 && b[0] == '0'))
		return "0";
	if (a.size() < b.size())
		swap(a, b);

	string res("");
	string tmp = "";

	for (int i = b.size()-1; i >= 0; i--)
	{
		tmp = multiOne(a, b[i]);
		for (int j = b.size()-1; j > i; j--)
			tmp.push_back('0');
		res = sum(res, tmp);
		shrinkNulls(res);
	}
	return res;
}

bool CBigInt::compByDigit(const string& a, const string& b)
{
	int lenA = a.size();
	int lenB = b.size();
	if (lenA > lenB) return true;
	else if (lenB > lenA) return false;

	for (int i = 0; i < lenA; i++)
	{
		if ((a[i]-'0') > (b[i]-'0'))
			return true;
		else if ((a[i]-'0') < (b[i]-'0'))
			return false;
	}
	return true;
}

/* ------------------------ INPUTS ---------------------------- */
bool CBigInt::checkStr(const string& a)
{
	int len = a.size();
	for (int i = 0; i < len; i++)
	{
		if (!isdigit(a[i]))
			return false;
	}
	return true;
}

/* ------------------------ CMP ------------------------------- */
bool operator==(const CBigInt& a, const CBigInt& b)
{
	if (a.num[0] == '0' && b.num[0] == '0') return true;
	else if (a.mark != b.mark) return false;
	return (a.num == b.num);
}

bool operator!=(const CBigInt& a, const CBigInt& b)
{
	if (a.num[0] == '0' && b.num[0] == '0') return false;
	if (a.mark != b.mark) return true;
	return (a.num != b.num);
}

bool operator>=(const CBigInt& a, const CBigInt& b)
{
	if (a.num[0] == '0' && b.num[0] == '0') return true;
	if (a.mark == b.mark)
		return compByDigit(a.num, b.num);
	else if (a.mark == '+' && b.mark == '-')
		return true;
	else
		return false;
}

bool operator<=(const CBigInt& a, const CBigInt& b)
{
	if (a.num[0] == '0' && b.num[0] == '0') return true;
	if (a.mark == b.mark)
		return compByDigit(b.num, a.num);
	else if (a.mark == '+' && b.mark == '-')
		return false;
	else
		return true;
}

bool operator>(const CBigInt& a, const CBigInt& b)
{
	if (a.num[0] == '0' && b.num[0] == '0') return false;
	if (a.mark == b.mark)
		return higher(a.num, b.num);
	else if (a.mark == '+' && b.mark == '-')
		return true;
	else
		return false;
}

bool operator<(const CBigInt& a, const CBigInt& b)
{
	if (a.num[0] == '0' && b.num[0] == '0') return false;
	if (a.mark == b.mark)
		return higher(b.num, a.num);
	else if (a.mark == '+' && b.mark == '-')
		return false;
	else
		return true;
}


bool higher(const string& a, const string& b)
{
	int lenA = a.size();
	int lenB = b.size();
	if (lenA > lenB) return true;
	else if (lenB > lenA) return false;

	for (int i = 0; i < lenA; i++)
	{
		if ((a[i]-'0') > (b[i]-'0'))
			return true;
		else if ((a[i]-'0') < (b[i]-'0'))
			return false;
	}
	return false;
}

bool compByDigit(const string& a, const string& b)
{
	int lenA = a.size();
	int lenB = b.size();
	if (lenA > lenB) return true;
	else if (lenB > lenA) return false;

	for (int i = 0; i < lenA; i++)
	{
		if ((a[i]-'0') > (b[i]-'0'))
			return true;
		else if ((a[i]-'0') < (b[i]-'0'))
			return false;
	}
	return true;
}

/* ------------------------ END ------------------------------- */

#ifndef __PROGTEST__
static bool equal ( const CBigInt & x, const char * val )
{
	ostringstream oss;
	oss << x;
	return oss . str () == val;
}

int main ()
{
	CBigInt a, b;
	istringstream is;
	a = 10;
	a += 20;
	assert ( equal ( a, "30" ) );
	a *= 5;
	assert ( equal ( a, "150" ) );
	b = a + 3;
	assert ( equal ( b, "153" ) );
	b = a * 7;
	assert ( equal ( b, "1050" ) );
	assert ( equal ( a, "150" ) );


	a = 10;
	a += -20;
	assert ( equal ( a, "-10" ) );
	a *= 5;
	assert ( equal ( a, "-50" ) );
	b = a + 73;
	assert ( equal ( b, "23" ) );
	b = a * -7;
	assert ( equal ( b, "350" ) );
	assert ( equal ( a, "-50" ) );

	a = "12345678901234567890";
	a += "-99999999999999999999";
	assert ( equal ( a, "-87654321098765432109" ) );

	a *= "54321987654321987654";
	assert ( equal ( a, "-4761556948575111126880627366067073182286" ) );
	a *= 0;
	assert ( equal ( a, "0" ) );
	a = 10;
	b = a + "400";
	assert ( equal ( b, "410" ) );
	b = a * "15";
	assert ( equal ( b, "150" ) );
	assert ( equal ( a, "10" ) );

	is . clear ();
	is . str ( " 1234" );
	assert ( is >> b );
	assert ( equal ( b, "1234" ) );
	is . clear ();
	is . str ( " 12 34" );
	assert ( is >> b );
	assert ( equal ( b, "12" ) );
	is . clear ();
	is . str ( "999z" );
	assert ( is >> b );
	assert ( equal ( b, "999" ) );
	is . clear ();
	is . str ( "abcd" );
	assert ( ! ( is >> b ) );
	is . clear ();
	is . str ( "- 758" );
	assert ( ! ( is >> b ) );
	try
	{
		a = "-xyz";
		assert ( "missing an exception" == NULL );
	}
	catch ( const invalid_argument & e )
	{
	}

	a = "73786976294838206464";
	assert ( a < "1361129467683753853853498429727072845824" );
	assert ( a <= "1361129467683753853853498429727072845824" );
	assert ( ! ( a > "1361129467683753853853498429727072845824" ) );
	assert ( ! ( a >= "1361129467683753853853498429727072845824" ) );
	assert ( ! ( a == "1361129467683753853853498429727072845824" ) );
	assert ( a != "1361129467683753853853498429727072845824" );
	assert ( ! ( a < "73786976294838206464" ) );
	assert ( a <= "73786976294838206464" );
	assert ( ! ( a > "73786976294838206464" ) );
	assert ( a >= "73786976294838206464" );
	assert ( a == "73786976294838206464" );
	assert ( ! ( a != "73786976294838206464" ) );
	assert ( a < "73786976294838206465" );
	assert ( a <= "73786976294838206465" );
	assert ( ! ( a > "73786976294838206465" ) );
	assert ( ! ( a >= "73786976294838206465" ) );
	assert ( ! ( a == "73786976294838206465" ) );
	assert ( a != "73786976294838206465" );
	a = "2147483648";
	assert ( ! ( a < -2147483648 ) );
	assert ( ! ( a <= -2147483648 ) );
	assert ( a > -2147483648 );
	assert ( a >= -2147483648 );
	assert ( ! ( a == -2147483648 ) );
	assert ( a != -2147483648 );
	a += a*(-1);
	assert ( 0 == a );
	a = 1+a;
	assert ( a == 1);

	return 0;
}
#endif /* __PROGTEST__ */
