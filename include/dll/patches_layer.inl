//=======================================================================
// Copyright (c) 2014-2016 Baptiste Wicht
// Distributed under the terms of the MIT License.
// (See accompanying file LICENSE or copy at
//  http://opensource.org/licenses/MIT)
//=======================================================================

#ifndef DLL_PATCHES_LAYER_INL
#define DLL_PATCHES_LAYER_INL

namespace dll {

/*!
 * \brief Layer to cut images into patches.
 */
template <typename Desc>
struct patches_layer {
    using desc = Desc;

    static constexpr const std::size_t width    = desc::width;
    static constexpr const std::size_t height   = desc::height;
    static constexpr const std::size_t v_stride = desc::v_stride;
    static constexpr const std::size_t h_stride = desc::h_stride;

    using weight = typename desc::weight;

    using input_one_t = etl::dyn_matrix<weight, 3>;
    using input_t     = std::vector<input_one_t>;

    template <std::size_t B>
    using input_batch_t = etl::fast_dyn_matrix<weight, 1, 1, 1, 1>; //This is fake, should never be used

    using output_one_t = std::vector<etl::fast_dyn_matrix<weight, 1UL, height, width>>;
    using output_t     = std::vector<output_one_t>;

    patches_layer() = default;

    static std::string to_short_string() {
        char buffer[1024];
        snprintf(buffer, 1024, "Patches -> (%lu:%lux%lu:%lu)", height, v_stride, width, h_stride);
        return {buffer};
    }

    static void display() {
        std::cout << to_short_string() << std::endl;
    }

    static constexpr std::size_t output_size() noexcept {
        return width * height;
    }

    static void activate_hidden(output_one_t& h_a, const input_one_t& input) {
        cpp_assert(etl::dim<0>(input) == 1, "Only one channel is supported for now");

        h_a.clear();

        for (std::size_t y = 0; y + height <= etl::dim<1>(input); y += v_stride) {
            for (std::size_t x = 0; x + width <= etl::dim<2>(input); x += h_stride) {
                h_a.emplace_back();

                auto& patch = h_a.back();

                for (std::size_t yy = 0; yy < height; ++yy) {
                    for (std::size_t xx = 0; xx < width; ++xx) {
                        patch(0, yy, xx) = input(0, y + yy, x + xx);
                    }
                }
            }
        }
    }

    static void activate_many(output_t& h_a, const input_t& input) {
        for (std::size_t i = 0; i < input.size(); ++i) {
            activate_one(input[i], h_a[i]);
        }
    }

    template <typename Input>
    static output_t prepare_output(std::size_t samples) {
        return output_t(samples);
    }

    template <typename Input>
    static output_one_t prepare_one_output() {
        return output_one_t();
    }
};

//Allow odr-use of the constexpr static members

template <typename Desc>
const std::size_t patches_layer<Desc>::width;

template <typename Desc>
const std::size_t patches_layer<Desc>::height;

template <typename Desc>
const std::size_t patches_layer<Desc>::v_stride;

template <typename Desc>
const std::size_t patches_layer<Desc>::h_stride;

} //end of dll namespace

#endif
