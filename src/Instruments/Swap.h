#ifndef INSTRUMENTS_SWAP_H
#define INSTRUMENTS_SWAP_H

#include <Instruments/IPricer.h>
#include <Instruments/Leg.h>

#include <memory>

class Swap : public IPricer
{
public:
    Swap(std::unique_ptr<Leg> receiver, std::unique_ptr<Leg> payer);

    double price() const override;

    const Leg& receiver_leg() const { return *receiver_; }
    const Leg& payer_leg() const { return *payer_; }

private:
    std::unique_ptr<Leg> receiver_;
    std::unique_ptr<Leg> payer_;
};

#endif
