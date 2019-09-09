// Copyright Tibor Szlavik for use by (C) MKC Associates, LLC
// All Rights Reserved
// Unauthorized copying of this file, via any medium is strictly prohibited
// Proprietary and confidential
// Written by Tibor Szlavik <seg2019s@gmail.com>, July-August 2019

#ifndef BACKTESTPROCESSOR_H
#define BACKTESTPROCESSOR_H

#include "Sorters.h"

namespace mkc_searchalgo
{
  using StrategyRepresentationType = std::vector<unsigned int>;

  template <class Decimal, typename TSearchAlgoBacktester>
  class BacktestProcessor
  {
  public:
    BacktestProcessor(unsigned minTrades, unsigned maxLosers, unsigned maxInactivity, std::shared_ptr<TSearchAlgoBacktester>& searchAlgoBacktester, const std::shared_ptr<UniqueSinglePAMatrix<Decimal, std::valarray<Decimal>>>& uniques):
      mUniqueId(0),
      mMinTrades(minTrades),
      mMaxLosers(maxLosers),
      mMaxInactivity(maxInactivity),
      mSearchAlgoBacktester(searchAlgoBacktester),
      mResults(),
      mStratMap(),
      mUniques(uniques)
    {}

    void processResult(const StrategyRepresentationType & compareContainer)
    {
      //init to ones
      std::valarray<Decimal> occurences(DecimalConstants<Decimal>::DecimalOne, mUniques->getDateCount());
      //the multiplication
      for (unsigned int el: compareContainer)
        occurences *= mUniques->getMappedElement(el);

      mSearchAlgoBacktester->backtest(occurences);
      const Decimal& pf = mSearchAlgoBacktester->getProfitFactor();
      const Decimal& po = mSearchAlgoBacktester->getPayoffRatio();
      const Decimal& pp = mSearchAlgoBacktester->getPALProfitability();
      unsigned int trades = mSearchAlgoBacktester->getTradeNumber();
      unsigned int maxLosers = mSearchAlgoBacktester->getMaxConsecutiveLosers();
      unsigned int maxInactivity = mSearchAlgoBacktester->getMaxInactivitySpan();

      //pre-filtering, we don't need to keep these results in memory
      if (trades < mMinTrades || maxLosers > mMaxLosers || maxInactivity > mMaxInactivity)
        return;
      mResults.emplace_back(ResultStat<Decimal>(pf, po, pp), trades, mUniqueId);
      mStratMap[mUniqueId] = compareContainer;
      mUniqueId++;
    }


    const std::vector<std::tuple<ResultStat<Decimal>, unsigned int, int>>& getResults() const
    { return mResults; }


    std::unordered_map<int, StrategyRepresentationType>& getStrategyMap()
    { return mStratMap;}


    template <class TSorter>
    void sortResults()
    {
      std::sort(mResults.begin(), mResults.end(), TSorter::sort);
    }

    template <class TSorter>
    void sortResults(Decimal ratio, Decimal multiplier)
    {
      std::cout << "sortResults called with: " << ratio << ", " << multiplier << std::endl;
      std::sort(mResults.begin(), mResults.end(), TSorter(ratio, multiplier));
    }

    void clearAll()
    {
      mResults.clear();
      mResults.shrink_to_fit();
      mStratMap.clear();
    }

  private:

    int mUniqueId;
    unsigned int mMinTrades;
    unsigned int mMaxLosers;
    unsigned int mMaxInactivity;
    std::shared_ptr<TSearchAlgoBacktester> mSearchAlgoBacktester;
    std::vector<std::tuple<ResultStat<Decimal>, unsigned int, int>> mResults;
    std::unordered_map<int, StrategyRepresentationType> mStratMap;
    const std::shared_ptr<UniqueSinglePAMatrix<Decimal, std::valarray<Decimal>>>& mUniques;

  };


}

#endif // BACKTESTPROCESSOR_H
