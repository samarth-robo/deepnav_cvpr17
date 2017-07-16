#ifndef VIS_H
#define VIS_H

#include <string>

#include <graph.h>

#include <pcl/pcl_base.h>
#include <pcl/point_types.h>
#include <pcl/visualization/pcl_visualizer.h>

class Vis {
  private:
    pcl::visualization::PCLVisualizer::Ptr viewer;
    unsigned int count;
    bool shown;
    std::string get_next_id(std::string word);
  public:
    Vis(std::string window_name = std::string("cloud"));
    template <typename PointT>
    bool addPointCloud(typename pcl::PointCloud<PointT>::ConstPtr cloud,
        double pt_size = 1.0);
    template <typename PointT>
    bool addPointCloud(typename pcl::PointCloud<PointT>::ConstPtr cloud,
        std::vector<double> colors, double pt_size = 1.0);
    bool addLine(pcl::PointXYZ p0, pcl::PointXYZ p1,
        std::vector<double> colors = {1.0, 1.0, 0.0});
    bool addCube(pcl::PointXYZ min_pt, pcl::PointXYZ max_pt,
        std::vector<double> colors = {1.0, 1.0, 1.0});
    bool addGraph(Graph::Ptr graph, double pt_size = 7.0,
        std::vector<double> node_colors = {0.0, 1.0, 0.0},
        std::vector<double> edge_colors = {1.0, 1.0, 0.0},
        std::vector<double> text_colors = {0.0, 1.0, 1.0});
    bool addPath(std::vector<Node::Ptr> path,
        std::vector<double> arrow_colors = {0.0, 0.0, 1.0});
    bool addText3D(std::string text, pcl::PointXYZ p,
        std::vector<double> text_colors = {0.0, 1.0, 1.0});
    void show(bool block = true);
};

#endif
