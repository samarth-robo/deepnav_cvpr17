# DeepNav: Learning to Navigate Large Cities
- Most of the code exists to collect data and label it
- Actual training code is minimal (you will find prototxt files for all 3 DeepNav networks in the 'ml' directory)
- Unfortunately I haven't had the time to extensively document everything, but here's what the important executables do:
  - `test_graph_maker` makes a city graph given the dataset
  - `test_search` demonstrates A* search
  - `test_dataset_maker` generates labels for a city and stores them in the city's directory
  - `test_tester` applies the learned model for navigating in a city graph
  - `test_eval` evaluates the learned model
- Data directory structure:
  - Each city has its own directory in `data/dataset` e.g. `data/dataset/new_york`
  - The data collection script stores the images, nodes and links in this directory
  - Once that is done, the `test_dataset_maker` executable generates labels in caffe-readable format in a subdirectory e.g. `new_york/pair` for `DeepNav-pair`, `new_york/distance` for `DeepNav-distance` etc.
  - You will have to collect your own data but I have provided some sample labels in `data/dataset/small` so that you can understand the format of training data
  - Each dataset folder (e.g. `/data/dataset/san_francisco`) should have a `box.txt`. `box.txt` has 2 lines: first line is top left limit of the city in `latitude, longitude` format and second line is bottom right limit of the city in `latitude, longitude` format. You can use Google Maps to figure out these limits for your city/area. For example, `data/dataset/small/box.txt` is for San Francisco.
  - The `python/combine_datasets.py` script can be used to combine the labels of multiple cities for large experiments

# Trained models
- [DeepNav-direction](https://drive.google.com/open?id=0B5_6NRwNEqMPMVNUTkZqd3NNRWM)
- [DeepNav-distance](https://drive.google.com/open?id=0B5_6NRwNEqMPUFktZjBiTHE1T0k)
- [DeepNav-pair](https://drive.google.com/open?id=0B5_6NRwNEqMPZzg3MVcybHJTWVk)

# Citation
@InProceedings{Brahmbhatt_2017_CVPR,<br/>
author = {Brahmbhatt, Samarth and Hays, James},<br/>
title = {DeepNav: Learning to Navigate Large Cities},<br/>
booktitle = {The IEEE Conference on Computer Vision and Pattern Recognition (CVPR)},<br/>
month = {July},<br/>
year = {2017}<br/>
}
