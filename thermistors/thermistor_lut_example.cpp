#include <cstdint>
#include <cmath>
#include <cassert>
#include <array>
#include <algorithm>
#include <numeric>
#include <span>
#include <numbers>

struct AdcSpacing {
    uint32_t start;
    uint32_t step;
    uint32_t size;

    constexpr uint32_t max() const {
        return at(size-1);
    }
    constexpr uint32_t at(const std::size_t index) const {
        return step*index + start;
    }
};

struct Thermistor {
    double A;
    double B;
    double C;
    double R = 10000;
};


template <typename T, std::size_t N>
constexpr T interpolate(const AdcSpacing& spacing, const std::array<T, N>& y_vals, const T& x) {
    static_assert(N >= 2, "x_vals and y_vals must have at least two elements");

    const std::size_t i = std::clamp<std::size_t>(
        (x - spacing.start + spacing.step / 2) / spacing.step,
        1, N - 1);

    const T x1 = spacing.step*i + spacing.start;
    const T x0 = x1 + spacing.step;
    const T y0 = y_vals.at(i - 1);
    const T y1 = y_vals.at(i);

    const T slope = (y1 - y0) / (x1 - x0);
    return y0 + slope * (x - x0);
}


template<typename type_t=double>
constexpr type_t steinhart_hart_equations(const type_t R, const type_t A, const type_t B, const type_t C) {
    const type_t logr = std::log(R);
    const auto inv_t = A + B*logr + C*logr*logr*logr;
    return 1/inv_t;
}

constexpr double steinhart_hart_equations(const double r, const Thermistor& thermistor) {
    return steinhart_hart_equations<double>(r, thermistor.A, thermistor.B, thermistor.C);
}

/*
 * Generate a list of temperature readings
 */
template<unsigned int kLutSize, unsigned int kShift=16, typename T=uint32_t>
constexpr std::array<T, kLutSize> calc_lut(const AdcSpacing& spacing, const std::size_t bits, const Thermistor& thermistor) {
    std::array<T, kLutSize> arr{};
    const double adc_max = (1<<bits) - 1;
    const double r_div = 10e3;
    const double mult = 1<<kShift;
    const double tmax = 1e3;
    const double tmin = 100;
    
    for (std::size_t i = 0; i < arr.size(); ++i) {
        const double voltage_ratio = static_cast<double>(spacing.at(i))/adc_max;

        const double resistance = voltage_ratio*r_div/(1-voltage_ratio); 
        const auto temp_f = std::clamp(steinhart_hart_equations(resistance, thermistor), tmin, tmax);
        arr[i] = static_cast<T>(temp_f * mult);
    }

    return arr;
}

template<unsigned int kLutSize>
constexpr std::array<uint32_t, kLutSize> linespace(const unsigned start, const unsigned stop) {
    static_assert(kLutSize > 0, "kLutSize must be greater than 0");
    std::array<uint32_t, kLutSize> result = {};

    if constexpr (kLutSize == 1) {
        result[0] = start;
        return result;
    }

    const double step = static_cast<double>(stop - start) / (kLutSize - 1);

    for (unsigned int i = 0; i < kLutSize; ++i) {
        result[i] = static_cast<uint32_t>(start + step * i + 0.5);  // round to nearest
    }

    return result;
}


const unsigned adc_bits = 12;
const unsigned adc_max = (1<<adc_bits)-1;
const std::size_t kLutSize = 128;
const constexpr AdcSpacing spacing {
    start: 8,
    step: 32,
    size: kLutSize
};

const constexpr Thermistor thermistor{
    A: 8.794452e-04, 
    B:2.525972e-04, 
    C:1.897193e-07,
    R: 10e3};


inline const constexpr auto lut = calc_lut<kLutSize,16>(spacing, adc_bits, thermistor);

std::array<uint32_t, kLutSize> get_lut() {
    return lut;
}

// Q16.16
uint32_t calculate_temperature_q16(const uint32_t adc) {
    return interpolate<uint32_t>(spacing, lut, adc);
}