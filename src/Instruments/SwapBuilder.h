#ifndef INSTRUMENTS_SWAPBUILDER_H
#define INSTRUMENTS_SWAPBUILDER_H

#include <Instruments/InstrumentDescription.h>
#include <Instruments/IPricer.h>

#include <memory>

class SwapBuilder
{
public:
    static std::unique_ptr<IPricer> build(const InstrumentDescription& instrument);
    static InstrumentDescription::Type getId();
};

#endif
