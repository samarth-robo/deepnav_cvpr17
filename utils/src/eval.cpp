#include <eval.h>

using namespace std;

BaseEvaluator::BaseEvaluator(string city, string dest_name, string experiment) :
  city(city), dest_name(dest_name), experiment(experiment) {
  base_dir = string("../data/dataset/") + city + string("/");
  test_locs_filename = base_dir + dest_name + string("_test_locs_850.txt");
}

void BaseEvaluator::evaluate() {
  Graph::Ptr graph = tester->get_graph();

  // read test locations
  test_locs.clear();
  cout << "Loading test locations" << endl;
  ifstream test_locs_file(test_locs_filename.c_str());
  if(!test_locs_file.is_open()) {
    cout << "Could not open " << test_locs_filename << " for reading" << endl;
    return;
  }
  string dummy_s;
  getline(test_locs_file, dummy_s);
  float dummy_f;
  char dummy_c;
  for(size_t id; test_locs_file >> id >> dummy_c >> dummy_f >> dummy_c >> dummy_f;) {
    Node::Ptr n = graph->get_valid_node(id);
    if(n) test_locs.push_back(n);
  }
  cout << "Found " << test_locs.size() << " test locations for " << dest_name
                                        << endl;

  string path_length_filename = base_dir + dest_name + string("_") + experiment +
      string("_path_lengths_850.txt");

  // multiple iterations for random walker
  size_t n_iters = experiment == string("random") ? 20 : 1;
  float success_rate = 0.f, avg_steps = 0.f, astar_avg_steps = 0.f,
      astar_avg_dist = 0.f;
  for(int iter = 0; iter < n_iters; iter++) {
    ofstream path_length_file(path_length_filename);
    if(!path_length_file.is_open()) {
      cout << "Could not open " << path_length_filename << " for writing" << endl;
      return;
    }
    size_t path_length_sum = 0, n_success = 0, shortest_path_length_sum = 0;
    float shortest_path_cost_sum = 0.f;
    for(auto const &test_loc : test_locs) {
      cout << "Testing start location " << test_loc << ", iter " << iter << endl;
      if (tester->find(test_loc->id)) {
        n_success++;
        size_t path_length = tester->get_path_length();
        path_length_sum += path_length;
        path_length_file << test_loc << " : " << path_length << endl;
        cout << "Found path, length = " << path_length << endl;
      } else {
        path_length_file << test_loc << " : -1" << endl;
        cout << "Path not found" << endl;
      }
      tester->gather_astar_stats(test_loc->id);
      shortest_path_length_sum += tester->get_shortest_path_length();
      shortest_path_cost_sum += tester->get_shortest_path_cost();
    }
    path_length_file.close();
    cout << path_length_filename << " written" << endl;

    success_rate    += float(n_success) / test_locs.size();
    astar_avg_steps += float(shortest_path_length_sum) / test_locs.size();
    astar_avg_dist  += shortest_path_cost_sum / test_locs.size();
    if(n_success > 0)
      avg_steps       += float(path_length_sum) / n_success;
  }
  success_rate    /= n_iters;
  avg_steps       /= n_iters;
  astar_avg_steps /= n_iters;
  astar_avg_dist  /= n_iters;

  string results_filename = base_dir + dest_name + string("_") + experiment +
    string("_results_850.txt");
  ofstream results_file(results_filename);
  if(!results_file.is_open()) {
    cout << "Could not open " << results_filename << " to write" << endl;
    return;
  }
  results_file << "Success rate = "     << success_rate*100.f << endl
               << "Avg. # steps = "     << avg_steps << endl
               << "A* Avg. # steps = "  << astar_avg_steps << endl
               << "A* Avg. distance = " << astar_avg_dist << endl;
  results_file.close();
  cout << results_filename << " written" << endl;
}

DistanceEvaluator::DistanceEvaluator(string city, string dest_name) :
  BaseEvaluator(city, dest_name, "distance") {
    tester.reset(new DistanceTester(city, dest_name));
}
DistanceGISTEvaluator::DistanceGISTEvaluator(string city, string dest_name) :
  BaseEvaluator(city, dest_name, "distance_gist") {
    tester.reset(new DistanceGISTTester(city, dest_name));
}
DirectionEvaluator::DirectionEvaluator(string city, string dest_name) :
  BaseEvaluator(city, dest_name, "direction") {
    tester.reset(new DirectionTester(city, dest_name));
}
PairEvaluator::PairEvaluator(string city, string dest_name) :
  BaseEvaluator(city, dest_name, "pair") {
    tester.reset(new PairTester(city, dest_name));
}
RandomWalkEvaluator::RandomWalkEvaluator(string city, string dest_name) :
  BaseEvaluator(city, dest_name, "random") {
    tester.reset(new RandomWalkTester(city, dest_name));
}
