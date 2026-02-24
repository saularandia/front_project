#ifndef INSTRUMENTS_FACTORY_H
#define INSTRUMENTS_FACTORY_H

#include <Instruments/IPricer.h>
#include <Instruments/InstrumentDescription.h>

#include <functional>
#include <map>
#include <memory>

class Factory
{
public:
    using Builder = std::function<std::unique_ptr<IPricer>(const InstrumentDescription&)>;

    std::unique_ptr<IPricer> operator()(const InstrumentDescription& description) const;

    void register_constructor(const InstrumentDescription::Type& id, const Builder& builder);

    static Factory& instance()
    {
        static Factory factory;
        return factory;
    }

private:
    Factory() = default;

    InstrumentDescription::Type getBuilderId(const InstrumentDescription& description) const;

    std::map<InstrumentDescription::Type, Builder> buildersMap_;
};

#endif
