#ifndef LIDAR_CLUSTERER_HPP
#define LIDAR_CLUSTERER_HPP

#include <vector>
#include <cmath>

// Struct to represent a 2D point
struct Point2D {
    double x;
    double y;
    int ray_index; // Index of the corresponding lidar ray
    int cluster_id = -1; // -1: unassigned, 0: unclustered, >0: cluster ID
};

class LidarClusterer {
public:
    LidarClusterer(double epsilon, int min_pts);

    // Function to perform DBSCAN clustering on lidar points
    void process(std::vector<Point2D>& points);

private:
    double epsilon_; // Maximum distance between points to be considered neighbors
    int min_pts_; // Minimum number of points to form a cluster 

    double calculate_distance(const Point2D& p1, const Point2D& p2) const;
    std::vector<int> get_neighbors(const std::vector<Point2D>& points, int point_idx) const;
    void expand_cluster(std::vector<Point2D>& points, int point_idx, std::vector<int>& neighbors, int cluster_id) const;
};

#endif // LIDAR_CLUSTERER_HPP