#include <Instruments/Factory.h>

#include <stdexcept>

std::unique_ptr<IPricer> Factory::operator()(const InstrumentDescription& description) const
{
    description.validate();

    auto builder = buildersMap_.find(getBuilderId(description));
    if (builder == buildersMap_.end())
        throw std::runtime_error("Factory: invalid payoff descriptor (no registered builder)");

    return (builder->second)(description);
}

InstrumentDescription::Type Factory::getBuilderId(const InstrumentDescription& description) const
{
    return description.type;
}

void Factory::register_constructor(const InstrumentDescription::Type& id, const Builder& builder)
{
    buildersMap_.insert(std::make_pair(id, builder));
}
