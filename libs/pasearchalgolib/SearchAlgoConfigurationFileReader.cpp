

#include <boost/algorithm/string.hpp>
#include <boost/filesystem.hpp>
#include "SearchAlgoConfigurationFileReader.h"
#include "PalParseDriver.h"
#include "TimeFrameUtility.h"
#include "TimeSeriesEntry.h"
#include "TimeSeriesCsvReader.h"
#include "SecurityAttributes.h"
#include "SecurityAttributesFactory.h"
#include <cstdio>
#include "number.h"

using namespace boost::filesystem;
//extern PriceActionLabSystem* parsePALCode();
//extern FILE *yyin;

using Decimal = num::DefaultNumber;

using namespace mkc_timeseries;

namespace mkc_searchalgo
{

  SearchAlgoConfigurationFileReader::SearchAlgoConfigurationFileReader (const std::string& configurationFileName)
    : mConfigurationFileName(configurationFileName)
  {}

  std::shared_ptr<SearchAlgoConfiguration<Decimal>> SearchAlgoConfigurationFileReader::readConfigurationFile()
  {
    io::CSVReader<9> csvConfigFile(mConfigurationFileName.c_str());

    csvConfigFile.set_header("Symbol", "IRPath", "DataPath","FileFormat","ISDateStart",
			     "ISDateEnd", "OOSDateStart", "OOSDateEnd", "TimeFrame");

    std::string tickerSymbol, palIRFilePathStr, historicDataFilePathStr, historicDataFormatStr;
    std::string inSampleStartDate, inSampleEndDate, oosStartDate, oosEndDate;
    std::string timeFrameStr;

    boost::gregorian::date insampleDateStart, insampleDateEnd, oosDateStart, oosDateEnd;

    csvConfigFile.read_row (tickerSymbol, palIRFilePathStr, historicDataFilePathStr,
			    historicDataFormatStr, inSampleStartDate, inSampleEndDate,
			    oosStartDate, oosEndDate, timeFrameStr);


//    insampleDateStart = boost::gregorian::from_undelimited_string(inSampleStartDate);
//    insampleDateEnd = boost::gregorian::from_undelimited_string(inSampleEndDate);

//    DateRange inSampleDates(insampleDateStart, insampleDateEnd);

//    oosDateStart = boost::gregorian::from_undelimited_string(oosStartDate);
//    oosDateEnd = boost::gregorian::from_undelimited_string(oosEndDate);

//    DateRange ooSampleDates( oosDateStart, oosDateEnd);

//    if (oosDateStart <= insampleDateEnd)
//      std::cout << "******** Warning OOS start date is before IS start date **********" << std::endl << std::endl;
//    //throw McptConfigurationFileReaderException("McptConfigurationFileReader::readConfigurationFile - OOS start date starts before insample end date");

//    boost::filesystem::path irFilePath (palIRFilePathStr);

//    if (!exists (irFilePath))
//      throw McptConfigurationFileReaderException("PAL IR path " +irFilePath.string() +" does not exist");

//    boost::filesystem::path historicDataFilePath (historicDataFilePathStr);
//    if (!exists (historicDataFilePath))
//      throw McptConfigurationFileReaderException("Historic data file path " +historicDataFilePath.string() +" does not exist");

//    std::shared_ptr<SecurityAttributes<Decimal>> attributes = createSecurityAttributes (tickerSymbol);
//    TimeFrame::Duration backTestingTimeFrame = getTimeFrameFromString(timeFrameStr);

//    std::shared_ptr<TimeSeriesCsvReader<Decimal>> reader = getHistoricDataFileReader(historicDataFilePathStr,
//										     historicDataFormatStr,
//										     backTestingTimeFrame,
//										     getVolumeUnit(attributes),
//										     attributes->getTick());
//    reader->readFile();

//    //  insampleDateStart
//    boost::gregorian::date timeSeriesStartDate = reader->getTimeSeries()->getFirstDate();
//    if (insampleDateStart < timeSeriesStartDate)
//      {
//	boost::gregorian::date_period daysBetween(insampleDateStart, timeSeriesStartDate);
//	if (daysBetween.length().days() > 10)
//	  {
//	    std::string inSampleDateStr(boost::gregorian::to_simple_string (insampleDateStart));
//	    std::string timeSeriesDateStr(boost::gregorian::to_simple_string (timeSeriesStartDate));

//	    throw McptConfigurationFileReaderException (std::string("Number of days between configuration file IS start date of ") +inSampleDateStr +std::string(" and TimeSeries start date of ") +timeSeriesDateStr +std::string(" is greater than 10 days"));
//	  }
//      }

//    // Constructor driver (facade) that will parse the IR and return
//    // and AST representation
//    mkc_palast::PalParseDriver driver (irFilePath.string());

//    // Read the IR file

//    driver.Parse();

//    std::cout << "Parsing successfully completed." << std::endl << std::endl;
//    PriceActionLabSystem* system = driver.getPalStrategies();
//    std::cout << "Total number IR patterns = " << system->getNumPatterns() << std::endl;
//    std::cout << "Total long IR patterns = " << system->getNumLongPatterns() << std::endl;
//    std::cout << "Total short IR patterns = " << system->getNumShortPatterns() << std::endl;
//    //yyin = fopen (irFilePath.string().c_str(), "r");
//    //PriceActionLabSystem* system = parsePALCode();

//    //fclose (yyin);

//    return std::make_shared<McptConfiguration<Decimal>>(getBackTester(backTestingTimeFrame, ooSampleDates),
//						  getBackTester(backTestingTimeFrame, inSampleDates),
//						  createSecurity (attributes, reader),
//						  system, inSampleDates, ooSampleDates);
//  }

//  static std::shared_ptr<BackTester<Decimal>> getBackTester(TimeFrame::Duration theTimeFrame,
//							 const DateRange& backtestingDates)
//  {
//    if (theTimeFrame == TimeFrame::DAILY)
//      return std::make_shared<DailyBackTester<Decimal>>(backtestingDates.getFirstDate(),
//						  backtestingDates.getLastDate());
//    else if (theTimeFrame == TimeFrame::WEEKLY)
//      return std::make_shared<WeeklyBackTester<Decimal>>(backtestingDates.getFirstDate(),
//						   backtestingDates.getLastDate());
//    else if (theTimeFrame == TimeFrame::MONTHLY)
//      return std::make_shared<MonthlyBackTester<Decimal>>(backtestingDates.getFirstDate(),
//						    backtestingDates.getLastDate());
//    else
//      throw McptConfigurationFileReaderException("getBacktester - cannot create backtester for time frame other than daily or monthly");
//  }

//  static std::shared_ptr<mkc_timeseries::Security<Decimal>>
//  createSecurity (std::shared_ptr<SecurityAttributes<Decimal>> attributes,
//		  std::shared_ptr<TimeSeriesCsvReader<Decimal>> aReader)
//  {
//    if (attributes->isEquitySecurity())
//      {
//	if (attributes->isFund())
//	  {
//	    return std::make_shared<EquitySecurity<Decimal>>(attributes->getSymbol(),
//						       attributes->getName(),
//						       aReader->getTimeSeries());
//	  }
//	else if (attributes->isCommonStock())
//	  {
//	    return std::make_shared<EquitySecurity<Decimal>>(attributes->getSymbol(),
//						       attributes->getName(),
//						       aReader->getTimeSeries());
//	  }
//	else
//	  throw McptConfigurationFileReaderException("Unknown security attribute");
//      }
//    else
//      return std::make_shared<FuturesSecurity<Decimal>>(attributes->getSymbol(),
//						  attributes->getName(),
//						  attributes->getBigPointValue(),
//						  attributes->getTick(),
//						  aReader->getTimeSeries());

  }

}
