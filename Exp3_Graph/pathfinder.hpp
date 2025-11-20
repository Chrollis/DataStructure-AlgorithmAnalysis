#ifndef PATHFINDER_HPP
#define PATHFINDER_HPP

#include <iostream>
#include <list>
#include <queue>
#include <cmath>
#include <unordered_map>
#include <stdexcept>
#include <unordered_set>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <functional>

namespace chr {

	class point2d {
	private:
		double x_, y_;
	public:
		point2d() :x_(0), y_(0) {}
		point2d(double x, double y) :x_(x), y_(y) {}
		double x() const { return x_; }
		double y() const { return y_; }
		double& rx() { return x_; }
		double& ry() { return y_; }
		point2d operator+(const point2d& other) const { return { x_ + other.x_,y_ + other.y_ }; }
		point2d operator-(const point2d& other) const { return { x_ - other.x_,y_ - other.y_ }; }
		point2d operator*(double k) const { return { x_ * k,y_ * k }; }
		point2d operator/(double k) const { return { x_ / k,y_ / k }; }
		bool operator==(const point2d& other) const { return x_ == other.x_ && y_ == other.y_; }
		bool operator>(const point2d& other) const { return x_ > other.x_ || y_ > other.y_; }
		bool operator<(const point2d& other) const { return x_ < other.x_ || y_ < other.y_; }
		bool operator!=(const point2d& other) const { return !(*this == other); }
		bool operator>=(const point2d& other) const { return !(*this < other); }
		bool operator<=(const point2d& other) const { return !(*this > other); }
		double dot(const point2d& other) const { return x_ * other.x_ + y_ * other.y_; }
		double norm() const { return std::sqrt(x_ * x_ + y_ * y_); }
		double distance_to(const point2d& other) const { return (other - *this).norm(); }
		point2d unit_vector() const { return *this / norm(); }
	};

	class location {
	public:
		static int utm_zone(double lon) { return static_cast<int>((lon + 180.0) / 6.0) + 1; }
		static point2d wgs84_to_utm(double lon, double lat);
	private:
		size_t id_;
		std::string name_;
		point2d globe_;
		point2d plane_;
		std::unordered_map<size_t, double> roads_;
	public:
		location() :id_(0) {}
		location(size_t id, const std::string& name, const point2d& globe_coordinate);
		size_t id() const { return id_; }
		const std::string& name() const { return name_; }
		point2d globe() const { return globe_; }
		double longitude() const { return globe_.y(); }
		double latitude() const { return globe_.x(); }
		point2d plane() const { return plane_; }
		const std::unordered_map<size_t, double>& roads() const { return roads_; }
		void add_road(size_t id, const point2d& plane_coordinate) { roads_[id] = plane_.distance_to(plane_coordinate); }
		bool remove_road(size_t id) {return roads_.erase(id) > 0;}
		bool has_road_to(size_t id) const {return roads_.find(id) != roads_.end();}
		double road_length_to(size_t id) const;
	};

	class city {
	public:
		static size_t place_id(unsigned city_id, unsigned place_serial) { return ((size_t)(city_id) << 32) + place_serial; }
	private:
		unsigned id_;
		std::string name_;
		std::unordered_map<size_t, std::shared_ptr<location>> places_;
	public:
		city() :id_(0) {}
		city(unsigned id, const std::string& name);
		size_t id() const { return id_; }
		const std::string& name() const { return name_; }
		const std::unordered_map<size_t, std::shared_ptr<location>>& places() const { return places_; }
		bool has_place(size_t id) const { return places_.find(id) != places_.end(); }
		bool has_local_place(unsigned serial) const { return places_.find(place_id(id_, serial)) != places_.end(); }
		std::shared_ptr<location> place(size_t id) const;
		std::shared_ptr<location> local_place(unsigned serial) const;
		location& add_place(size_t id, const std::string& name, const point2d& globe_coordinate);
		location& add_local_place(unsigned serial, const std::string& name, const point2d& globe_coordinate);
		bool remove_place(size_t id);
		bool remove_local_place(unsigned serial);
	public:
		double add_road(size_t from, size_t to) const;
		double add_local_road(unsigned from_serial, unsigned to_serial) const;
		double add_bidirectional_road(size_t from, size_t to) const;
		double add_local_bidirectional_road(unsigned from_serial, unsigned to_serial) const;
		double add_intercity_road(size_t from, size_t to, const point2d& plane_coordinate) const;
		bool has_road(size_t from, size_t to) const;
		bool has_local_road(unsigned from_serial, unsigned to_serial) const;
		double road_length(size_t from, size_t to) const;
		double local_road_length(unsigned from_serial, unsigned to_serial) const;
	};

	class plat {
	private:
		std::unordered_map<unsigned, std::shared_ptr<city>> towns_;
		struct astar_node {
			double g;
			double f;
			size_t parent;
			bool operator>(const astar_node& other) const { return f > other.f; }
			bool operator==(const astar_node& other) const { return g == other.g && f == other.f && parent == other.parent; }
			static double heuristic(const point2d& a, const point2d& b) { return a.distance_to(b); }
		};
	public:
		const std::unordered_map<unsigned, std::shared_ptr<city>>& towns() const { return towns_; }
		std::shared_ptr<location> place(size_t id) const;
		city& add_town(unsigned id, const std::string& name);
		bool has_town(unsigned id) { return towns_.find(id) != towns_.end(); }
		std::shared_ptr<city> town(unsigned id) const;
		bool remove_town(unsigned id);
		std::vector<unsigned> get_all_town_ids() const;
		std::vector<size_t> find_path(size_t from, size_t to) const;
		void print_path(const std::vector<size_t>& path) const;
	public:
		std::vector<std::pair<size_t, std::string>> fuzzy_find_places(const std::string& keyword) const;
		double add_road(size_t from, size_t to) const;
		double add_bidirectional_road(size_t from, size_t to) const;
		bool has_road(size_t from, size_t to) const;
		double road_length(size_t from, size_t to) const;
	public:
		void save_all_cities_as_json(const std::filesystem::path& path) const;
		void load_all_cities_from_json(const std::filesystem::path& path);
	private:
		std::vector<size_t> astar_search(const location& start, const location& goal) const;
		std::vector<size_t> reconstruct_path(const std::unordered_map<size_t, astar_node>& nodes, size_t end_id) const;
	};
}

#endif // !PATHFINDER_HPP
