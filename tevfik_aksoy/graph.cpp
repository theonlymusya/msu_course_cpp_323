#include "graph.hpp"
#include <cassert>

namespace uni_cpp_practice {
namespace {
bool is_depth_valid(int depth,
                    const std::vector<std::vector<VertexId>>& depth_map) {
  if (depth > depth_map.size())
    return false;
  if (depth_map[depth].empty())
    return false;
  return true;
}
}  // namespace

void Vertex::add_edge_id(const EdgeId& id) {
  assert(!has_edge_id(id, edge_ids_) && "Edge already exists in vertex!");
  edge_ids_.push_back(id);
}

const std::vector<EdgeId>& Vertex::get_edge_ids() const {
  return edge_ids_;
}

std::string color_to_string(const Edge::Color& color) {
  switch (color) {
    case Edge::Color::Gray:
      return "gray";
    case Edge::Color::Green:
      return "green";
    case Edge::Color::Blue:
      return "blue";
    case Edge::Color::Yellow:
      return "yellow";
    case Edge::Color::Red:
      return "red";
    case Edge::Color::Unknown:
      return "unknown";
  }
}

std::string Edge::to_JSON() const {
  std::string json_string;
  json_string += "\t{ \"id\": " + std::to_string(id) + ", \"vertex_ids\": [" +
                 std::to_string(source) + ", " + std::to_string(destination) +
                 R"(], "color": ")" + color_to_string(color) + "\" }";
  return json_string;
}

std::string Vertex::to_JSON() const {
  std::string json_string;
  json_string += "\t{ \"id\": " + std::to_string(id) + ", \"edge_ids\": [";
  for (int i = 0; i < edge_ids_.size(); i++) {
    json_string += std::to_string(edge_ids_[i]);
    if (i + 1 != edge_ids_.size())
      json_string += ", ";
  }
  json_string += "], \"depth\": " + std::to_string(depth) + "}";
  return json_string;
}

bool Graph::does_vertex_exist(const VertexId& id,
                              const std::vector<Vertex>& vertices) const {
  for (const auto& vertex : vertices)
    if (vertex.id == id) {
      return true;
    }
  return false;
}

VertexId Graph::insert_vertex() {
  const auto id = get_new_vertex_id_();
  vertices_.emplace_back(id);
  if (id == 0) {
    VertexDepth depth = 0;
    depth_map_.emplace_back();
    depth_map_[depth].push_back(id);
  }
  return id;
}

Edge::Color Graph::calculate_color_for_edge(const Vertex& source,
                                            const Vertex& destination) const {
  if (source.get_edge_ids().empty() || destination.get_edge_ids().empty())
    return Edge::Color::Gray;
  if (source.id == destination.id)
    return Edge::Color::Green;
  if (source.depth == destination.depth) {
    for (int i = 0; i < depth_map_[source.depth].size() - 1; i++) {
      auto first = depth_map_[source.depth][i];
      auto second = depth_map_[source.depth][i + 1];
      if ((source.id == first && destination.id == second) ||
          (destination.id == first && source.id == second))
        return Edge::Color::Blue;
    }
  }
  if (source.depth == destination.depth - 1)
    return Edge::Color::Yellow;
  if (source.depth == destination.depth - 2)
    return Edge::Color::Red;

  // unknown нужно для return val of calculate_edge_color
  return Edge::Color::Unknown;
}

void Graph::insert_edge(const VertexId& source_id,
                        const VertexId& destination_id) {
  assert(!are_vertices_connected(source_id, destination_id) &&
         "Vertices are already connected!");
  const auto color =
      calculate_color_for_edge(vertices_[source_id], vertices_[destination_id]);
  const int edge_id = get_new_edge_id_();
  edges_.emplace_back(source_id, destination_id, edge_id, color);

  vertices_[source_id].add_edge_id(edge_id);
  if (color != Edge::Color::Green) {
    vertices_[destination_id].add_edge_id(edge_id);
    if (color == Edge::Color::Gray) {
      const auto depth = vertices_[source_id].depth + 1;
      vertices_[destination_id].depth = depth;
      if (depth_map_.size() == depth) {
        depth_map_.emplace_back();
      }
      depth_map_[depth].emplace_back(destination_id);
    }
  }
}

bool Graph::are_vertices_connected(const VertexId& source,
                                   const VertexId& destination) const {
  assert(does_vertex_exist(source, vertices_) &&
         "Source vertex doesn't exist!");
  assert(does_vertex_exist(destination, vertices_) &&
         "Destination vertex doesn't exist!");

  const auto& source_vertex_edges = vertices_[source].get_edge_ids();
  const auto& destination_vertex_edges = vertices_[destination].get_edge_ids();
  for (const auto& edge_of_source_vertex : source_vertex_edges)
    if (source == destination) {
      if (edges_[edge_of_source_vertex].source ==
          edges_[edge_of_source_vertex].destination)
        return true;
    } else {
      for (const auto& edge_of_destination_vertex : destination_vertex_edges)
        if (edge_of_source_vertex == edge_of_destination_vertex)
          return true;
    }
  return false;
}

int Graph::total_edges_of_color(const Edge::Color& color) const {
  int total = 0;
  for (const auto& edge : edges_) {
    if (edge.color == color)
      total++;
  }
  return total;
}

int Graph::depth() const {
  return depth_map_.size() - 1;
}

const std::vector<Vertex>& Graph::get_vertices() const {
  return vertices_;
}

const std::vector<Edge>& Graph::get_edges() const {
  return edges_;
}

const std::vector<VertexId>& Graph::get_vertices_in_depth(
    const VertexDepth& depth) const {
  assert(is_depth_valid(depth, depth_map_) && "Depth is not valid!");
  return depth_map_.at(depth);
}
}  // namespace uni_cpp_practice