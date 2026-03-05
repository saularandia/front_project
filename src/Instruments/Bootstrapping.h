#ifndef INSTRUMENTS_BOOTSTRAPPING_H
#define INSTRUMENTS_BOOTSTRAPPING_H

#include <Instruments/IPricer.h>

#include <boost/date_time/gregorian/gregorian.hpp>

#include <map>
#include <memory>

class Bootstrapping
{
public:
    using Date = boost::gregorian::date;
    using DiscountCurveType = std::map<Date, double>;
    using InstrumentsMapType = std::map<Date, std::shared_ptr<IPricer>>;

    Bootstrapping() = default;

    DiscountCurveType operator()(const InstrumentsMapType& instruments) const;

    virtual ~Bootstrapping() = default;
};

#endif
