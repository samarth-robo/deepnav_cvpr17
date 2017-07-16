#include <search.h>

#include <climits>
#include <iostream>
#include <utility>

#include <boost/functional/hash.hpp>

using namespace std;

size_t NodeHash::operator()(Node::Ptr const &n) const {
  size_t seed = 7;
  boost::hash_combine(seed, n->id);
  boost::hash_combine(seed, static_cast<size_t>(n->d));
  return seed;
}

bool NodeEqual::operator()(Node::Ptr const &a, Node::Ptr const &b) const {
  return (a->id == b->id) && (a->d == b->d);
}

bool PairCompare::operator()(pair<Node::Ptr, double> const &p1,
    pair<Node::Ptr, double> const &p2) const {
  return p1.second > p2.second;  // > for reversed priority q (pop() gives element with lowest priority)
}

size_t Searcher::get_path_length() {
  size_t len = 0;
  for(int i = 0; i < path.size()-1; i++) {
    if(!path[i]->equal(path[i+1])) len++;
  }
  return len;
}

// Exhaustive enumeration in the graph
// ignores start and end
bool ExhaustiveSearcher::search(Node::Ptr const &start,
    Node::Ptr const &end) {
  // check that graph has been set
  if(!graph) {
    cout << "set_graph() must be called before a_star_search()" << endl;
    return false;
  }

  path.clear();
  for(auto it = graph->cbegin(); it != graph->cend(); it++) {
    Node::Ptr n = it->second;
    path.push_back(n);
  }
  return true;
}

float AStarSearcher::a_star_heuristic(Node::Ptr const &s,
    Node::Ptr const &t) const {
  double dist;
  geod.Inverse(float(s->lat), float(s->lng), float(t->lat), float(t->lng),
      dist);
  return static_cast<float>(dist);
}

// returns length of shortest path from start_node to destination
// assumes search() has been run before
float AStarSearcher::get_cost(Node::Ptr const &start,
    Node::Ptr const &end) const {
  auto got_start = g_score.find(start);
  auto got_end   = g_score.find(end);
  if(got_start == g_score.end()) {
    cout << "Could not find in g_score " << start << endl;
    return -1.f;
  } else if(got_end == g_score.end()) {
    cout << "Could not find in g_score " << end << endl;
    return -1.f;
  } else {
    float cost = got_end->second - got_start->second;
    return cost;
  }
}

// A* search in the graph
bool AStarSearcher::search(Node::Ptr const &s, Node::Ptr const &t) {
  // cout << "A* start " << s << " end " << t << endl;

  // check that start and end are valid nodes
  Node::Ptr start = graph->get_node_ptr(s->id, s->d);
  if(!start) {
    cout << "Start node " << s << " is not a valid node" << endl;
    return false;
  }
  Node::Ptr end = graph->get_node_ptr(t->id, t->d);
  if(!end) {
    cout << "End node " << t << " is not a valid node" << endl;
    return false;
  }

  // check that graph has been set
  if(!graph) {
    cout << "set_graph() must be called before a_star_search()" << endl;
    return false;
  }

  // clear book keeping data structures
  while(!q.empty()) {q.pop();}
  g_score.clear();
  came_from.clear();

  // init
  q.push(make_pair(start, 0.f));  // priority does not matter for first element
  // cout << "Pushed " << start << ", " << 0.0 << " to q" << endl;
  g_score[start] = 0.0;
  came_from[start] = start;
  
  // search
  bool found = false;
  while(!q.empty()) {
    pair<Node::Ptr, double> p = q.top();
    q.pop();
    Node::Ptr current = p.first;
    // cout << "Popped " << current << " from q" << endl;
    if(current->equald(end)) {
      found = true;
      break;
    }

    // process neighbors
    // cout << "Has " << current->nbrs.size() << " neighbors" << endl;
    for(int i = 0; i < current->nbrs.size(); i++) {
      Node::Ptr nbr = current->nbrs[i];
      double nbr_g_score = g_score[current] + current->distances[i];
      // cout << "Considering Nbr " << nbr << ", with potential g_score = " << nbr_g_score << endl;
      bool nbr_unexplored = true;
      if(g_score.count(nbr)) {
        nbr_unexplored = false;
        // cout << nbr << " is previously explored, with a g_score of " << g_score[nbr] << endl;
      } else {
        // cout << nbr << " is unexplored" << endl;
      }
      if(nbr_unexplored || nbr_g_score < g_score[nbr]) {
        g_score[nbr] = nbr_g_score;
        double nbr_f_score = nbr_g_score + a_star_heuristic(nbr, end);
        q.push(make_pair(nbr, nbr_f_score));
        // cout << "Pushed " << nbr << ", " << nbr_f_score << " to q" << endl;
        came_from[nbr] = current;
      }
    }
  }

  // get path
  // cout << "Calculating path" << endl;
  path.clear();
  if(found) {
    path.insert(path.begin(), end);
    while(!path[0]->equald(start)) {
      path.insert(path.begin(), came_from[path[0]]);
    }
    // cout << "Path calculated" << endl;
    // cout << "Path is" << endl;
    // for(int i = 0; i < path.size(); i++) cout << path[i] << endl;
  } else {
    cout << "Path not found" << endl;
  }
  return found;
}
