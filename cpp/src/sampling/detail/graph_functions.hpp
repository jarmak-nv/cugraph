/*
 * Copyright (c) 2022, NVIDIA CORPORATION.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <thrust/optional.h>

namespace cugraph {
namespace detail {

// FIXME: Several of the functions in this file assume that store_transposed=false,
//    in implementation, naming and documentation.  We should review these and
//    consider updating things to support an arbitrary value for store_transposed

/**
 * @brief Gather active majors across gpus in a column communicator
 *
 * Collect all the vertex ids and client gpu ids to be processed by every gpu in
 * the column communicator and sort the list.
 *
 * @tparam vertex_t Type of vertex indices.
 * @param handle RAFT handle object to encapsulate resources (e.g. CUDA stream, communicator, and
 * handles to various CUDA libraries) to run graph algorithms.
 * @param d_in Device vector containing vertices local to this GPU
 * @return Device vector containing all the vertices that are to be processed by every gpu
 * in the column communicator
 */
template <typename vertex_t>
rmm::device_uvector<vertex_t> allgather_active_majors(raft::handle_t const& handle,
                                                      rmm::device_uvector<vertex_t>&& d_in);

// FIXME: Need docs if this function survives
template <typename vertex_t, typename edge_t, typename weight_t>
std::tuple<rmm::device_uvector<vertex_t>,
           rmm::device_uvector<vertex_t>,
           rmm::device_uvector<weight_t>,
           rmm::device_uvector<edge_t>>
count_and_remove_duplicates(raft::handle_t const& handle,
                            rmm::device_uvector<vertex_t>&& src,
                            rmm::device_uvector<vertex_t>&& dst,
                            rmm::device_uvector<weight_t>&& wgt);

/**
 * @brief Gather edge list for specified vertices
 *
 * Collect all the edges that are present in the adjacency lists on the current gpu
 *
 * @tparam GraphViewType Type of the passed non-owning graph object.
 * @param handle RAFT handle object to encapsulate resources (e.g. CUDA stream, communicator, and
 * handles to various CUDA libraries) to run graph algorithms.
 * @param graph_view Non-owning graph object.
 * @param active_majors Device vector containing all the vertex id that are processed by
 * gpus in the column communicator
 * @return A tuple of device vector containing the majors, minors and weights gathered locally
 */
template <typename GraphViewType>
std::tuple<rmm::device_uvector<typename GraphViewType::vertex_type>,
           rmm::device_uvector<typename GraphViewType::vertex_type>,
           std::optional<rmm::device_uvector<typename GraphViewType::weight_type>>>
gather_one_hop_edgelist(
  raft::handle_t const& handle,
  GraphViewType const& graph_view,
  const rmm::device_uvector<typename GraphViewType::vertex_type>& active_majors,
  bool do_expensive_check = false);

/**
 * @brief Randomly sample edges from the adjacency list of specified vertices
 *
 * @tparam GraphViewType Type of the passed non-owning graph object.
 * @param handle RAFT handle object to encapsulate resources (e.g. CUDA stream, communicator, and
 * handles to various CUDA libraries) to run graph algorithms.
 * @param rng_state Random number generator state
 * @param graph_view Non-owning graph object.
 * @param active_majors Device vector containing all the vertex id that are processed by
 * gpus in the column communicator
 * @param fanout How many edges to sample for each vertex
 * @param with_replacement If true sample with replacement, otherwise sample without replacement
 * @param invalid_vertex_id Value to use for an invalid vertex
 * @return A tuple of device vector containing the majors, minors and weights gathered locally
 */
template <typename GraphViewType>
std::tuple<rmm::device_uvector<typename GraphViewType::vertex_type>,
           rmm::device_uvector<typename GraphViewType::vertex_type>,
           std::optional<rmm::device_uvector<typename GraphViewType::weight_type>>>
sample_edges(raft::handle_t const& handle,
             GraphViewType const& graph_view,
             raft::random::RngState& rng_state,
             rmm::device_uvector<typename GraphViewType::vertex_type> const& active_majors,
             size_t fanout,
             bool with_replacement);

}  // namespace detail
}  // namespace cugraph
