#ifndef RNG_H
#define RNG_H

#include <chrono>
#include <cstdint>
#include <random>
#include <vector>

// Small seedable random source. std::mt19937 is specified exactly by the
// standard, but std::uniform_*_distribution is not, so the helpers below are
// implemented by hand to make a given seed reproduce the same game on every
// compiler and platform.
class Rng {
public:
    explicit Rng(std::uint32_t seed = randomSeed()) : gen_(seed) {}

    static std::uint32_t randomSeed() {
        std::random_device rd;
        // Older MinGW runtimes ship a deterministic random_device; mix in the clock.
        auto now = static_cast<std::uint32_t>(std::chrono::steady_clock::now().time_since_epoch().count());
        return rd() ^ now;
    }

    std::uint32_t next() { return gen_(); }

    // Uniform double in the open interval (0, 1).
    double uniform() { return (static_cast<double>(gen_()) + 0.5) / 4294967296.0; }

    // Uniform integer in [lo, hi].
    int range(int lo, int hi) {
        if (hi <= lo)
            return lo;
        const double span = static_cast<double>(static_cast<long long>(hi) - lo + 1);
        return lo + static_cast<int>(uniform() * span);
    }

    // True with probability p.
    bool chance(double p) { return uniform() < p; }

    template <class T>
    const T &pick(const std::vector<T> &v) {
        return v[static_cast<std::size_t>(range(0, static_cast<int>(v.size()) - 1))];
    }

private:
    std::mt19937 gen_;
};

#endif
