#ifndef INSTRUMENTS_IPRICER_H
#define INSTRUMENTS_IPRICER_H

class IPricer
{
public:
    virtual ~IPricer() = default;
    virtual double price() const = 0;
};

#endif
