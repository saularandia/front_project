#ifndef INSTRUMENTS_CASHFLOW_H
#define INSTRUMENTS_CASHFLOW_H

#include <boost/date_time/gregorian/gregorian.hpp>

struct CashFlow
{
    using Date = boost::gregorian::date;

    Date pay_date;
    double amount = 0.0; // positive = receive, negative = pay
};

#endif
