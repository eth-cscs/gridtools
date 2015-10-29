#pragma once

namespace functors{


    typedef gridtools::interval<gridtools::level<0,-1>, gridtools::level<1,-1> > x_interval;
    typedef gridtools::interval<gridtools::level<0,-2>, gridtools::level<1,1> > axis;



    // [update_jac]
    /**
        exactly the same as update_jac, just looping over the boundary faces. will need to be merged
        with the other version.
     */
    template<typename Geometry, enumtype::Shape S=Geometry::parent_shape >
    struct update_bd_jac{
        using cub=typename Geometry::cub;
        using geo_map=typename Geometry::geo_map;

        typedef accessor<0, range<0,0,0,0> , 5> const grid_points;
        typedef accessor<1, range<0,0,0,0> , 4> const dphi;
        typedef accessor<2, range<0,0,0,0> , 7> jac;
        typedef boost::mpl::vector< grid_points, dphi, jac> arg_list;

        template <typename Evaluation>
        GT_FUNCTION
        static void Do(Evaluation const & eval, x_interval) {
            dimension<4>::Index qp;
            dimension<5>::Index dimx;
            dimension<6>::Index dimy;
            dimension<1>::Index i;
            dimension<2>::Index j;
            dimension<3>::Index k;

            uint_t const num_cub_points=eval.get().get_storage_dims(dphi())[1];
            uint_t const basis_cardinality=eval.get().get_storage_dims(dphi())[0];
            uint_t const n_faces_=eval.get().get_storage_dims(jac())[6];

#ifndef __CUDACC__
            assert(num_cub_points==cub::numCubPoints());
#endif

            for(short_t face_=0; face_< n_faces_; ++face_)
            {
                //TODO dimensions should be generic
                for(short_t icoor=0; icoor< shape_property<Geometry::parent_shape>::dimension; ++icoor)
                {
                    for(short_t jcoor=0; jcoor< shape_property<S>::dimension; ++jcoor)
                    {
                        for(short_t iter_quad=0; iter_quad< num_cub_points; ++iter_quad)
                        {
                            eval( jac(dimx+icoor, dimy+jcoor, qp+iter_quad, dimension<7>(face_) ) )=0.;
                            for (int_t iterNode=0; iterNode < basis_cardinality ; ++iterNode)
                            {//reduction/gather
                                eval( jac(dimx+icoor, dimy+jcoor, qp+iter_quad, dimension<7>(face_)) ) += eval(grid_points(dimension<4>(iterNode), dimension<5>(icoor)) * !dphi(i+iterNode, j+iter_quad, k+jcoor, dimension<4>(face_) ) );
                            }
                        }
                    }
                }
            }
        }
    };
    // [update_jac]



    // //! [det]
    // /** updates the values of the Jacobian matrix. The Jacobian matrix, component (i,j) in the quadrature point q, is computed given the geometric map discretization as \f$ J(i,j,q)=\sum_k\frac{\partial \phi_i(x_k,q)}{\partial x_j} x_k \f$
    //     where x_k are the points in the geometric element*/
    // template<typename BdGeometry>
    // struct bd_projection{
    //     using bd_cub=typename BdGeometry::cub;

    //     using jac =  accessor<0, range<0,0,0,0> , 6> const;
    //     using normals =  accessor<1, range<0,0,0,0> , 5> const;
    //     using jac_projected = accessor<2, range<0,0,0,0> , 6>;
    //     using arg_list= boost::mpl::vector< jac, normals, jac_projected > ;

    //     template <typename Evaluation>
    //     GT_FUNCTION
    //     static void Do(Evaluation const & eval, x_interval) {
    //         dimension<4>::Index qp;
    //         dimension<5>::Index dimx;
    //         dimension<6>::Index dimy;

    //         uint_t const num_cub_points=eval.get().get_storage_dims(jac())[3];

    //         //"projection" on the tangent space:
    //         //J_{ij} - n_i n_k J_kj + n_i n_j
    //         for(short_t i=0; i< 3; ++i)
    //         {
    //             for(short_t j=0; j< 3; ++j)
    //             {
    //                 for(short_t q=0; q< num_cub_points; ++q)
    //                 {
    //                     float_type inner_product=0.;
    //                     for(short_t k=0; k< num_cub_points; ++k)
    //                     {
    //                         inner_product += eval(jac(dimx+i, dimy+j, qp+q))-
    //                             eval(normals(dimx+i, qp+q))*
    //                             eval(normals(dimx+k, qp+q))*
    //                             eval(jac(dimx+k, dimy+j, qp+q))+
    //                             eval(normals(dimx+i, qp+q))*
    //                             eval(normals(dimx+j, qp+q))//so that the matrix is not singular
    //                             ;
    //                     }
    //                     eval( jac_projected(dimx+i, dimy+j, qp+q) ) = inner_product;
    //                 }
    //             }
    //         }
    //     }
    // };


    // //! [det]

    template<typename Geometry, ushort_t Codimensoin>
    struct measure;

    //! [measure]
    template<typename Geometry>
    struct measure<Geometry, 2>{
        using cub=typename Geometry::cub;

        using jac = accessor<0, range<0,0,0,0> , 7> const;
        using jac_det =  accessor<1, range<0,0,0,0> , 5>;
        using arg_list= boost::mpl::vector< jac, jac_det > ;

        template <typename Evaluation>
        GT_FUNCTION
        static void Do(Evaluation const & eval, x_interval) {
            dimension<4>::Index qp;
            dimension<5>::Index dimx;
            dimension<6>::Index dimy;

            uint_t const num_faces=eval.get().get_storage_dims(jac_det())[4];
            uint_t const num_cub_points=eval.get().get_storage_dims(jac_det())[3];

            for(short_t face_=0; face_< num_faces; ++face_)
            {
                alias<jac, dimension<7> > J(face_);
                alias<jac_det, dimension<5> > Jdet(face_);

                for(short_t q=0; q< num_cub_points; ++q)
                {
                    eval( Jdet(qp+q) )= eval(
                        (
                            J(        qp+q)*J(dimx+1, dimy+1, qp+q) +
                            J(dimx+1, qp+q)*J(dimx+2, dimy+1, qp+q) +
                            J(dimy+1, qp+q)*J(dimx+2,         qp+q) -
                            J(dimy+1, qp+q)*J(dimx+1,         qp+q) -
                            J(        qp+q)*J(dimx+2, dimy+1, qp+q) -
                            J(dimx+1, dimy+1, qp+q)*J(dimx+2,         qp+q)
                            )
                        );
                }
            }
        }
    };
        //! [measure]


    // [normals]
    template<typename BdGeometry>
    struct compute_face_normals{
        using bd_cub=typename BdGeometry::cub;
        static const auto parent_shape=BdGeometry::parent_shape;

        using jac=accessor< 0, range<>, 7 >;
        using ref_normals=accessor< 1, range<>, 2 >;
        using normals=accessor< 2, range<>, 6 >;
        using arg_list=boost::mpl::vector<jac, ref_normals, normals> ;

        /** @brief compute the normal vectors in the face quadrature points

            compute the map of the tangent vectors, and take their vector product
            (works also for non-conformal maps)
        */
        template <typename Evaluation>
        GT_FUNCTION
        static void Do(Evaluation const & eval, x_interval) {
            x::Index i;
            y::Index j;
            z::Index k;
            dimension<4>::Index quad;
            dimension<5>::Index dimI;
            dimension<6>::Index dimJ;
            dimension<7>::Index f;
            uint_t const num_cub_points=eval.get().get_storage_dims(jac())[3];
            uint_t const num_faces=eval.get().get_storage_dims(jac())[6];

            for(ushort_t face_=0; face_<num_faces; ++face_){
                for(ushort_t q_=0; q_<num_cub_points; ++q_){
                    for(ushort_t i_=0; i_<3; ++i_){
                        double product = 0.;
                        for(ushort_t j_=0; j_<3; ++j_){
                            product += eval(jac(quad+q_, dimI+i_, dimJ+j_, f+face_)) * eval(!ref_normals(j_,face_));
                        }
                        eval(normals(quad+q_, dimI+i_)) = product;
                    }
                }
            }
        }
    };
    // [normals]

    // template<typename BdGeometry, ushort_t faceID>
    // struct map_vectors{
    //     using bd_cub=typename BdGeometry::cub;
    //     static const auto parent_shape=BdGeometry::parent_shape;

    //     using jac=accessor< 0, range<>, 6 >;
    //     using normals=accessor< 1, range<>, 5 >;
    //     using arg_list=boost::mpl::vector<jac, normals> ;

    //     map_vectors()
    //         {}

    //     /** @brief compute the normal vectors in the face quadrature points

    //         compute the map of the tangent vectors, and take their vector product
    //         (works also for non-conformal maps)
    //     */
    //     template <typename Evaluation>
    //     GT_FUNCTION
    //     static void Do(Evaluation const & eval, x_interval) {
    //         x::Index i;
    //         y::Index j;
    //         z::Index k;
    //         dimension<4>::Index quad;
    //         dimension<5>::Index dimI;
    //         dimension<6>::Index dimJ;

    //         uint_t const num_cub_points=eval.get().get_storage_dims(jac())[3];

    //         array<double, 3> tg_u;
    //         array<double, 3> tg_v;

    //         for(ushort_t q_=0; q_<num_cub_points; ++q_){
    //             for(ushort_t i_=0; i_<3; ++i_){
    //                 for(ushort_t j_=0; j_<3; ++j_){
    //                     tg_u[j_]=shape_property<parent_shape>::template tangent_u<faceID>::value[i_]*eval(jac(quad+q_, dimI+i_, dimJ+j_));
    //                     tg_v[j_]=shape_property<parent_shape>::template tangent_v<faceID>::value[i_]*eval(jac(quad+q_, dimI+i_, dimJ+j_));
    //                 }
    //             }
    //             array<double, 3> normal(vec_product(tg_u, tg_v));
    //             for(ushort_t j_=0; j_<3; ++j_){
    //                 eval(normals(quad+q_, dimJ+j_))=normal[j_];
    //             }
    //         }
    //     }
    // };
    // // [normals]



}//namespace functors
