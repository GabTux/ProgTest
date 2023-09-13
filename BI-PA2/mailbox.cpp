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
#include <set>
#include <list>
#include <map>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <memory>
#include <functional>
#include <stdexcept>

#endif /* __PROGTEST */
using namespace std;

#ifndef __PROGTEST__

string toFixedString(int n, int a)
{
	string res;
	string tmp;
	tmp = to_string(a);
	for (unsigned int i = 0; i < n - tmp.length(); i++)
		res += "0";
	res += tmp;
	return res;
}

class CTimeStamp
{
public:
		CTimeStamp(int year,
							 int month,
							 int day,
							 int hour,
							 int minute,
							 int sec)
		{
			dateTime += toFixedString(4, year);
			dateTime += toFixedString(2, month);
			dateTime += toFixedString(2, day);
			dateTime += toFixedString(2, hour);
			dateTime += toFixedString(2, minute);
			dateTime += toFixedString(2, sec);
		}

		int Compare(const CTimeStamp &x) const
		{
			if (this->dateTime < x.dateTime)
				return -1;
			else if (this->dateTime > x.dateTime)
				return 1;
			else
				return 0;
		}

		friend ostream &operator<<(ostream &os,
															 const CTimeStamp &x);

private:
		string dateTime;

};

ostream &operator<<(ostream &os, const CTimeStamp &x)
{
	os << x.dateTime.substr(0, 4) << "-" << x.dateTime.substr(4, 2) << "-" << x.dateTime.substr(6, 2) << " "
		 << x.dateTime.substr(8, 2) << ":" << x.dateTime.substr(10, 2) << ":" << x.dateTime.substr(12, 2);
	return os;
}


//=================================================================================================
class CMailBody
{
public:
		CMailBody(int size,
						string data) :
						m_Size(size),
						m_Data(data)
		{

		}

		// copy cons/op=/destructor is correctly implemented in the testing environment
		friend ostream &operator<<(ostream &os,
															 const CMailBody &x)
		{
			return os << "mail body: " << x.m_Size << " B";
		}

private:
		int m_Size;
		string m_Data;
};

//=================================================================================================
class CAttach
{
public:
		CAttach(int x)
						: m_X(x),
							m_RefCnt(1)
		{
		}

		void AddRef(void) const
		{
			m_RefCnt++;
		}

		void Release(void) const
		{
			if (!--m_RefCnt)
				delete this;
		}

private:
		int m_X;
		mutable int m_RefCnt;

		CAttach(const CAttach &x);

		CAttach &operator=(const CAttach &x);

		~CAttach(void) = default;

		friend ostream &operator<<(ostream &os,
															 const CAttach &x)
		{
			return os << "attachment: " << x.m_X << " B";
		}
};
//=================================================================================================
#endif /* __PROGTEST__, DO NOT remove */


class CMail
{
public:
		CMail(const CTimeStamp &timeStamp,
					const string &from,
					const CMailBody &body,
					const CAttach *attach) :
						m_timeStamp(timeStamp),
						m_from(from),
						m_body(body)
		{
			m_attach = attach;
			if (m_attach) m_attach->AddRef();
		}

		CMail(const CMail &in) noexcept(true):
						m_timeStamp(in.m_timeStamp),
						m_from(in.m_from),
						m_body(in.m_body)
		{
			m_attach = in.m_attach;
			if (m_attach) m_attach->AddRef();
		}

		CMail(const CMail &&in) noexcept(true):
						m_timeStamp(in.m_timeStamp),
						m_from(in.m_from),
						m_body(in.m_body)
		{
			m_attach = in.m_attach;
			if (m_attach) m_attach->AddRef();
		}

		const string &From(void) const
		{
			return m_from;
		}

		const CMailBody &Body(void) const
		{
			return m_body;
		}

		const CTimeStamp &TimeStamp(void) const
		{
			return m_timeStamp;
		}

		const CAttach *Attachment(void) const
		{
			return m_attach;
		}

		friend ostream &operator<<(ostream &os,
															 const CMail &x);

		friend bool operator<(const CMail & a, const CMail & b)
		{
			return a.m_timeStamp.Compare(b.m_timeStamp) < 0;
		}

private:
		CTimeStamp m_timeStamp;
		string m_from;
		CMailBody m_body;
		const CAttach *m_attach;
};

ostream &operator<<(ostream &os, const CMail &x)
{
	os << x.m_timeStamp << " " << x.m_from << " " << x.m_body;
	if (x.m_attach) os << " + " << *x.m_attach;
	return os;
}

//=================================================================================================

class CMailBox
{
public:
		CMailBox(void)
		{
			m_mailMap["inbox"];
		}

		bool Delivery(const CMail &mail)
		{
			m_mailMap["inbox"].insert(mail);
			return true;
		}

		bool NewFolder(const string &folderName)
		{
			if (m_mailMap.count(folderName) > 0)
				return false;
			m_mailMap[folderName];
			return true;
		}

		bool MoveMail(const string &fromFolder,
									const string &toFolder)
		{
			if (fromFolder == toFolder)
				return true;
			if (m_mailMap.count(toFolder) == 0 || m_mailMap.count(fromFolder) == 0)
				return false;
			m_mailMap[toFolder].merge(m_mailMap[fromFolder]);
			return true;
		}

		list<CMail> ListMail(const string &folderName,
												 const CTimeStamp &from,
												 const CTimeStamp &to) const
		{
			list<CMail> res;
			set<CMail>::iterator itFrom;
			set<CMail>::iterator itTo;

			if (m_mailMap.count(folderName) == 0) return res;
			CMail fromMail = CMail(from, "", CMailBody(0, ""), NULL);
			CMail toMail = CMail(to, "", CMailBody(0, ""), NULL);
			itFrom = m_mailMap.at(folderName).lower_bound(fromMail);
			itTo = m_mailMap.at(folderName).upper_bound(toMail);

			res.insert(res.begin(), itFrom, itTo);
			return res;
		}

		set<string> ListAddr(const CTimeStamp &from,
												 const CTimeStamp &to) const
		{
			set<CMail>::iterator itFrom;
			set<CMail>::iterator itTo;
			set<string> res;

			CMail fromMail = CMail(from, "", CMailBody(0, ""), NULL);
			CMail toMail = CMail(to, "", CMailBody(0, ""), NULL);

			for (auto const &mapIt: m_mailMap)
			{
				itFrom = mapIt.second.lower_bound(fromMail);
				itTo = mapIt.second.upper_bound(toMail);
				while (itFrom != itTo)
				{
					res.insert(itFrom->From());
					itFrom++;
				}
			}
			return res;
		}

private:
		unordered_map<string, multiset<CMail> > m_mailMap;
};
//=================================================================================================
#ifndef __PROGTEST__

static string showMail(const list<CMail> &l)
{
	ostringstream oss;
	for (const auto &x : l)
		oss << x << endl;
	return oss.str();
}

static string showUsers(const set<string> &s)
{
	ostringstream oss;
	for (const auto &x : s)
		oss << x << endl;
	return oss.str();
}

int main(void)
{
	list<CMail> mailList;
	set<string> users;
	CAttach *att;

	CMailBox m0;
	assert (m0.Delivery(
					CMail(CTimeStamp(2014, 3, 31, 15, 24, 13), "user1@fit.cvut.cz", CMailBody(14, "mail content 1"), nullptr)));
	assert (m0.Delivery(
					CMail(CTimeStamp(2014, 3, 31, 15, 26, 23), "user2@fit.cvut.cz", CMailBody(22, "some different content"),
								nullptr)));
	att = new CAttach(200);
	assert (m0.Delivery(
					CMail(CTimeStamp(2014, 3, 31, 11, 23, 43), "boss1@fit.cvut.cz", CMailBody(14, "urgent message"), att)));
	assert (m0.Delivery(
					CMail(CTimeStamp(2014, 3, 31, 18, 52, 27), "user1@fit.cvut.cz", CMailBody(14, "mail content 2"), att)));
	att->Release();
	att = new CAttach(97);
	assert (m0.Delivery(
					CMail(CTimeStamp(2014, 3, 31, 16, 12, 48), "boss1@fit.cvut.cz", CMailBody(24, "even more urgent message"),
								att)));
	att->Release();

	/*cout << showMail ( m0 . ListMail ( "inbox",
	                           CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
	                           CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) );*/

	assert (showMail(m0.ListMail("inbox",
															 CTimeStamp(2000, 1, 1, 0, 0, 0),
															 CTimeStamp(2050, 12, 31, 23, 59, 59))) ==
					"2014-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B + attachment: 200 B\n"
					"2014-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
					"2014-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B\n"
					"2014-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B + attachment: 97 B\n"
					"2014-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 200 B\n");

	/*cout << showMail ( m0 . ListMail ( "inbox",
																		 CTimeStamp ( 2014, 3, 31, 15, 26, 23 ),
																		 CTimeStamp ( 2014, 3, 31, 16, 12, 48 ) )  );*/

	assert (showMail(m0.ListMail("inbox",
															 CTimeStamp(2014, 3, 31, 15, 26, 23),
															 CTimeStamp(2014, 3, 31, 16, 12, 48))) ==
					"2014-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B\n"
					"2014-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B + attachment: 97 B\n");

	/*cout << showUsers (m0 . ListAddr ( CTimeStamp ( 2000, 1, 1, 0, 0, 0 ),
	                CTimeStamp ( 2050, 12, 31, 23, 59, 59 ) ) );*/

	assert (showUsers(m0.ListAddr(CTimeStamp(2000, 1, 1, 0, 0, 0),
																CTimeStamp(2050, 12, 31, 23, 59, 59))) == "boss1@fit.cvut.cz\n"
																																					"user1@fit.cvut.cz\n"
																																					"user2@fit.cvut.cz\n");

	/*cout <<  ( showUsers ( m0 . ListAddr ( CTimeStamp ( 2014, 3, 31, 15, 26, 23 ),
	                       CTimeStamp ( 2014, 3, 31, 16, 12, 48 ) ) ) );*/

	assert (showUsers(m0.ListAddr(CTimeStamp(2014, 3, 31, 15, 26, 23),
																CTimeStamp(2014, 3, 31, 16, 12, 48))) == "boss1@fit.cvut.cz\n"
																																				 "user2@fit.cvut.cz\n");

	CMailBox m1;
	assert (m1.NewFolder("work"));
	assert (m1.NewFolder("spam"));
	assert (!m1.NewFolder("spam"));
	assert (m1.Delivery(
					CMail(CTimeStamp(2014, 3, 31, 15, 24, 13), "user1@fit.cvut.cz", CMailBody(14, "mail content 1"), nullptr)));
	att = new CAttach(500);
	assert (m1.Delivery(
					CMail(CTimeStamp(2014, 3, 31, 15, 26, 23), "user2@fit.cvut.cz", CMailBody(22, "some different content"),
								att)));
	att->Release();
	assert (m1.Delivery(
					CMail(CTimeStamp(2014, 3, 31, 11, 23, 43), "boss1@fit.cvut.cz", CMailBody(14, "urgent message"), nullptr)));
	att = new CAttach(468);
	assert (m1.Delivery(
					CMail(CTimeStamp(2014, 3, 31, 18, 52, 27), "user1@fit.cvut.cz", CMailBody(14, "mail content 2"), att)));
	att->Release();
	assert (m1.Delivery(
					CMail(CTimeStamp(2014, 3, 31, 16, 12, 48), "boss1@fit.cvut.cz", CMailBody(24, "even more urgent message"),
								nullptr)));
	assert (showMail(m1.ListMail("inbox",
															 CTimeStamp(2000, 1, 1, 0, 0, 0),
															 CTimeStamp(2050, 12, 31, 23, 59, 59))) ==
					"2014-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
					"2014-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
					"2014-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
					"2014-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
					"2014-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n");
	assert (showMail(m1.ListMail("work",
															 CTimeStamp(2000, 1, 1, 0, 0, 0),
															 CTimeStamp(2050, 12, 31, 23, 59, 59))) == "");
	assert (m1.MoveMail("inbox", "work"));
	assert (showMail(m1.ListMail("inbox",
															 CTimeStamp(2000, 1, 1, 0, 0, 0),
															 CTimeStamp(2050, 12, 31, 23, 59, 59))) == "");
	assert (showMail(m1.ListMail("work",
															 CTimeStamp(2000, 1, 1, 0, 0, 0),
															 CTimeStamp(2050, 12, 31, 23, 59, 59))) ==
					"2014-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
					"2014-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
					"2014-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
					"2014-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
					"2014-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n");
	assert (m1.Delivery(
					CMail(CTimeStamp(2014, 3, 31, 19, 24, 13), "user2@fit.cvut.cz", CMailBody(14, "mail content 4"), nullptr)));
	att = new CAttach(234);
	assert (m1.Delivery(CMail(CTimeStamp(2014, 3, 31, 13, 26, 23), "user3@fit.cvut.cz", CMailBody(9, "complains"), att)));
	att->Release();
	assert (showMail(m1.ListMail("inbox",
															 CTimeStamp(2000, 1, 1, 0, 0, 0),
															 CTimeStamp(2050, 12, 31, 23, 59, 59))) ==
					"2014-03-31 13:26:23 user3@fit.cvut.cz mail body: 9 B + attachment: 234 B\n"
					"2014-03-31 19:24:13 user2@fit.cvut.cz mail body: 14 B\n");
	assert (showMail(m1.ListMail("work",
															 CTimeStamp(2000, 1, 1, 0, 0, 0),
															 CTimeStamp(2050, 12, 31, 23, 59, 59))) ==
					"2014-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
					"2014-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
					"2014-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
					"2014-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
					"2014-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n");
	assert (m1.MoveMail("inbox", "work"));
	assert (showMail(m1.ListMail("inbox",
															 CTimeStamp(2000, 1, 1, 0, 0, 0),
															 CTimeStamp(2050, 12, 31, 23, 59, 59))) == "");
	assert (showMail(m1.ListMail("work",
															 CTimeStamp(2000, 1, 1, 0, 0, 0),
															 CTimeStamp(2050, 12, 31, 23, 59, 59))) ==
					"2014-03-31 11:23:43 boss1@fit.cvut.cz mail body: 14 B\n"
					"2014-03-31 13:26:23 user3@fit.cvut.cz mail body: 9 B + attachment: 234 B\n"
					"2014-03-31 15:24:13 user1@fit.cvut.cz mail body: 14 B\n"
					"2014-03-31 15:26:23 user2@fit.cvut.cz mail body: 22 B + attachment: 500 B\n"
					"2014-03-31 16:12:48 boss1@fit.cvut.cz mail body: 24 B\n"
					"2014-03-31 18:52:27 user1@fit.cvut.cz mail body: 14 B + attachment: 468 B\n"
					"2014-03-31 19:24:13 user2@fit.cvut.cz mail body: 14 B\n");

	return 0;
}

#endif /* __PROGTEST__ */
