#include <Instruments/Swap.h>

#include <stdexcept>

Swap::Swap(std::unique_ptr<Leg> receiver, std::unique_ptr<Leg> payer)
    : receiver_(std::move(receiver)), payer_(std::move(payer))
{
    if (!receiver_ || !payer_)
        throw std::invalid_argument("Swap: null leg");
}

double Swap::price() const
{
    return receiver_->price() - payer_->price();
}
