# Burg

The Burg's algorithms is an algorithms which allows us to compute efficiently the parameters of an AR model, starting from a series of data samples.

The idea behind it is to compute the AR parameters, starting from the reflection coefficients computed through the Burg's method. 

Burg in fact demonstrated that there is a 1-to-1 relation between the reflection coefficients and the partial autocorrelation matrix, allowing us to actually compute those coefficients without needing to compute the whole matrix.



The Burg's algorithm works as follow:

- Let's define $f$ as the **forward propagation error**, and $e$ as the **backward propagation error**.
- $f$ and $e$ are **initialized starting from the samples** which we will define as $d$
- We will also defile $\sigma$ as the AR error energy (**Residual variance estimates** where the value in position $i$ is the residual variance in an AR model that includes $p$ lags) 
- We define $J$ as an anti-diagonal matrix with 1 on the anti-diagonal: $\left[ \begin{matrix}0 & 1\\ 1 & 0\end{matrix}\right]$
- $f = d$
- $e = d$
- $\sigma[0] = e \cdot b$
- $den = e \cdot e + f \cdot f =  2 \cdot \sigma[0]$
- $N = len(d)$
- $k = 0$
- $a[0] = 1$
- for $i \in N$ in $[1, .., p]$
  - $f' = f[i:N]$
  - $e' = e[0:N-i]$
  - $num = -2 \space e' \cdot f'$
  - $den = f' \cdot f' + e' \cdot e' = (1 - k^2) den - f[i-1]^2 - b[N-i+1]^2$
  - $k = \frac{num}{den}$
  - $f = f' + kb'$
  - $b = b' + kf'$
  - $\sigma[i] = (1 - k^2)\sigma[i-1]$
  - $a = a +kJa$



A C++ implementation is as follow

```c++
// Class definition
template <typename T>
class burg {
private:
    std::size_t max_size;
    std::size_t max_order;

    std::vector<T> f;
    std::vector<T> b;

public:
    burg_basic(const std::size_t max_size) : max_size{max_size},
                                             max_order{max_size - 1},
                                             f(max_size),
                                             b(max_size)
    {};
    ...
}
```

```c++
// Fit method
template <typename T>
class burg {
...
public:
    std::vector<T> fit(std::vector<T>& samples, std::size_t order)
    {
        // Let's find the actual sample size and order
        std::size_t actual_size = std::min(samples.size(), max_size);
        std::size_t samples_start = samples.size() - actual_size;
        std::size_t actual_order = std::min(order, max_order);

        // Initialize f and b
        std::copy(samples.cbegin() + samples_start, samples.cend(), f.begin());
        std::copy(samples.cbegin() + samples_start, samples.cend(), b.begin());

        // Alloc vector with AR coefficients
        std::vector<T> a(actual_order + 1);
        a[0] = 1.; // As per burg's specifications

        // Initialize burg methods variables
        T ki = 0.;  // K at i iteration
        T num = 0.; // Numerator
        T den = 0.; // Denominator
        T err = la::prod::dot_basic(&samples.data()[samples_start], &samples.data()[samples_start], actual_size); // Error

        // AR main loop
        for (std::size_t i = 1; i <= actual_order; ++i)
        {
            // Compute numerator and denominator
            num = -2 * la::prod::dot_basic(&b.data()[0], &f.data()[i], actual_size - i);
            den = la::prod::dot_basic(&f.data()[i], &f.data()[i], actual_size - i) + la::prod::dot_basic(&b.data()[0], &b.data()[0], actual_size - i);
            
            // Compute the reflection coefficient
            ki = num / den;

            // Update f and b
            for (std::size_t j = i; j < actual_size; j++)
            {
                T bj = b[j - i];
                T fj = f[j];

                b[j - i] = bj + ki * fj;
                f[j] = fj + ki * bj;
            }

            // Update a coefficients
            for (std::size_t j = 1; j <= i / 2; j++)
            {
                T aj = a[j];
                T anj = a[i - j];

                a[j] = aj + ki * anj;
                a[i - j] = anj + ki * aj;
            }
            a[i] = ki;
			
            // Update error (\sigma)
            err = err * (1 - ki * ki);
        }

        // Return coefficients
        return a;
    }
}
```

```c++
// Fit method
template <typename T>
class burg {
...
public:
    std::vector<T> predict(std::vector<T>& samples, std::vector<T>& a, std::size_t n)
    {
        std::vector<T> predictions(n); // Array where to store predictions
        std::vector<T> section(a.size() - 1); // Array where to store values in the current section
		
        // Main prediction loop
        for (ssize_t i = 0; i < static_cast<ssize_t>(n); i++)
        {
            // Fill the section with actual samples or predicted ones
            for (ssize_t j = 1; j < static_cast<ssize_t>(a.size()); j++)
            {
                section[j - 1] = -(i - j < 0 ? static_cast<T>(samples[samples.size() + i - j]) : predictions[i - j]);
            }
			
            // Predict the new sample
            predictions[i] = la::prod::dot_basic(&section.data()[0], &a.data()[1], a.size() - 1);
        }

        return predictions;
    }
}
```



## Fine-tuning

Due to the fact that translation from the math world of real numbers to the computer world of real number we are introducing some errors we need to pay attention to what we perform in order to minimize error propagations, since we are predicting samples based on the previous history, which consists also of previous predictions.

The above implementation is the most basic one, which is not based on the recursive denominator optimization, but relies on the formula $f' \cdot f' + e' \cdot e'$.  The C++ class implementing it is [burg_basic.hpp](../src/burg_basic.hpp).

While this is good from the point of view of error propagation, it is not desirable from the point of view of computation time, since we are performing two dot products instead of four multiplications and three sums, as in the optimized formula $(1 - k^2) den - f[i-1]^2 - b[N-i+1]^2$.

So, the next implementation is based solely on the optimized denominator formula. The C++ class implementing it is [burg_optimized_den.hpp](../src/burg_optimized_den.hpp).

This second implementation is way faster, but error propagation has a heavier impact. One observation about the burgs algorithm is that it is implicitly recursive, meaning that values calculated at iteration 1, will affect values calculated at iteration 2, which will affect values calculated at iteration 3 and so on. This means that the error introduced during the first iterations has a heavier impact on the results. So the next implementation was based on the idea of combining both implementation into an hybrid one, where the denominator in the first $\sqrt{p + 1}$ (to recall, $p$ is the order of the model) iterations is computed using the dot product version, as in [burg_basic.hpp](../src/burg_basic.hpp), then for the next iterations we use the optimized denominator formula, as in [burg_optimized_den.hpp](../src/burg_optimized_den.hpp). The implementation of this last version can be found in  [burg_optimized_den_sqrt.hpp](../src/burg_optimized_den_sqrt.hpp)



## Error compensation

Another strategy I decided to pursue was to try to avoid using a type with higher precision (like `long double` instead of `double`) and try to compensate the error introduced by float/double operations. To do so, I found a useful [paper by Stef Graillat and Valérie Ménissier-Morain](https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=&cad=rja&uact=8&ved=2ahUKEwiWp-6bksz8AhVpSPEDHZjEC48QFnoECA4QAQ&url=https%3A%2F%2Fwww-pequan.lip6.fr%2F~graillat%2Fpapers%2Fnolta07.pdf&usg=AOvVaw3Sybqk1uP42ZjyOsHIvRlC) which contained a recap of formulas to perform error-free transformation in floating point arithmetic.

I implemented the ones useful for my specific case (dot product and sum related) in a header file called [precise_la.hpp](../src/precise_la.hpp)

I then proceeded to re-implement all the Burg's algorithm variations using those error-free transformations. I obtained respectively [compensated_burg_basic.hpp](../src/compensated_burg_basic.hpp), [compensated_burg_optimized_den.hpp](../src/compensated_burg_optimized_den.hpp) and [compensated_burg_optimized_den_sqrt.hpp](../src/compensated_burg_optimized_den_sqrt.hpp)