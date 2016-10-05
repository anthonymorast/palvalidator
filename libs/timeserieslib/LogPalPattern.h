// Copyright (C) MKC Associates, LLC - All Rights Reserved
// Unauthorized copying of this file, via any medium is strictly prohibited
// Proprietary and confidential
// Written by Michael K. Collison <collison956@gmail.com>, July 2016
//
#ifndef __LOG_PAL_PATTERN_H
#define __LOG_PAL_PATTERN_H 1

#include <fstream>
#include <memory>
#include "PalAst.h"

namespace mkc_timeseries
{
  using std::shared_ptr;

  class LogPalPattern
  {
  public:
    static void LogPattern (std::shared_ptr<PriceActionLabPattern> pattern,
			    std::ofstream& outputFileStream);
  private:
    LogPalPattern();

  private:
    static void LogPatternDescription (std::shared_ptr<PatternDescription> desc,
				       std::ofstream& outputFileStream);
    static void LogExpression (PatternExpression *expression,
			       std::ofstream& outputFileStream);
    static void LogPriceBarExpr (PriceBarReference *barReference,
				 std::ofstream& outputFileStream);
    static void LogMarketExpression (MarketEntryExpression *expression,
				     std::ofstream& outputFileStream);
    static void LogProfitTarget (ProfitTargetInPercentExpression *expression,
				 std::ofstream& outputFileStream);
    static void LogStopLoss (StopLossInPercentExpression *expression,
			     std::ofstream& outputFileStream);
    static void LogPatternSeparator(std::ofstream& outputFileStream);
  };


}

#endif
