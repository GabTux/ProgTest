/* ------------------------------------ PERSON STRUCT ---------------------------------- */
/** Used to store people, for better manipulation with records
 * this structure is nested inside CTaxRegister class
 */
struct Person
{
  string m_name;
  string m_addr;
  string m_acc;
  long long m_income;
  long long m_outcome;
    
  static bool byNameAddr(const Person& a, const Person& b);
};

/** Function will compare Person structs by name and address
 * 
 * 
 * @param[in] person structs to compare
 * @return =false first higher, =true first lower
 */
bool Person::byNameAddr(const Person& a, const Person& b)
{
  if (a.m_name == b.m_name)
    return a.m_addr < b.m_addr;
  return a.m_name < b.m_name;
}

/* ----------------------------------- CITERATOR CLASS -------------------------------- */
/** Default class, as it is in assigment
 * main idea: get reference of vector with people from CTaxRegister class
 * this is done in constructor
 */
class CIterator
{
  public:
    CIterator(const vector<Person>&);
    bool AtEnd(void) const;
    void Next(void);
    string Name(void) const;
    string Addr(void) const;
    string Account(void) const;
  private:
    unsigned long long m_idx;
    unsigned long long m_peopleCount;
    vector<Person> m_people;
    
};

/* ----------------------------------- CITERATOR METHODS -------------------------------- */
/** Constructor for CIteration
 * 
 * 
 * @param[in] reference to vector with people
 */
CIterator::CIterator(const vector<Person> &people)
{
  m_idx = 0;
  m_peopleCount = people.size();
  m_people = people;
}

/** These trivial functions will be used to:
 * 1) determine if we are at the end of Register
 * 2) move to next person
 * 3) get name of person at actual index
 * 3) get account of person at actual index
 * note: according to cpp reference: operator []
 *       can be used same way for vector as for array
 */
bool CIterator::AtEnd(void) const
{
  return m_idx >= m_peopleCount;
}

void CIterator::Next(void)
{
  m_idx++;
}

string CIterator::Name(void) const
{
  return m_people[m_idx].m_name;
}
string CIterator::Addr(void) const
{
  return m_people[m_idx].m_addr;
}

string CIterator::Account(void) const
{
  return m_people[m_idx].m_acc;
}

/* ----------------------------------- CTAXREGISTER CLASS -------------------------------- */
/** Default class, as it is in assigment
 * main idea: save all records to sorted vector by people's names and addresses
 * C++ STL is used
 * note: lower_bound is LogN
 */
class CTaxRegister
{
  public:
    bool Birth(const string& name, const string& addr, const string& account);
    bool Death(const string& name, const string& addr);
    bool Income(const string& account, int amount);
    bool Income(const string& name, const string& addr, int amount);
    bool Expense(const string& account, int amount);
    bool Expense(const string& name, const string& addr, int amount);
    bool Audit(const string& name, const string& addr, string& account, int& sumIncome, int& sumExpense) const;
    CIterator ListByName(void) const;
  
  private:
    vector<Person> m_people;
    bool exist(const string& account);
};

/* --------------------------- CTAXREGISTER METHODS ------------------------------------- */

/** Function to find person by bank account
 * 
 * 
 * @param[in] account to find
 * @return =false not found, =true found
 */
bool CTaxRegister::exist(const string& account)
{
  for (auto const& idx: m_people)
  {
    if (idx.m_acc == account)
      return true;
  }
  
  return false;
}

/** Function to add person to TaxRegister
 * 
 * it will check if that person already exist in database:
 * 1) check by name and address - LogN
 * 2) check by account - N
 * if none is found, add to sorted vector at appropriate index - N
 * vector.insert(pos, toAdd) - inserts before pos and move whole "array"
 * if needed, vector will automatically realloc
 * 
 * @param[in] name of person to remove
 * @param[in] address  of person to remove
 * @param[in] account  of person to remove
 * @return =false person already registered, =true added
 */
bool CTaxRegister::Birth(const string& name, const string& addr, const string& account)
{
  Person toAdd = {name, addr, account, 0, 0};
  auto resultSearch = lower_bound(m_people.begin(), m_people.end(), toAdd, Person::byNameAddr);
  
  if (resultSearch == m_people.end() || resultSearch->m_name != name || resultSearch->m_addr != addr)
  {
    if (exist(account))
      return false;
    
    m_people.insert(resultSearch, toAdd);
    return true;
  }
  else
    return false;
}

/** Function to remove person from TaxRegister
 * 
 * it will check if that person already exist in database:
 * 1) check by name and address - LogN
 * 2) if person is found, remove from sorted vector at appropriate index - N
 * vector.erase(pos) - remove pos and move whole "array" 
 * 
 * @param[in] name of new person
 * @param[in] address of new person
 * @param[in] account of new person
 * @return =false person not found, =true removed
 */
bool CTaxRegister::Death(const string& name, const string& addr)
{
  Person toKill = {name, addr};
  auto resultSearch = lower_bound(m_people.begin(), m_people.end(), toKill, Person::byNameAddr);
  
  if (resultSearch == m_people.end() || resultSearch->m_name != name || resultSearch->m_addr != addr)
    return false;
  
  m_people.erase(resultSearch);
  return true;
}

/** Function to add income by person account
 * 
 * find that account - N
 * 
 * @param[in] account of person
 * @param[in] amount to add
 * @return =false person not found, =true removed
 */
bool CTaxRegister::Income(const string& account, int amount)
{
  for (auto& idx: m_people)
  {
    if (idx.m_acc == account)
    {
      idx.m_income += amount;
      return true;
    }
  }
  
  return false;
}

/** Function to add income by person name and address
 * 
 * find that person - logN
 * 
 * @param[in] name of person
 * @param[in] address of person
 * @param[in] amount to add
 * @return =false person not found, =true removed
 */
bool CTaxRegister::Income(const string& name, const string& addr, int amount)
{
  Person toFind = {name, addr};
  auto resultSearch = lower_bound(m_people.begin(), m_people.end(), toFind, Person::byNameAddr);
  
  if (resultSearch == m_people.end() || resultSearch->m_name != name || resultSearch->m_addr != addr)
    return false;
  
  resultSearch->m_income += amount;
  return true;
}

/** Function to add expense by person account
 * 
 * find that account - N
 * 
 * @param[in] account of person
 * @param[in] amount to add
 * @return =false person not found, =true removed
 */
bool CTaxRegister::Expense(const string& account, int amount)
{
  for (auto& idx: m_people)
  {
    if (idx.m_acc == account)
    {
      idx.m_outcome += amount;
      return true;
    }
  }
  
  return false;
}

/** Function to add expense by person name and address
 * 
 * find that person - logN
 * 
 * @param[in] name of person
 * @param[in] address of person
 * @param[in] amount to add
 * @return =false person not found, =true removed
 */
bool CTaxRegister::Expense(const string& name, const string& addr, int amount)
{
  Person toFind = {name, addr};
  auto resultSearch = lower_bound(m_people.begin(), m_people.end(), toFind, Person::byNameAddr);
  
  if (resultSearch == m_people.end() || resultSearch->m_name != name || resultSearch->m_addr != addr)
    return false;
  
  resultSearch->m_outcome += amount;
  return true;
}

/** Function to audit person
 * 
 * find that person by binary search - logN
 * 
 * @param[in] name of person
 * @param[in] address of person
 * @param[out] account of that person
 * @param[out] total income of that person
 * @param[out] total expense of that person
 * @return =false person not found, =true removed
 */
bool CTaxRegister::Audit(const string& name, const string& addr, string& account, int& sumIncome, int& sumExpense ) const
{
  Person toFind = {name, addr};
  auto resultSearch = lower_bound(m_people.begin(), m_people.end(), toFind, Person::byNameAddr);
  
  if (resultSearch == m_people.end() || resultSearch->m_name != name || resultSearch->m_addr != addr)
    return false;
  
  account = resultSearch->m_acc;
  sumIncome = resultSearch->m_income;
  sumExpense = resultSearch->m_outcome;
  return true;
}

/** Function to create CIterator instance
 * 
 * 
 * create iterator instance, store vector m_people reference
 * 
 * @return =iterator instance
 */
CIterator CTaxRegister::ListByName(void) const
{
  CIterator it(m_people);
  return it;
}
