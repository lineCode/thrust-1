/*
 *  Copyright 2008-2011 NVIDIA Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */


#pragma once

#include <thrust/detail/backend/generic/reduce.h>
#include <thrust/detail/backend/reduce.h>
#include <thrust/iterator/iterator_traits.h>
#include <thrust/iterator/detail/backend_iterator_spaces.h>
#include <thrust/detail/uninitialized_array.h>

namespace thrust
{

namespace detail
{

namespace backend
{

namespace generic
{

namespace detail
{

// this metafunction passes through a type unless it's any_space_tag,
// in which case it returns default_device_space_tag
template<typename Space>
  struct any_space_to_default_device_space_tag
{
  typedef Space type;
}; // end any_space_to_default_device_space_tag

template<>
  struct any_space_to_default_device_space_tag<thrust::any_space_tag>
{
  typedef thrust::detail::default_device_space_tag type;
}; // end any_space_to_default_device_space_tag


} // end detail

template<typename RandomAccessIterator,
         typename SizeType,
         typename OutputType,
         typename BinaryFunction>
  OutputType reduce_n(RandomAccessIterator first,
                      SizeType n,
                      OutputType init,
                      BinaryFunction binary_op)
{
  // compute schedule for first stage
  const SizeType num_blocks = thrust::detail::backend::get_unordered_blocked_reduce_n_schedule(first, n, init, binary_op);
  
  // allocate storage for the initializer and partial sums
  typedef typename thrust::iterator_space<RandomAccessIterator>::type PossiblyAnySpace;
  typedef typename detail::any_space_to_default_device_space_tag<PossiblyAnySpace>::type Space;
  thrust::detail::uninitialized_array<OutputType,Space> partial_sums(1 + num_blocks);
  
  // set first element of temp array to init
  partial_sums[0] = init;
  
  // accumulate partial sums
  thrust::detail::backend::unordered_blocked_reduce_n(first, n, num_blocks, binary_op, partial_sums.begin() + 1);

  // reduce partial sums
  thrust::detail::backend::unordered_blocked_reduce_n(partial_sums.begin(), num_blocks + 1, 1, binary_op, partial_sums.begin());

  return partial_sums[0];
} // end reduce_n()

} // end generic

} // end backend

} // end detail

} // end thrust

