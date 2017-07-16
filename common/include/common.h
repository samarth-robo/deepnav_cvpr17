#ifndef COMMON_H
#define COMMON_H

#include <graph.h>

#include <string>
#include <vector>

bool write_path(std::vector<Node::Ptr> path, std::string filename);
bool clean_im_list(std::string filename);
bool im_exists(std::string filename);

#endif
