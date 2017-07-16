// visualization utilities
// Samarth Brahmbhatt @ Geogia Tech
#include <vis.h>

#include <pcl/visualization/pcl_visualizer.h>

#include <sstream>

using namespace pcl;
using namespace pcl::visualization;
using namespace std;

// append count to a word to get ID for adding an object to the PCLViewer
string Vis::get_next_id(string word) {
  stringstream id;
  id << word << count;
  count++;
  return id.str();
}

Vis::Vis(string window_name) : viewer(new PCLVisualizer(window_name, false)),
  count(0), shown(false) {
  viewer->setBackgroundColor(0, 0, 0);
  viewer->addCoordinateSystem();
  viewer->setShowFPS(false);
}

// Visualize point cloud in PCLViewer
template <typename PointT>
bool Vis::addPointCloud(typename PointCloud<PointT>::ConstPtr cloud, double pt_size) {
  string id = get_next_id("cloud");
  bool done = viewer->addPointCloud<PointT>(cloud, id);
  if(done) {
    viewer->setPointCloudRenderingProperties(PCL_VISUALIZER_POINT_SIZE, pt_size, id);
  }
  return done;
} 
// instantiation
template bool Vis::addPointCloud<PointXYZ>(PointCloud<PointXYZ>::ConstPtr cloud, double pt_size);
template bool Vis::addPointCloud<PointXYZRGB>(PointCloud<PointXYZRGB>::ConstPtr cloud,
    double pt_size);

// Visualize point cloud with custom color handler
template <typename PointT>
bool Vis::addPointCloud(typename PointCloud<PointT>::ConstPtr cloud,
    vector<double> colors, double pt_size) {
  PointCloudColorHandlerCustom<PointT> ch(cloud, colors[0]*255, colors[1]*255, colors[2]*255);
  string id = get_next_id("cloud");
  bool done = viewer->addPointCloud<PointT>(cloud, ch, id);
  if(done) {
    viewer->setPointCloudRenderingProperties(PCL_VISUALIZER_POINT_SIZE, pt_size, id);
  }
  return done;
}
// instantiation
template bool Vis::addPointCloud<PointXYZ>(PointCloud<PointXYZ>::ConstPtr cloud,
    vector<double> colors, double pt_size);
template bool Vis::addPointCloud<PointXYZRGB>(PointCloud<PointXYZRGB>::ConstPtr cloud,
    vector<double> colors, double pt_size);

bool Vis::addLine(PointXYZ p0, PointXYZ p1, vector<double> colors) {
  string id = get_next_id("line");
  bool done = viewer->addLine(p0, p1, colors[0], colors[1], colors[2], id);
  return done;
}

bool Vis::addText3D(string text, PointXYZ p, vector<double> text_colors) {
  string id = get_next_id("text");
  bool done = viewer->addText3D(text, p, 1.0/20, text_colors[0], text_colors[1],
      text_colors[2]);
  return done;
}

// add wireframe cube from 3D BB
bool Vis::addCube(PointXYZ min_pt, PointXYZ max_pt, vector<double> colors) {
  bool done = true;
  done &= addLine(PointXYZ(min_pt.x, min_pt.y, min_pt.z),
      PointXYZ(max_pt.x, min_pt.y, min_pt.z), colors);
  done &= addLine(PointXYZ(min_pt.x, min_pt.y, min_pt.z),
      PointXYZ(min_pt.x, max_pt.y, min_pt.z), colors);
  done &= addLine(PointXYZ(max_pt.x, min_pt.y, min_pt.z),
      PointXYZ(max_pt.x, max_pt.y, min_pt.z), colors);
  done &= addLine(PointXYZ(min_pt.x, max_pt.y, min_pt.z),
      PointXYZ(max_pt.x, max_pt.y, min_pt.z), colors);
  done &= addLine(PointXYZ(min_pt.x, min_pt.y, min_pt.z),
      PointXYZ(min_pt.x, min_pt.y, max_pt.z), colors);
  done &= addLine(PointXYZ(min_pt.x, max_pt.y, min_pt.z),
      PointXYZ(min_pt.x, max_pt.y, max_pt.z), colors);
  done &= addLine(PointXYZ(max_pt.x, min_pt.y, min_pt.z),
      PointXYZ(max_pt.x, min_pt.y, max_pt.z), colors);
  done &= addLine(PointXYZ(max_pt.x, max_pt.y, min_pt.z),
      PointXYZ(max_pt.x, max_pt.y, max_pt.z), colors);
  done &= addLine(PointXYZ(min_pt.x, min_pt.y, max_pt.z),
      PointXYZ(max_pt.x, min_pt.y, max_pt.z), colors);
  done &= addLine(PointXYZ(min_pt.x, min_pt.y, max_pt.z),
      PointXYZ(min_pt.x, max_pt.y, max_pt.z), colors);
  done &= addLine(PointXYZ(min_pt.x, max_pt.y, max_pt.z),
      PointXYZ(max_pt.x, max_pt.y, max_pt.z), colors);
  done &= addLine(PointXYZ(max_pt.x, min_pt.y, max_pt.z),
      PointXYZ(max_pt.x, max_pt.y, max_pt.z), colors);
  return done;
}

// add graph visualization (nodes as points and edges as lines)
bool Vis::addGraph(Graph::Ptr graph, double pt_size, vector<double> node_colors,
    vector<double> edge_colors, vector<double> text_colors) {
  PointCloud<PointXYZ>::Ptr nodes(new PointCloud<PointXYZ>);
  for(int y = 0; y < graph->get_grid_dim_y(); y++) {
    for(int x = 0; x < graph->get_grid_dim_x(); x++) {
      Node::Ptr n = graph->get_node_ptr(y, x, Direction::N);
      if(n) {
        // cout << "(" << x << ", " << y << ") is a node" << endl;
        PointXYZ np = n->p;
        nodes->push_back(np);
        stringstream ss;
        ss << "(" << x << ", " << y << ")";
        addText3D(ss.str(), np, text_colors);
        for(int i = 0; i < n->nbrs.size(); i++) {
          addLine(np, (n->nbrs[i])->p, edge_colors);
        }
        n = graph->get_node_ptr(y, x, Direction::E);
        for(int i = 0; i < n->nbrs.size(); i++) {
          addLine(np, (n->nbrs[i])->p, edge_colors);
        }
      }
    }
  }
  addPointCloud<PointXYZ>(nodes, node_colors, pt_size);
}

// visualization of path as a series of arrows
bool Vis::addPath(vector<Node::Ptr> path, vector<double> arrow_colors) {
  bool done = true;
  for(int i = 0; i < path.size(); i++) {
    if(i+1 == path.size()) break;
    string id = get_next_id("path");
    done &= viewer->addArrow(path[i+1]->p, path[i]->p, arrow_colors[0], arrow_colors[1],
        arrow_colors[2], false, id);
  }
  return done;
}

void Vis::show(bool block) {
  if(!shown) {
    viewer->createInteractor();
    viewer->loadCameraParameters("../data/camera_params.cam");
    shown = true;
  }
  do {
    viewer->spinOnce(100);
    pcl_sleep(0.1);
  } while(block && !viewer->wasStopped());
  viewer->resetStoppedFlag();
}
