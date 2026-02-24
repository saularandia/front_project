#include <Instruments/GoalSeeker.h>

#include <cmath>
#include <stdexcept>

double GoalSeeker::newton_raphson(const std::function<double(double)>& f,
                                  const std::function<double(double)>& df,
                                  double x0,
                                  double tol,
                                  int max_iter)
{
    double x = x0;
    for (int i = 0; i < max_iter; ++i)
    {
        const double fx = f(x);
        if (std::abs(fx) < tol)
            return x;

        const double dfx = df(x);
        if (dfx == 0.0)
            throw std::runtime_error("GoalSeeker::newton_raphson: zero derivative");

        x -= fx / dfx;
    }
    throw std::runtime_error("GoalSeeker::newton_raphson: did not converge");
}
