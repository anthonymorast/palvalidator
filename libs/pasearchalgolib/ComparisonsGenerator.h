#ifndef COMPARISONSGENERATOR_H
#define COMPARISONSGENERATOR_H

#include <boost/circular_buffer.hpp>
#include <map>
#include <vector>
#include <set>
#include <unordered_set>
#include <iostream>
#include "ComparableBar.h"
#include <functional>

///
///Extend the (std) hash functionality to take std::array hashing
///

template <class T>
inline void hash_combine(std::size_t& seed, const T& v)
{
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed<<6) + (seed>>2);
}

template<typename T, std::size_t S>
struct std::hash< std::array< T, S > >
{
  size_t operator() (const std::array< T, S > & arrayKey) const
    {
        std::size_t seed = 0;
        for (int i = 0; i < arrayKey.size(); ++i)
            hash_combine(seed, arrayKey[i]);

        return seed;
    }
};


namespace mkc_searchalgo
{
  //a simplified type to represent bar to bar comparison
  using ComparisonEntryType = std::array<unsigned int, 4>;

  template <class Decimal> class ComparisonsGenerator
  {
  public:
    explicit ComparisonsGenerator(unsigned int maxlookback):
      mDateIndex(0),
      mMaxLookBack(maxlookback),
      mBarBuffer(maxlookback),  //circular buffer instantiation
      mComparisonsCount(0),
      mComparisonsBatches{{mDateIndex, {}}}
    {}

    const std::unordered_map<unsigned int, std::unordered_set<ComparisonEntryType>>& getComparisons() const { return mComparisonsBatches; }

    const std::set<ComparisonEntryType>& getUniqueComparisons() const { return mUniqueComparisons; }

    const unsigned int& getComparisonsCount() const { return mComparisonsCount; }

    void addNewLastBar(const Decimal& open, const Decimal& high, const Decimal& low, const Decimal& close)
    {
      //create new last bar (0 indexed)
      ComparableBar<Decimal, 4> lastBar(open, high, low, close, 0);
      //shift up all bar offsets in circular buffer
      shiftBarsUp();
      //add to circular buffer (because we will self-compare, too)
      mBarBuffer.push_back(lastBar);
//      std::cout << "bar buffer now has " << mBarBuffer.size() << " elements " << std::endl;
//      for (int i = 0; i < mBarBuffer.size(); ++i)
//        {
//          std::cout << mBarBuffer[i] << std::endl;
//        }
      runCompare();
      mDateIndex++;
      newComparisonsBatch();
    }

  private:

    template <class Comparable>
    void compareWith(const Comparable& first, const Comparable& second)
    {
      const auto& fOhlcArr = first.getOhlcArr();
      const auto& sOhlcArr = second.getOhlcArr();

      bool same = first.getOffset() == second.getOffset();
      if (same && !(first == second))
          throw;

      for (int i = 0; i < fOhlcArr.size(); ++i)
        {
          for (int c = 0; c < fOhlcArr.size(); ++c)
            {
              // self-checking case (high- and low- based comparison, or same-to-same does not make sense)
              if (same && (c == 1 || i == 1 || c == 2 || i == 2 || c == i) )
                continue;

              if (fOhlcArr[i] > sOhlcArr[c])
                {
                  addComparison(first.getOffset(), i, second.getOffset(), c);
                }
              else if (fOhlcArr[i] < sOhlcArr[c])
                {
                  addComparison(second.getOffset(), c, first.getOffset(), i);
                }
            }
        }
    }

    void addComparison(unsigned int fOffset, unsigned int fOhlcId, unsigned int sOffset, unsigned int sOhlcId)
    {
      ComparisonEntryType compEntry = {fOffset, fOhlcId, sOffset, sOhlcId};
      mComparisonsBatches[mDateIndex].insert(compEntry);
      mComparisonsCount++;
      mUniqueComparisons.insert(compEntry);
    }
    //compare last bar with all bars (including itself)
    void runCompare()
    {
      const ComparableBar<Decimal, 4>& lastBar = mBarBuffer.back();
      for (int i = mBarBuffer.size() -1; i --> 0;)
          compareWith(lastBar, mBarBuffer[i]);

    }

    void shiftBarsUp()
    {
      for (int i = 0; i < mBarBuffer.size(); ++i)
        mBarBuffer[i].incrementOffset();
    }

    void newComparisonsBatch()
    {
      std::unordered_set<ComparisonEntryType, std::hash<ComparisonEntryType>> newBatch{};
      //increment offsets in old comparisons
      for (const auto& entry: mComparisonsBatches[mDateIndex - 1])
        {
          ComparisonEntryType newEntry = entry;
          newEntry[0]++;
          newEntry[2]++;

          //erase/disregard comparisons outside of lookback window
          if ( (newEntry)[0] <= mMaxLookBack - 1 && (newEntry)[2] <= mMaxLookBack -1 )
            {
              newBatch.insert(newEntry);
              mUniqueComparisons.insert(newEntry);
            }
        }
      mComparisonsBatches[mDateIndex] = newBatch;
    }

    unsigned int mDateIndex;
    unsigned int mMaxLookBack;
    unsigned int mComparisonsCount;
    boost::circular_buffer<ComparableBar<Decimal, 4>> mBarBuffer;
    std::unordered_map<unsigned int, std::unordered_set<ComparisonEntryType, std::hash<ComparisonEntryType> >>  mComparisonsBatches;
    std::set<ComparisonEntryType> mUniqueComparisons;
  };
}

#endif // COMPARISONSGENERATOR_H
