#ifndef SHORTCUTSEARCHALGOBACKTESTER_H
#define SHORTCUTSEARCHALGOBACKTESTER_H

#include "McptConfigurationFileReader.h"
#include <string>
#include <vector>
#include <memory>
#include <stdio.h>

//#include "McptConfigurationFileReader.h"
#include "PALMonteCarloValidation.h"
//#include "RobustnessTester.h"
//#include "LogPalPattern.h"
//#include "LogRobustnessTest.h"
//#include "number.h"
//#include <cstdlib>
//#include "ComparisonsGenerator.h"
//#include "UniqueSinglePAMatrix.h"
#include "ComparisonsCombiner.h"
//#include <map>
#include <algorithm>

using namespace mkc_timeseries;
using namespace mkc_searchalgo;
using std::shared_ptr;
using Decimal = num::DefaultNumber;


namespace mkc_searchalgo {

  enum ShortcutBacktestMethod {PlainVanilla, Pyramiding};

  ///
  /// Backteser that uses vector multiplication and non-pyramiding implementation of backtest
  ///
  template <class Decimal, ShortcutBacktestMethod> class ShortcutSearchAlgoBacktester
  {
  public:
    ShortcutSearchAlgoBacktester(const std::valarray<Decimal>& backtestResults, const std::valarray<unsigned int>& numBarsInPosition, unsigned int minTrades, bool isLong):
      mBacktestResultBase(backtestResults),
      mNumBarsInPosition(numBarsInPosition),
      mMinTrades(minTrades),
      mNumTrades(0),
      mIsLong(isLong)
    {}

    void backtest2(const std::valarray<Decimal>& occurences);
    void backtest(const std::vector<std::valarray<Decimal>>& compareContainer);

    Decimal getProfitFactor() const
    {
      if (mNumTrades < mMinTrades)
        return DecimalConstants<Decimal>::DecimalZero;

      if ((mNumWinners >= 1) and (mNumLosers >= 1))
        return (mSumWinners / num::abs(mSumLosers));
      else if (mNumWinners == 0)
        return (DecimalConstants<Decimal>::DecimalZero);
      else if (mNumLosers == 0)
        return (DecimalConstants<Decimal>::DecimalOneHundred);
      else
        throw std::logic_error(std::string("SearchBacktestPolicy:getProfitFactor - getNumPositions > 0 error"));
    }

  private:
    const std::valarray<Decimal>& mBacktestResultBase;
    const std::valarray<unsigned int>& mNumBarsInPosition;
    unsigned int mMinTrades;
    unsigned int mNumTrades;
    Decimal mSumWinners;
    Decimal mSumLosers;
    unsigned int mNumWinners;
    unsigned int mNumLosers;
    bool mIsLong;
  };


  template <>
  //void ShortcutSearchAlgoBacktester<Decimal, ShortcutBacktestMethod::PlainVanilla>::backtest(const std::valarray<Decimal>& occurences)
  void ShortcutSearchAlgoBacktester<Decimal, ShortcutBacktestMethod::PlainVanilla>::backtest(const std::vector<std::valarray<Decimal>>& compareContainer)
  {
    std::valarray<Decimal> occurrences(DecimalConstants<Decimal>::DecimalOne, compareContainer.back().size());
    for (auto it = compareContainer.begin(); it != compareContainer.end(); ++it)
      {
        occurrences *= *it;
      }

    if (occurrences.size() != mBacktestResultBase.size())
      throw;

    std::valarray<Decimal> allResults = occurrences * mBacktestResultBase;
    int nextSkipStart = -1;
    int nextSkipEnd = -1;
    for (int i = 0; i < allResults.size(); i++)
      {
        //the skip procedure
        if (i >= nextSkipStart && i <= nextSkipEnd)
          {
            allResults[i] = DecimalConstants<Decimal>::DecimalZero;
            if (i == nextSkipEnd)
              {
                nextSkipStart = -1;
                nextSkipEnd = -1;
              }
          }
        //the signal identification procedure
        const Decimal& res = allResults[i];
        if (res != DecimalConstants<Decimal>::DecimalZero)
          {
            mNumTrades++;
            if (res > DecimalConstants<Decimal>::DecimalZero)
              {
                mNumWinners++;
                mSumWinners += res;
              }
            if (res < DecimalConstants<Decimal>::DecimalZero)
              {
                mNumLosers++;
                mSumLosers += res;
              }

            nextSkipStart = i + 1;
            nextSkipEnd = i + mNumBarsInPosition[i];
          }
      }
  }

  template <>
  void ShortcutSearchAlgoBacktester<Decimal, ShortcutBacktestMethod::Pyramiding>::backtest2(const std::valarray<Decimal>& occurrences)
  {
    if (occurrences.size() != mBacktestResultBase.size())
      throw;

    std::valarray<Decimal> allResults = occurrences * mBacktestResultBase;
    int nextSkipStart = -1;
    int nextSkipEnd = -1;
    for (int i = 0; i < allResults.size(); i++)
      {
        //the signal identification procedure, without skipping
        const Decimal& res = allResults[i];
        if (res != DecimalConstants<Decimal>::DecimalZero)
          {
            mNumTrades++;
            if (res > DecimalConstants<Decimal>::DecimalZero)
              {
                mNumWinners++;
                mSumWinners += res;
              }
            if (res < DecimalConstants<Decimal>::DecimalZero)
              {
                mNumLosers++;
                mSumLosers += res;
              }
          }
      }
  }



  template <class Decimal, bool isLong> class OriginalBacktestPolicy
  {
    public:

    void backtest(const std::valarray<Decimal>& occurences);

    Decimal getProfitFactor() const;
  };

}

#endif // SHORTCUTSEARCHALGOBACKTESTER_H
