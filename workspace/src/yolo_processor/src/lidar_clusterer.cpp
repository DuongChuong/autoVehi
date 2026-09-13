#include "yolo_processor/lidar_clusterer.hpp"

LidarClusterer::LidarClusterer(double epsilon, int min_pts)
    : epsilon_(epsilon), min_pts_(min_pts) {}

double LidarClusterer::calculate_distance(const Point2D& p1, const Point2D& p2) const {
    return std::hypot(p1.x - p2.x, p1.y - p2.y);
}

std::vector<int> LidarClusterer::get_neighbors(const std::vector<Point2D>& points, int point_idx) const {
    std::vector<int> neighbors;
    for (size_t i = 0; i < points.size(); ++i) {
        if (calculate_distance(points[point_idx], points[i]) <= epsilon_) {
            neighbors.push_back(i);
        }
    }
    return neighbors;
}

void LidarClusterer::expand_cluster(std::vector<Point2D>& points, int point_idx, std::vector<int>& neighbors, int cluster_id) const {
    points[point_idx].cluster_id = cluster_id;

    for (size_t i = 0; i < neighbors.size(); ++i) {
        int current_neighbor = neighbors[i];

        if(points[current_neighbor].cluster_id == 0) { // Unclustered
            points[current_neighbor].cluster_id = cluster_id;
        }

        if (points[current_neighbor].cluster_id == -1) { // Unassigned
            points[current_neighbor].cluster_id = cluster_id;
            std::vector<int> new_neighbors = get_neighbors(points, current_neighbor);
            if (new_neighbors.size() >= static_cast<size_t>(min_pts_)) {
                neighbors.insert(neighbors.end(), new_neighbors.begin(), new_neighbors.end());
            }
        }
    }
}

void LidarClusterer::process(std::vector<Point2D>& points) {
    int cluster_id = 1; // Start cluster IDs from 1

    for (size_t i = 0; i < points.size(); ++i) {
        if (points[i].cluster_id != -1) { // Already assigned
            continue;
        }

        std::vector<int> neighbors = get_neighbors(points, i);
        if (neighbors.size() < static_cast<size_t>(min_pts_)) {
            points[i].cluster_id = 0; // Mark as noise
        } else {
            expand_cluster(points, i, neighbors, cluster_id);
            ++cluster_id; // Move to the next cluster ID
        }
    }
}