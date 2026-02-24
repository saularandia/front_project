#ifndef INSTRUMENTS_GOALSEEKER_H
#define INSTRUMENTS_GOALSEEKER_H

#include <functional>

class GoalSeeker
{
public:
    // Newton-Raphson root finder.
    // Throws if it doesn't converge.
    static double newton_raphson(const std::function<double(double)>& f,
                                 const std::function<double(double)>& df,
                                 double x0,
                                 double tol = 1e-12,
                                 int max_iter = 50);
};

#endif
