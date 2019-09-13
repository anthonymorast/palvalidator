// Copyright Tibor Szlavik for use by (C) MKC Associates, LLC
// All Rights Reserved
// Unauthorized copying of this file, via any medium is strictly prohibited
// Proprietary and confidential
// Written by Tibor Szlavik <seg2019s@gmail.com>, July-August 2019

#include "SearchAlgoConfigurationFileReader.h"


using namespace boost::filesystem;
//extern PriceActionLabSystem* parsePALCode();
//extern FILE *yyin;

using Decimal = num::DefaultNumber;

using namespace mkc_timeseries;

namespace mkc_searchalgo
{

  static TradingVolume::VolumeUnit getVolumeUnit (std::shared_ptr<Security<Decimal>> security)
  {
    if (security->isEquitySecurity())
      return TradingVolume::SHARES;
    else
      return TradingVolume::CONTRACTS;
  }

  template <class T>
  static T tryCast(std::string inputString)
  {
    try
    {
      return boost::lexical_cast<T>(inputString);
    }
    catch (const boost::bad_lexical_cast & e)
    {
      std::string exceptionStr(std::string("Exception caught when trying to cast: ") + inputString + std::string(" as ") + typeid(T).name() + std::string("."));
      std::cout << exceptionStr << "\nException: " << e.what() << "!" << std::endl;
      throw SearchAlgoConfigurationFileReaderException(exceptionStr);
    }
    catch (const std::exception& e)
    {
      throw SearchAlgoConfigurationFileReaderException(std::string("Undefined exception encountered in tryCast. Exception details: " + std::string(e.what())));
    }
  }


  SearchAlgoConfigurationFileReader::SearchAlgoConfigurationFileReader (const std::string& configurationFileName)
    : mConfigurationFileName(configurationFileName)
  {}

  std::shared_ptr<SearchAlgoConfiguration<Decimal>> SearchAlgoConfigurationFileReader::readConfigurationFile(const std::shared_ptr<Security<Decimal>>& security, int timeFrameIdToLoad)
  {
    std::cout << "Time frame id started: " << timeFrameIdToLoad << std::endl;
    //io::CSVReader<8, io::trim_chars<' '>, io::double_quote_escape<',','\"'>> mCsvFile;
    io::CSVReader<11, io::trim_chars<' '>, io::double_quote_escape<',','\"'>> csvConfigFile(mConfigurationFileName.c_str());

//    csvConfigFile.set_header("MaxDepth", "MinTrades", "SortMultiplier","PassingStratNumPerRound","ProfitFactorCriterion", "MaxConsecutiveLosers",
//                             "MaxInactivitySpan", "TargetsToSearchConfigFilePath", "TimeFramesToSearchConfigFilePath","HourlyDataFilePath", "ValidationConfigFilePath");

    csvConfigFile.read_header(io::ignore_extra_column, "MaxDepth", "MinTrades", "SortMultiplier","PassingStratNumPerRound","ProfitFactorCriterion", "MaxConsecutiveLosers",
                             "MaxInactivitySpan", "TargetsToSearchConfigFilePath", "TimeFramesToSearchConfigFilePath","HourlyDataFilePath", "ValidationConfigFilePath");

    std::string maxDepth, minTrades, sortMultiplier, passingStratNumPerRound, profitFactorCritierion, maxConsecutiveLosers;
    std::string maxInactivitySpan, targetsToSearchConfigFilePath, timeFramesToSearchConfigFilePath, hourlyDataFilePath, validationConfigFilePath;


    csvConfigFile.read_row (maxDepth, minTrades, sortMultiplier, passingStratNumPerRound, profitFactorCritierion, maxConsecutiveLosers,
                            maxInactivitySpan, targetsToSearchConfigFilePath, timeFramesToSearchConfigFilePath, hourlyDataFilePath, validationConfigFilePath);


    boost::filesystem::path validationFile (validationConfigFilePath);
    if (!exists (validationFile))
      throw SearchAlgoConfigurationFileReaderException("Validation config file path: " + validationFile.string() + " does not exist");

    std::string numPermutations, numStratsFull, numStratsBeforeValidation;
    io::CSVReader<3, io::trim_chars<' '>, io::double_quote_escape<',','\"'>> validationCsv(validationConfigFilePath);
    //validationCsv.set_header("NumPermutations", "NumStratsFullPeriod", "NumStratsBeforeValidation");
    validationCsv.read_header(io::ignore_extra_column, "NumPermutations", "NumStratsFullPeriod", "NumStratsBeforeValidation");
    validationCsv.read_row(numPermutations, numStratsFull, numStratsBeforeValidation);


    boost::filesystem::path targetsFile (targetsToSearchConfigFilePath);
    if (!exists (targetsFile))
      throw SearchAlgoConfigurationFileReaderException("Targets to search config file path: " + targetsFile.string() + " does not exist");

    std::vector<std::pair<Decimal, Decimal>> targetStops;
    io::CSVReader<2, io::trim_chars<' '>, io::double_quote_escape<',','\"'>> targetsCsv(targetsToSearchConfigFilePath);
    //targetsCsv.set_header("TargetMultiplier", "StopMultiplier");
    targetsCsv.read_header(io::ignore_extra_column, "TargetMultiplier", "StopMultiplier");

    std::string target, stop;
    while (targetsCsv.read_row(target, stop))
      {
        targetStops.push_back(std::make_pair(Decimal(tryCast<float>(target)), Decimal(tryCast<float>(stop))));
      }

    boost::filesystem::path timeFramesFile (timeFramesToSearchConfigFilePath);
    if (!exists (timeFramesFile))
      throw SearchAlgoConfigurationFileReaderException("Timeframe to search config file path: " +  timeFramesFile.string() + " does not exist");


    std::vector<time_t> timeFrames;
    io::CSVReader<1, io::trim_chars<' '>, io::double_quote_escape<',','\"'>> timesCsv(timeFramesToSearchConfigFilePath);
    timesCsv.read_header(io::ignore_extra_column, "TimeFrame");

    std::string timeFrame;
    while (timesCsv.read_row(timeFrame))
      {
        try
        {
          struct std::tm tm{0,0,0,0,0,2000,0,0,-1};
          strptime(timeFrame.c_str(), "%H:%M", &tm);
          timeFrames.push_back(std::mktime(&tm));
        }
        catch (const std::exception& e)
        {
          std::cout << "Time conversion exception." << std::endl;
          throw SearchAlgoConfigurationFileReaderException("Time conversion exception in file: " + timeFramesFile.string() + ", when converting: " + timeFrame + "\nException details: " + std::string(e.what()));
        }
      }

    boost::filesystem::path hourlyFile (hourlyDataFilePath);
    if (!exists (hourlyFile))
      throw SearchAlgoConfigurationFileReaderException("Hourly data file path: " +  hourlyFile.string() + " does not exist");

    if (static_cast<size_t>(timeFrameIdToLoad) > timeFrames.size() || timeFrameIdToLoad < 0)
      throw SearchAlgoConfigurationFileReaderException("Invalid timeFrameIdToLoad: " + std::to_string(timeFrameIdToLoad) + " timeframes size: " + std::to_string(timeFrames.size()) + ".");

    std::shared_ptr<OHLCTimeSeries<Decimal>> series;
    if (timeFrameIdToLoad > 0)
      {
        const time_t& timeFilter =  timeFrames.at(static_cast<size_t>(timeFrameIdToLoad - 1));
        std::cout << std::asctime(std::localtime(&timeFilter)) << std::endl;
        std::shared_ptr<TradeStationTimeFilteredCsvReader<Decimal>> timeFilteredCsv = std::make_shared<TradeStationTimeFilteredCsvReader<Decimal>>(hourlyDataFilePath, security->getTimeSeries()->getTimeFrame(), getVolumeUnit(security), security->getTick(), timeFilter);
        timeFilteredCsv->readFile();
        typename OHLCTimeSeries<Decimal>::ConstRandomAccessIterator it = security->getTimeSeries()->beginRandomAccess();

        for (; it != security->getTimeSeries()->endRandomAccess(); it++)
          {
            const Decimal& cOpen = security->getTimeSeries()->getOpenValue (it, 0);
            const Decimal& cHigh = security->getTimeSeries()->getHighValue (it, 0);
            const Decimal& cLow = security->getTimeSeries()->getLowValue (it, 0);
            const Decimal& cClose = security->getTimeSeries()->getCloseValue (it, 0);

            auto dt = security->getTimeSeries()->getDateValue(it, 0);
            if (!timeFilteredCsv->getTimeSeries()->isDateFound(dt))
              {
                std::cout << "FilteredTime csv: adding dt entry: " << dt << " from the original security timeseries (mixing)." << std::endl;
                timeFilteredCsv->addEntry(OHLCTimeSeriesEntry<Decimal>(dt, cOpen, cHigh, cLow, cClose, DecimalConstants<Decimal>::DecimalZero, security->getTimeSeries()->getTimeFrame()));
              }
            else
              {
                std::cout << "First date found in file: " << dt << ", no more mixing." << std::endl;
                break;
              }
          }
        std::cout << "First date random access: " << timeFilteredCsv->getTimeSeries()->getDateValue(timeFilteredCsv->getTimeSeries()->beginRandomAccess(),0) << std::endl;
        std::cout << "First date sorted access: " << timeFilteredCsv->getTimeSeries()->getFirstDate() << std::endl;
        series = timeFilteredCsv->getTimeSeries();
        series->syncronizeMapAndArray();
        std::string outFileName(hourlyDataFilePath + std::string("_timeframe_") + std::to_string(timeFrameIdToLoad));
        PalTimeSeriesCsvWriter<Decimal> tsWriter(outFileName, *series);
        tsWriter.writeFile();

      }
    else
      {
        series = std::make_shared<OHLCTimeSeries<Decimal>>(*security->getTimeSeries());
      }

    return std::make_shared<SearchAlgoConfiguration<Decimal>>(tryCast<unsigned int>(maxDepth),
                                                              tryCast<unsigned int>(minTrades),
                                                              Decimal(tryCast<float>(sortMultiplier)),
                                                              tryCast<unsigned int>(passingStratNumPerRound),
                                                              Decimal(tryCast<float>(profitFactorCritierion)),
                                                              tryCast<unsigned int>(maxConsecutiveLosers),
                                                              tryCast<unsigned int>(maxInactivitySpan),
                                                              targetStops,
                                                              timeFrames,
                                                              series,
                                                              tryCast<unsigned int>(numPermutations),
                                                              tryCast<unsigned int>(numStratsFull),
                                                              tryCast<unsigned int>(numStratsBeforeValidation)
                                                              );

  }

}
