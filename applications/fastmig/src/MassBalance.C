#include "MassBalance.h"

#include "math.h"

#include <iostream>
#include <iomanip>
using std::setw;
using std::setfill;
#ifdef DEBUG_MASSBALANCE
#endif // DEBUG_MASSBALANCE

const double TOLERANCE = 100;

using namespace DataAccess;

namespace migration { 

#ifdef DEBUG_MASSBALANCE
template <typename STREAM>
int MassBalance<STREAM>::m_highestMassBalanceIndex = 0;
#endif

template <typename STREAM>
MassBalance<STREAM>::MassBalance(STREAM& stream):
   m_massBalanceStream(stream),
   m_massBalance(0.0)
{
#ifdef DEBUG_MASSBALANCE
   m_massBalanceNumber = m_highestMassBalanceIndex;
   ++m_highestMassBalanceIndex;
#endif
}

template <typename STREAM>
void MassBalance<STREAM>::addComment(const string& comment)
{
   m_comments.push_back(comment);
}

template <typename STREAM>
void MassBalance<STREAM>::printMassBalance(const string& name) const
{
   m_massBalanceStream << endl;
   m_massBalanceStream << " ---------------------------------- " << name << " --" << endl;
   m_massBalanceStream << endl;

   printBalance ();
}

template <typename STREAM>
void MassBalance<STREAM>::printMassBalance (const DataAccess::Interface::Snapshot* start,
   const DataAccess::Interface::Snapshot* end,
   const string& name) const
{
   m_massBalanceStream << endl;
   m_massBalanceStream << "-- Snapshot: " << end->getTime () << " Ma ----------------------------- " << name << " --" << endl;
   m_massBalanceStream << endl;

   printBalance ();
}

template <typename STREAM>
void MassBalance<STREAM>::addToBalance (const string & description, double quantity)
{
   m_additionDescriptions.push_back (description);
   m_additionQuantities.push_back (quantity);

#ifdef DEBUG_MASSBALANCE
   cerr << "Because of '" << description << "' add      " << quantity << " to balance " << m_massBalanceNumber << "." << endl;
#endif

   m_massBalance += quantity;
}

template <typename STREAM>
void MassBalance<STREAM>::subtractFromBalance (const string & description, double quantity)
{
   m_subtractionDescriptions.push_back (description);
   m_subtractionQuantities.push_back (quantity);

#ifdef DEBUG_MASSBALANCE
   cerr << "Because of '" << description << "' subtract " << quantity << " from balance " << m_massBalanceNumber << "." << endl;
#endif

   m_massBalance -= quantity;
}

template <typename STREAM>
void MassBalance<STREAM>::printBalance (void) const
{
   for (vector < string >::const_iterator itComment = m_comments.begin (); itComment != m_comments.end (); ++itComment)
      m_massBalanceStream << *itComment;

   vector < string >::const_iterator itDescription;
   vector < double >::const_iterator itQuantity;
   int len;

   double totalIn = 0;

   for (itDescription = m_additionDescriptions.begin (), itQuantity = m_additionQuantities.begin ();
        itDescription != m_additionDescriptions.end (); ++itDescription, ++itQuantity)
   {
      m_massBalanceStream << *itDescription << ":";
      // aligning
      for (len = (*itDescription).size (); len < 50; ++len)
         m_massBalanceStream << " ";

      m_massBalanceStream << "" << setfill(' ') << setw(16) << *itQuantity << " kg" << endl;

      totalIn += *itQuantity;
   }

   m_massBalanceStream << "----------------------------------------------------------------------" << endl;
   m_massBalanceStream << "Total in:";
   // aligning
   for (len = 8; len < 50; ++len)
      m_massBalanceStream << " ";
   m_massBalanceStream << setfill(' ') << setw(16) << totalIn << " kg" << endl << endl;

   double totalOut = 0;

   for (itDescription = m_subtractionDescriptions.begin (), itQuantity = m_subtractionQuantities.begin ();
        itDescription != m_subtractionDescriptions.end (); ++itDescription, ++itQuantity)
   {
      m_massBalanceStream << *itDescription << ":";
      // aligning
      for (len = (*itDescription).size (); len < 50; ++len)
         m_massBalanceStream << " ";
      m_massBalanceStream << "" << setfill(' ') << setw(16) << *itQuantity << " kg" << endl;
      totalOut += *itQuantity;
   }

   m_massBalanceStream << "----------------------------------------------------------------------" << endl;
   m_massBalanceStream << "Total out:";
   // aligning
   for (len = 9; len < 50; ++len)
      m_massBalanceStream << " ";
   m_massBalanceStream << setfill(' ') << setw(16) << totalOut << " kg" << endl << endl;

   m_massBalanceStream << "----------------------------------------------------------------------" << endl;
   m_massBalanceStream << "Balance:";
   // aligning
   for (len = 7; len < 50; ++len)
      m_massBalanceStream << " ";

   m_massBalanceStream << setfill(' ') << setw(16) << totalIn - totalOut << " kg" << endl << endl;
   m_massBalanceStream << "======================================================================" << endl;
   m_massBalanceStream << endl << endl;
}

template <typename STREAM>
double MassBalance<STREAM>::balance(void) const
{
   vector<string>::const_iterator itDescription;
   vector<double>::const_iterator itQuantity;

#ifdef DEBUG_MASSBALANCE
   double totalIn = 0;
   for (itDescription = m_additionDescriptions.begin (), itQuantity = m_additionQuantities.begin ();
	 itDescription != m_additionDescriptions.end ();
	 ++itDescription, ++itQuantity)
   {
      totalIn += * itQuantity;
   }

   double totalOut = 0;
   for (itDescription = m_subtractionDescriptions.begin (), itQuantity = m_subtractionQuantities.begin ();
	 itDescription != m_subtractionDescriptions.end (); ++itDescription, ++itQuantity)
   {
      totalOut += * itQuantity;
   }
   assert(fabs(totalIn - totalOut - m_massBalance) <= TOLERANCE);
#endif

   return m_massBalance;
}

template <typename STREAM>
void MassBalance<STREAM>::clear(void)
{
   m_additionDescriptions.clear ();
   m_additionQuantities.clear ();
   m_subtractionDescriptions.clear ();
   m_subtractionQuantities.clear ();

   m_massBalance = 0.0;

   m_comments.clear();
}

} // namespace migration
