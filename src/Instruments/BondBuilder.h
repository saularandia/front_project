#ifndef INSTRUMENTS_BONDBUILDER_H
#define INSTRUMENTS_BONDBUILDER_H

#include <Instruments/InstrumentDescription.h>
#include <Instruments/IPricer.h>

#include <memory>

class BondBuilder
{
public:
    static std::unique_ptr<IPricer> build(const InstrumentDescription& instrument);
    static InstrumentDescription::Type getId();
};

#endif
