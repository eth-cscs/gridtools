#pragma once
#include <gridtools.hpp>

/**
   @file
   reference implementation of the shallow_water example.
   For an exhaustive description of the shallow water problem refer to:
   http://www.mathworks.ch/moler/exm/chapters/water.pdf

*/
using namespace gridtools;

namespace {
    template < int exponent >
    static constexpr float_type pow(float_type const &x) {
        return gridtools::gt_pow< exponent >::apply(x);
    }
}

template < typename StorageType, uint_t DimI, uint_t DimJ >
struct shallow_water_reference {

    typedef StorageType storage_type;
#ifdef __CUDACC__
    typedef hybrid_pointer< float_type > pointer_type;
#else
    typedef wrap_pointer< float_type > pointer_type;
#endif

    static constexpr uint_t strides[2] = {DimI, 1};
    static constexpr uint_t size = DimI * DimJ;
    static constexpr uint_t ip1 = strides[0];
    static constexpr uint_t jp1 = strides[1];
    static constexpr uint_t im1 = -strides[0];
    static constexpr uint_t jm1 = -strides[1];

    typename storage_type::storage_info_type solution_meta;
    storage_type solution;
    float_type u_array[size];
    float_type v_array[size];
    float_type h_array[size];
    float_type ux_array[size];
    float_type vx_array[size];
    float_type hx_array[size];
    float_type uy_array[size];
    float_type vy_array[size];
    float_type hy_array[size];

    pointer_type u;
    pointer_type v;
    pointer_type h;
    pointer_type ux;
    pointer_type vx;
    pointer_type hx;
    pointer_type uy;
    pointer_type vy;
    pointer_type hy;

    static float_type dx() { return 1.; }
    static float_type dy() { return 1.; }
    static float_type dt() { return .02; }
    static float_type g() { return 9.81; }

    static constexpr float_type height = 2.;
    GT_FUNCTION
    static float_type droplet(uint_t const &i, uint_t const &j) {
#ifndef __CUDACC__
        return 1. +
               height * std::exp(-5 * (((i - 3) * dx()) * (((i - 3) * dx())) + ((j - 7) * dy()) * ((j - 7) * dy())));
#else // if CUDA we test the serial case
        return 1. +
               height * std::exp(-5 * (((i - 3) * dx()) * (((i - 3) * dx())) + ((j - 3) * dy()) * ((j - 3) * dy())));
#endif
    }

    shallow_water_reference() : solution_meta(DimI, DimJ, static_cast< uint_t >(1)), solution(solution_meta) {}

    void setup() {
        u = pointer_type(u_array, size, true);
        v = pointer_type(v_array, size, true);
        h = pointer_type(h_array, size, true);
        ux = pointer_type(ux_array, size, true);
        vx = pointer_type(vx_array, size, true);
        hx = pointer_type(hx_array, size, true);
        uy = pointer_type(uy_array, size, true);
        vy = pointer_type(vy_array, size, true);
        hy = pointer_type(hy_array, size, true);
        for (uint_t i = 0; i < DimI; ++i)
            for (uint_t j = 0; j < DimJ; ++j) {
                uint_t id = i * strides[0] + j * strides[1];
                u[id] = 0;
                v[id] = 0;
                h[id] = droplet(i, j);
                ux[id] = 0;
                vx[id] = 0;
                hx[id] = 0;
                uy[id] = 0;
                vy[id] = 0;
                hy[id] = 0;
            }
        solution.template set< 0, 0 >(h);
        solution.template set< 0, 1 >(u);
        solution.template set< 0, 2 >(v);
    }

    void iterate() {

        for (uint_t i = 0; i < DimI - 1; ++i)
            for (uint_t j = 0; j < DimJ - 2; ++j) {
                uint_t id = i * strides[0] + j * strides[1];
                hx[id] =
                    (h[id + ip1 + jp1] + h[id + jp1]) / 2. - (u[id + ip1 + jp1] - u[id + jp1]) * (dt() / (2 * dx()));

                ux[id] = (u[id + ip1 + jp1] + u[id + jp1]) / 2. -
                         (((pow< 2 >(u[id + ip1 + jp1])) / h[id + ip1 + jp1] + pow< 2 >(h[id + ip1 + jp1]) * g() / 2.) -
                             (pow< 2 >(u[id + jp1]) / h[id + jp1] + pow< 2 >(h[id + jp1]) * (g() / 2.))) *
                             (dt() / (2. * dx()));

                vx[id] = (v[id + ip1 + jp1] + v[id + jp1]) / 2. -
                         (u[id + ip1 + jp1] * v[id + ip1 + jp1] / h[id + ip1 + jp1] -
                             u[id + jp1] * v[id + jp1] / h[id + jp1]) *
                             (dt() / (2 * dx()));
            }

        for (uint_t i = 0; i < DimI - 2; ++i)
            for (uint_t j = 0; j < DimJ - 1; ++j) {
                uint_t id = i * strides[0] + j * strides[1];
                hy[id] =
                    (h[id + ip1 + jp1] + h[id + ip1]) / 2. - (v[id + ip1 + jp1] - v[id + ip1]) * (dt() / (2 * dy()));

                uy[id] = (u[id + ip1 + jp1] + u[id + ip1]) / 2. -
                         (v[id + ip1 + jp1] * u[id + ip1 + jp1] / h[id + ip1 + jp1] -
                             v[id + ip1] * u[id + ip1] / h[id + ip1]) *
                             (dt() / (2 * dy()));

                vy[id] = (v[id + ip1 + jp1] + v[id + ip1]) / 2. -
                         ((pow< 2 >(v[id + ip1 + jp1]) / h[id + ip1 + jp1] + pow< 2 >(h[id + ip1 + jp1]) * g() / 2.) -
                             (pow< 2 >(v[id + ip1]) / h[id + ip1] + pow< 2 >(h[id + ip1]) * (g() / 2.))) *
                             (dt() / (2. * dy()));
            }

        for (uint_t i = 1; i < DimI - 2; ++i)
            for (uint_t j = 1; j < DimJ - 2; ++j) {
                uint_t id = i * strides[0] + j * strides[1];
                h[id] = h[id] - (ux[id + jm1] - ux[id + im1 + jm1]) * (dt() / dx()) -
                        (vy[id + im1] - vy[id + im1 + jm1]) * (dt() / dy());

                u[id] = u[id] -
                        (pow< 2 >(ux[id + jm1]) / hx[id + jm1] + hx[id + jm1] * hx[id + jm1] * ((g() / 2.)) -
                            (pow< 2 >(ux[id + im1 + jm1]) / hx[id + im1 + jm1] +
                                pow< 2 >(hx[id + im1 + jm1]) * ((g() / 2.)))) *
                            ((dt() / dx())) -
                        (vy[id + im1] * uy[id + im1] / hy[id + im1] -
                            vy[id + im1 + jm1] * uy[id + im1 + jm1] / hy[id + im1 + jm1]) *
                            (dt() / dy());

                v[id] = v[id] -
                        (ux[id + jm1] * vx[id + jm1] / hx[id + jm1] -
                            (ux[id + im1 + jm1] * vx[id + im1 + jm1]) / hx[id + im1 + jm1]) *
                            ((dt() / dx())) -
                        (pow< 2 >(vy[id + im1]) / hy[id + im1] + pow< 2 >(hy[id + im1]) * ((g() / 2.)) -
                            (pow< 2 >(vy[id + im1 + jm1]) / hy[id + im1 + jm1] +
                                pow< 2 >(hy[id + im1 + jm1]) * ((g() / 2.)))) *
                            ((dt() / dy()));
            }
    }
};

template < typename StorageType, uint_t DimI, uint_t DimJ >
constexpr uint_t shallow_water_reference< StorageType, DimI, DimJ >::strides[2];
template < typename StorageType, uint_t DimI, uint_t DimJ >
constexpr uint_t shallow_water_reference< StorageType, DimI, DimJ >::size;
template < typename StorageType, uint_t DimI, uint_t DimJ >
constexpr uint_t shallow_water_reference< StorageType, DimI, DimJ >::ip1;
template < typename StorageType, uint_t DimI, uint_t DimJ >
constexpr uint_t shallow_water_reference< StorageType, DimI, DimJ >::jp1;
template < typename StorageType, uint_t DimI, uint_t DimJ >
constexpr uint_t shallow_water_reference< StorageType, DimI, DimJ >::im1;
template < typename StorageType, uint_t DimI, uint_t DimJ >
constexpr uint_t shallow_water_reference< StorageType, DimI, DimJ >::jm1;
