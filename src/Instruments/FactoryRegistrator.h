#ifndef INSTRUMENTS_FACTORYREGISTRATOR_H
#define INSTRUMENTS_FACTORYREGISTRATOR_H

#include <Instruments/Factory.h>

// Registers a BuilderClass in the global singleton Factory at static initialization time.
// BuilderClass must provide:
//   static InstrumentDescription::Type getId();
//   static std::unique_ptr<IPricer> build(const InstrumentDescription&);

template <typename BuilderClass>
class FactoryRegistrator
{
public:
    FactoryRegistrator()
    {
        Factory::instance().register_constructor(BuilderClass::getId(), &BuilderClass::build);
    }
};

#endif
