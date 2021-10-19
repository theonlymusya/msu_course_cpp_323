#include <cassert>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <vector>

using std::string;
using std::vector;

using VertexId = int;
using EdgeId = int;
using VertexDepth = int;

struct Edge;
struct Vertex;
bool vertex_exists_in_graph(const VertexId& id, const vector<Vertex>& vertices);
bool edge_exists_in_graph(const EdgeId& id, const vector<Edge>& edges);
bool edge_id_exists_in_vertex(const EdgeId& id, const vector<EdgeId>& edge_ids);
bool is_vertex_id_valid(const VertexId& id);
bool is_edge_id_valid(const EdgeId& id);

float probability() {
  std::random_device rd;
  std::mt19937 mt(rd());
  std::uniform_real_distribution<float> probability(0.0, 100.0);
  return probability(mt);
}

struct Vertex {
 public:
  Vertex(const VertexId& source_id,
         const VertexId& id,
         const VertexDepth& depth)
      : source_id_(source_id), id_(id), depth_(depth) {}

  void add_edge(const EdgeId& id) {
    assert(!edge_id_exists_in_vertex(id, edge_ids_) &&
           "Edge already exists in vertex!");
    edge_ids_.push_back(id);
  }
  const VertexId get_source_id() const { return source_id_; }
  const VertexId get_id() const { return id_; }
  const VertexDepth get_depth() const { return depth_; }
  const vector<EdgeId>& get_edge_ids() const { return edge_ids_; }
  std::string to_JSON() const {
    std::string json_string;

    json_string += "\t{ \"id\": " + std::to_string(id_) + ", \"edge_ids\": [";
    for (int i = 0; i < edge_ids_.size(); i++) {
      json_string += std::to_string(edge_ids_[i]);
      if (i + 1 != edge_ids_.size())
        json_string += ", ";
    }
    json_string += "], \"depth\": " + std::to_string(depth_) + "}";
    return json_string;
  }

 private:
  vector<EdgeId> edge_ids_;
  const VertexId source_id_;
  const VertexId id_;
  const VertexDepth depth_;
};

struct Edge {
 public:
  enum class Color { GRAY, GREEN, BLUE, YELLOW, RED };

  Edge(VertexId source, VertexId destination, VertexId id, Color color)
      : id_(id), color_(color), source_(source), destination_(destination) {}

  std::string color_to_string(const Edge::Color& color) const;

  const EdgeId get_id() const { return id_; }

  std::string to_JSON() const {
    std::string json_string;
    json_string += "\t{ \"id\": " + std::to_string(id_) +
                   ", \"vertex_ids\": [" + std::to_string(source_) + ", " +
                   std::to_string(destination_) + "], \"color\": \"" +
                   color_to_string(color_) + "\" }";
    return json_string;
  }

 private:
  const EdgeId id_;
  const Color color_;
  const VertexId source_;
  const VertexId destination_;
};

std::string Edge::color_to_string(const Edge::Color& _color) const {
  switch (_color) {
    case Edge::Color::GRAY:
      return "gray";
    case Edge::Color::GREEN:
      return "green";
    case Edge::Color::BLUE:
      return "blue";
    case Edge::Color::YELLOW:
      return "yellow";
    case Edge::Color::RED:
      return "red";
  }
}

class Graph {
 public:
  Graph(VertexDepth max_depth, int new_vertices_num)
      : max_depth_(max_depth), new_vertices_num_(new_vertices_num) {}

  VertexId source_of_vertex(const VertexId& vertex) {
    return vertices_[vertex].get_source_id();
  }

  void insert_vertex(const VertexId& source_vertex,
                     const VertexId& vertex,
                     const VertexDepth& depth) {
    assert(is_vertex_id_valid(vertex) && "Vertex id is not valid!");  // OK
    assert(!vertex_exists_in_graph(vertex, vertices_) &&              // OK
           "Vertex already exists!");
    vertices_.emplace_back(source_vertex, vertex, depth);
  }
  void insert_edge(const VertexId& source,
                   const VertexId& destination,
                   const EdgeId& id,
                   const Edge::Color& color) {
    assert(is_edge_id_valid(id) && "Edge id is not valid!");              // OK
    assert(!edge_exists_in_graph(id, edges_) && "Edge already exists!");  // OK
    // assert(vertex_exists_in_graph(source, vertices_) &&
    //        "Source vertex id doesn't exist!");
    // assert(vertex_exists_in_graph(destination, vertices_) &&
    //       "Destination vertex id doesn't exist!");

    edges_.emplace_back(source, destination, id, color);
    if (color != Edge::Color::GREEN) {
      assert(!are_vertices_connected(source, destination) &&  // OK
             "Vertices are already connected!");
      vertices_[source].add_edge(id);
      vertices_[destination].add_edge(id);
    } else {
      vertices_[source].add_edge(id);
    }
  }

  bool are_vertices_connected(const VertexId& source_vertex,
                              const VertexId& destination_vertex) {
    for (const auto& source : vertices_[source_vertex].get_edge_ids()) {
      for (const auto& destination :
           vertices_[destination_vertex].get_edge_ids()) {
        if (source == destination) {
          std::cerr << "Vertices: " << source_vertex << " and "
                    << destination_vertex
                    << " are already connected with edge: " << source << "\n";
          return true;
        }
      }
    }
    return false;
  }

  VertexDepth get_max_depth() { return max_depth_; }
  void set_max_depth(VertexDepth max_depth) { max_depth_ = max_depth; }
  int get_new_vertices_num() { return new_vertices_num_; }

  std::string to_JSON() const {
    std::string json_string;
    json_string += "{\n\"vertices\": [\n";
    for (int i = 0; i < vertices_.size(); i++) {
      json_string += vertices_[i].to_JSON();
      if (i + 1 == vertices_.size()) {
        json_string += "\n  ],\n";
      } else {
        json_string += ",\n";
      }
    }

    json_string += "\"edges\": [\n";
    for (int i = 0; i < edges_.size(); i++) {
      json_string += edges_[i].to_JSON();
      if (i + 1 == edges_.size()) {
        json_string += "\n";
      } else {
        json_string += ",\n";
      }
    }
    json_string += "  ]\n}\n";
    return json_string;
  }

  // Returns depth of vertex in the graph
  const VertexDepth depth_of(const VertexId& id) const {
    return vertices_[id].get_depth();
  }

  // Returns the number of vertices created by vertex in the graph
  const int created_by_vertex(const VertexId& id) const {
    if (id == 0)
      return new_vertices_num_;
    int count = 0;
    for (const auto& vertex : vertices_) {
      if (vertex.get_source_id() == id)
        count++;
    }
    return count;
  }

  vector<Vertex> get_vertices() const { return vertices_; }
  vector<Edge> get_edges() const { return edges_; }

  // Returns total number of vertices in the depth
  int vertices_count_in_depth(const VertexDepth& depth) const {
    if (depth == 0)
      return 1;

    int count = 0;
    for (const auto& vertex : vertices_) {
      if (depth == vertex.get_depth())
        count++;
    }
    return count;
  }

  // Returns all the vertices in depth(x)
  vector<VertexId> vertices_in_depth(const VertexDepth& depth) {
    vector<VertexId> vertices;
    for (const auto& vertex : vertices_) {
      if (vertex.get_depth() == depth) {
        vertices.push_back(vertex.get_id());
      }
    }
    return vertices;
  }

 private:
  vector<Edge> edges_;
  vector<Vertex> vertices_;
  int max_depth_;
  int new_vertices_num_;
};

void generate_vertices_and_gray_edges(Graph& graph);
void generate_green_edges(Graph& graph);
void generate_blue_edges(Graph& graph);
void generate_yellow_edges(Graph& graph);
void generate_red_edges(Graph& graph);

const Graph generateGraph(VertexDepth max_depth, int new_vertices_num) {
  Graph graph(max_depth, new_vertices_num);
  clock_t begin, end;
  double elapsed_time;
  begin = clock();
  std::cout << "Generating vertices and gray edges...   ";
  generate_vertices_and_gray_edges(graph);
  end = clock();
  elapsed_time = (double)(end - begin) / CLOCKS_PER_SEC;
  std::cout << "Generated vertices and gray edges. Elapsed Time: "
            << elapsed_time << " s.\n";
  std::cout << "Generating green edges...   ";
  begin = clock();
  generate_green_edges(graph);
  end = clock();
  elapsed_time = (double)(end - begin) / CLOCKS_PER_SEC;
  std::cout << "Generated green edges. Elapsed Time: " << elapsed_time
            << " s.\n";
  std::cout << "Generating blue edges...   ";
  begin = clock();
  generate_blue_edges(graph);
  end = clock();
  elapsed_time = (double)(end - begin) / CLOCKS_PER_SEC;
  std::cout << "Generated blue edges. Elapsed Time: " << elapsed_time
            << " s.\n";
  std::cout << "Generating yellow edges...   ";
  begin = clock();
  generate_yellow_edges(graph);
  end = clock();
  elapsed_time = (double)(end - begin) / CLOCKS_PER_SEC;
  std::cout << "Generated yellow edges. Elapsed Time: " << elapsed_time
            << " s.\n";

  std::cout << "Generating red edges...   ";
  begin = clock();
  generate_red_edges(graph);
  end = clock();
  elapsed_time = (double)(end - begin) / CLOCKS_PER_SEC;
  std::cout << "Generated red edges. Elapsed Time: " << elapsed_time
            << " s.\n\n";

  return graph;
}

void generate_vertices_and_gray_edges(Graph& graph) {
  graph.insert_vertex(0, 0, 0);
  float condition = 0;
  VertexId source_vertex = 0;
  VertexId vertex = 1;

  for (VertexDepth depth = 0; depth < graph.get_max_depth(); depth++) {
    for (int i = 0; i < graph.vertices_count_in_depth(depth); i++) {
      for (int j = 0; j < graph.get_new_vertices_num(); j++) {
        if (probability() >= condition) {
          graph.insert_vertex(source_vertex, vertex, depth + 1);
          EdgeId edge_id = graph.get_edges().size();
          graph.insert_edge(source_vertex, vertex, edge_id, Edge::Color::GRAY);
          vertex++;
        }
      }
      source_vertex++;
    }
    condition += 100 / (float)(graph.get_max_depth());
  }
  if (graph.get_max_depth() != graph.depth_of(vertex - 1)) {
    std::cout << "\nMax depth couldn't be reached. Depth of final vertex: "
              << graph.depth_of(vertex - 1) << "\n";
    graph.set_max_depth(graph.depth_of(vertex - 1));
  }
}

void generate_green_edges(Graph& graph) {
  int condition = 90;
  for (const auto& vertex : graph.get_vertices()) {
    if (probability() >= condition) {
      EdgeId edge_id = graph.get_edges().size();
      graph.insert_edge(vertex.get_source_id(), vertex.get_source_id(), edge_id,
                        Edge::Color::GREEN);
    }
  }
}

void generate_blue_edges(Graph& graph) {
  float condition = 75;
  for (int i = 0; i < graph.get_vertices().size() - 1; i++) {
    if (graph.depth_of(i) == graph.depth_of(i + 1))
      if (probability() >= condition) {
        EdgeId edge_id = graph.get_edges().size();
        graph.insert_edge(i, i + 1, edge_id, Edge::Color::BLUE);
      }
  }
}

void generate_yellow_edges(Graph& graph) {
  int m = 0, n = 0;
  for (VertexDepth i = 1; i < graph.get_max_depth(); i++) {
    float condition = 100 - i * (100 / (float)(graph.get_max_depth() - 1));
    while (m < graph.vertices_in_depth(i).size()) {
      while (n < graph.vertices_in_depth(i + 1).size()) {
        if (probability() >= condition) {
          std::uniform_int_distribution<int> random_vertex_distribution(
              0, graph.vertices_count_in_depth(i + 1) - 1);

          VertexId source = graph.vertices_in_depth(i).at(m);
          VertexId destination;
          VertexId random_vertex;

          bool break_flag = false;
          do {
            if (graph.created_by_vertex(source) ==
                graph.vertices_count_in_depth(i + 1)) {
              break_flag = true;
              break;
            }
            std::random_device rd;
            std::mt19937 mt(rd());
            random_vertex = random_vertex_distribution(mt);
            destination = graph.vertices_in_depth(i + 1).at(random_vertex);
          } while (source == graph.source_of_vertex(destination));

          if (break_flag)
            break;
          EdgeId edge_id = graph.get_edges().size();
          graph.insert_edge(source, destination, edge_id, Edge::Color::YELLOW);
          break;
        }
        n++;
      }
      n = 0;
      m++;
    }
    m = 0;
  }
}

void generate_red_edges(Graph& graph) {
  float condition = 67;
  int m = 0, n = 0;
  for (VertexDepth i = 0; i < graph.get_max_depth(); i++) {
    while (m != graph.vertices_in_depth(i).size()) {
      while (n != graph.vertices_in_depth(i + 2).size()) {
        if (probability() >= condition) {
          std::uniform_int_distribution<int> random_vertex_distribution(
              0, graph.vertices_count_in_depth(i + 2) - 1);
          std::random_device rd;
          std::mt19937 mt(rd());
          VertexId random_vertex = random_vertex_distribution(mt);
          VertexId source = graph.vertices_in_depth(i).at(m);
          VertexId destination =
              graph.vertices_in_depth(i + 2).at(random_vertex);
          EdgeId edge_id = graph.get_edges().size();
          graph.insert_edge(source, destination, edge_id, Edge::Color::RED);
          break;
        }
        n++;
      }
      n = 0;
      m++;
    }
    m = 0;
  }
}

bool edge_exists_in_graph(const EdgeId& id, const vector<Edge>& edges) {
  for (const auto& edge : edges)
    if (edge.get_id() == id) {
      std::cerr << "[ERROR] Edge id: " << id << " already exists in graph...\n";
      return true;
    }
  return false;
}
bool vertex_exists_in_graph(const VertexId& id,
                            const vector<Vertex>& vertices) {
  for (const auto& vertex : vertices)
    if (vertex.get_id() == id) {
      std::cerr << "[ERROR] Vertex id: " << id
                << " already exists in the graph...\n";
      return true;
    }
  return false;
}

bool edge_id_exists_in_vertex(const EdgeId& edge_id,
                              const vector<EdgeId>& edge_ids) {
  for (const auto& edge : edge_ids) {
    if (edge_id == edge) {
      std::cerr << "[ERROR] Edge id: " << edge_id
                << " already exists in this Vertex...\n";
      return true;
    }
  }
  return false;
}

bool is_vertex_id_valid(const VertexId& id) {
  if (id < 0) {
    std::cerr << "[ERROR] Vertex id: " << id << " is not valid...\n";
    return false;
  }
  return true;
}

bool is_edge_id_valid(const EdgeId& id) {
  if (id < 0) {
    std::cerr << "[ERROR] Edge id: " << id << " is not valid...\n";
    return false;
  }
  return true;
}

int main() {
  string directory = "./temp";
  try {
    if (!std::filesystem::create_directory(directory)) {
    }
  } catch (const std::exception& ex) {
    std::cerr << ex.what() << "\n";
  }

  int graph_count;
  int max_depth;
  int new_vertices_num;

  std::cout << "Enter max_depth: ";
  std::cin >> max_depth;
  std::cout << "Enter new_vertices_num: ";
  std::cin >> new_vertices_num;
  std::cout << "Enter the number of graphs to be created: ";
  std::cin >> graph_count;
  std::cout << "\n";

  int k = 1;
  for (int i = 0; i < graph_count; i++) {
    string filename = "./temp/graph";
    filename = filename + std::to_string(k++) + ".json";
    std::ofstream file(filename, std::ios::out);
    if (!file.is_open())
      std::cerr << "Error opening the file graph.json!\n";
    else {
      const auto graph = generateGraph(max_depth, new_vertices_num);
      file << graph.to_JSON();
      file.close();
    }
  }
  return 0;
}
