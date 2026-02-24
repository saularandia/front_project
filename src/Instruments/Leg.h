#ifndef INSTRUMENTS_LEG_H
#define INSTRUMENTS_LEG_H

#include <Instruments/CashFlow.h>

#include <vector>

class Leg
{
public:
    virtual ~Leg() = default;

    virtual std::vector<CashFlow> cashflows() const = 0;
    virtual double price() const = 0;
};

#endif
