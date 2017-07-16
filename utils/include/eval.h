#ifndef EVAL_H
#define EVAL_H

#include <tester.h>

class BaseEvaluator {
  public:
    BaseEvaluator(std::string city, std::string dest_name, std::string experiment);
    void evaluate();
  protected:
    BaseTester::Ptr tester;
    std::string base_dir;
    std::string city, dest_name;
    std::vector<Node::Ptr> test_locs;
    std::string test_locs_filename;
    std::string experiment;
};

class DistanceEvaluator : public BaseEvaluator {
  public:
    DistanceEvaluator(std::string city, std::string dest_name);
};

class DistanceGISTEvaluator : public BaseEvaluator {
  public:
    DistanceGISTEvaluator(std::string city, std::string dest_name);
};

class DirectionEvaluator : public BaseEvaluator {
  public:
    DirectionEvaluator(std::string city, std::string dest_name);
};

class PairEvaluator : public BaseEvaluator {
  public:
    PairEvaluator(std::string city, std::string dest_name);
};

class RandomWalkEvaluator : public BaseEvaluator {
  public:
    RandomWalkEvaluator(std::string city, std::string dest_name);
};
#endif
