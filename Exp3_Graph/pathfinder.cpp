#include "pathfinder.hpp"

namespace chr {

	constexpr double UTM_K0 = 0.9996;
	constexpr double M_PI = 3.1415829535898;
	constexpr double WGS84_A = 6378137.0;
	constexpr double WGS84_B = 6356752.314245;
	constexpr double WGS84_E2 = 0.00669437999013;

	point2d location::wgs84_to_utm(double lon, double lat) {
		double lat_rad = lat * M_PI / 180.0;
		double lon_rad = lon * M_PI / 180.0;
		int zone = utm_zone(lon);
		double lon_origin = (zone - 1) * 6.0 - 180.0 + 3.0;
		double lon_origin_rad = lon_origin * M_PI / 180.0;
		double e2 = WGS84_E2;
		double e4 = e2 * e2;
		double e6 = e4 * e2;
		double A0 = 1 - e2 / 4 - 3 * e4 / 64 - 5 * e6 / 256;
		double A2 = 3.0 / 8.0 * (e2 + e4 / 4 + 15 * e6 / 128);
		double A4 = 15.0 / 256.0 * (e4 + 3 * e6 / 4);
		double A6 = 35 * e6 / 3072;
		double M = WGS84_A * (A0 * lat_rad - A2 * std::sin(2 * lat_rad) + A4 * std::sin(4 * lat_rad) - A6 * std::sin(6 * lat_rad));
		double N = WGS84_A / std::sqrt(1 - e2 * std::sin(lat_rad) * std::sin(lat_rad));
		double T = std::tan(lat_rad) * std::tan(lat_rad);
		double C = e2 / (1 - e2) * std::cos(lat_rad) * std::cos(lat_rad);
		double A = (lon_rad - lon_origin_rad) * std::cos(lat_rad);
		double x = UTM_K0 * N * (A + (1 - T + C) * A * A * A / 6 + (5 - 18 * T + T * T + 72 * C - 58 * e2) * A * A * A * A * A / 120);
		double y = UTM_K0 * (M + N * std::tan(lat_rad) * (A * A / 2 + (5 - T + 9 * C + 4 * C * C) * A * A * A * A / 24 + (61 - 58 * T + T * T + 600 * C - 330 * e2) * A * A * A * A * A * A / 720));
		x += 500000.0;
		if (lat < 0) y += 10000000.0;
		return { x, y };
	}

	location::location(size_t id, const std::string& name, const point2d& globe_coordinate)
		:id_(id), name_(name), globe_(globe_coordinate), plane_(wgs84_to_utm(globe_coordinate.y(), globe_coordinate.x())) {
		if (id == 0) throw std::invalid_argument("地点ID不可为0");
	}
	double location::road_length_to(size_t id) const {
		auto it = roads_.find(id);
		return it != roads_.end() ? it->second : 0.0;
	}

	city::city(unsigned id, const std::string& name)
		:id_(id), name_(name) {
		if (id == 0) throw std::invalid_argument("城市ID不可为0");
	}
	std::shared_ptr<location> city::place(size_t id) const {
		auto it = places_.find(id);
		return it != places_.end() ? it->second : nullptr;
	}
	std::shared_ptr<location> city::local_place(unsigned serial) const {
		auto it = places_.find(place_id(id_, serial));
		return it != places_.end() ? it->second : nullptr;
	}
	location& city::add_place(size_t id, const std::string& name, const point2d& globe_coordinate) {
		if (has_place(id)) {
			throw std::invalid_argument("地点ID已存在");
		}
		places_[id] = std::make_shared<location>(id, name, globe_coordinate);
		return *places_[id];
	}
	location& city::add_local_place(unsigned serial, const std::string& name, const point2d& globe_coordinate) {
		auto id = place_id(id_, serial);
		if (has_place(id)) {
			throw std::invalid_argument("地点ID已存在");
		}
		places_[id] = std::make_shared<location>(id, name, globe_coordinate);
		return *places_[id];
	}
	bool city::remove_place(size_t id) {
		if (!has_place(id)) return false;
		for (auto& [place_id, place] : places_) {
			place->remove_road(id);
		}
		return places_.erase(id) > 0;
	}
	bool city::remove_local_place(unsigned serial) {
		auto id = place_id(id_, serial);
		if (!has_place(id)) return false;
		for (auto& [place_id, place] : places_) {
			place->remove_road(id);
		}
		return places_.erase(id) > 0;
	}
	double city::add_road(size_t from, size_t to) const {
		auto from_place = place(from);
		auto to_place = place(to);
		if (!from_place || !to_place) {
			throw std::invalid_argument("地点ID不存在");
		}
		from_place->add_road(to_place->id(), to_place->plane());
		return from_place->road_length_to(to);
	}
	double city::add_local_road(unsigned from_serial, unsigned to_serial) const {
		auto from = place_id(id_, from_serial);
		auto to = place_id(id_, to_serial);
		auto from_place = place(from);
		auto to_place = place(to);
		if (!from_place || !to_place) {
			throw std::invalid_argument("地点ID不存在");
		}
		from_place->add_road(to_place->id(), to_place->plane());
		return from_place->road_length_to(to);
	}
	double city::add_bidirectional_road(size_t from, size_t to) const {
		double dist1 = add_road(from, to);
		double dist2 = add_road(to, from);
		return std::max(dist1, dist2);
	}
	double city::add_local_bidirectional_road(unsigned from_serial, unsigned to_serial) const {
		auto from = place_id(id_, from_serial);
		auto to = place_id(id_, to_serial);
		double dist1 = add_road(from, to);
		double dist2 = add_road(to, from);
		return std::max(dist1, dist2);
	}
	double city::add_intercity_road(size_t from, size_t to, const point2d& plane_coordinate) const {
		auto from_place = place(from);
		if (!from_place) {
			throw std::invalid_argument("地点ID不存在");
		}
		from_place->add_road(to, plane_coordinate);
		return from_place->road_length_to(to);
	}
	bool city::has_road(size_t from, size_t to) const {
		auto from_place = place(from);
		return from_place ? from_place->has_road_to(to) : false;
	}
	bool city::has_local_road(unsigned from_serial, unsigned to_serial) const {
		auto from = place_id(id_, from_serial);
		auto to = place_id(id_, to_serial);
		auto from_place = place(from);
		return from_place ? from_place->has_road_to(to) : false;
	}
	double city::road_length(size_t from, size_t to) const {
		auto from_place = place(from);
		return from_place ? from_place->road_length_to(to) : 0.0;
	}
	double city::local_road_length(unsigned from_serial, unsigned to_serial) const {
		auto from = place_id(id_, from_serial);
		auto to = place_id(id_, to_serial);
		auto from_place = place(from);
		return from_place ? from_place->road_length_to(to) : 0.0;
	}

	std::shared_ptr<location> plat::place(size_t id) const {
		unsigned town_id = id >> 32;
		auto town_it = towns_.find(town_id);
		return town_it == towns_.end() ? nullptr : town_it->second->place(id);
	}
	city& plat::add_town(unsigned id, const std::string& name) {
		if (has_town(id)) {
			throw std::invalid_argument("城市ID已存在");
		}
		towns_[id] = std::make_shared<city>(id, name);
		return *towns_[id];
	}
	std::shared_ptr<city> plat::town(unsigned id) const {
		auto town_it = towns_.find(id);
		return town_it == towns_.end() ? nullptr : town_it->second;
	}
	bool plat::remove_town(unsigned id) {
		return towns_.erase(id) > 0;
	}
	std::vector<unsigned> plat::get_all_town_ids() const {
		std::vector<unsigned> ids;
		for (const auto& [id, town] : towns_) {
			ids.push_back(id);
		}
		return ids;
	}
	std::vector<size_t> plat::find_path(size_t from, size_t to) const {
		auto start_place = place(from);
		auto goal_place = place(to);
		if (!start_place || !goal_place) {
			return {};
		}
		return astar_search(*start_place, *goal_place);
	}
	void plat::print_path(const std::vector<size_t>& path) const {
		if (path.empty()) {
			std::cout << "路径为空\n";
			return;
		}
		double sum = 0;
		auto begin = place(path[0]);
		if (!begin) {
			throw std::runtime_error("路径包含未知地点");
		}
		std::cout << "途经：" << begin->name();
		for (size_t i = 1; i < path.size(); i++) {
			auto from = place(path[i - 1]);
			auto to = place(path[i]);
			if (!from || !to) {
				throw std::runtime_error("路径包含未知地点");
			}
			double dist = from->road_length_to(to->id());
			std::string dist_str;
			if (dist <= 1e3) {
				dist_str = std::to_string(dist) + "米";
			}
			else if (dist <= 1e7) {
				dist_str = std::to_string(dist / 1e3) + "公里";
			}
			else {
				dist_str = std::to_string(dist / 1e7) + "万公里";
			}
			if (to->has_road_to(from->id())) {
				std::cout << "<-" << dist_str << "->";
			}
			else {
				std::cout << "=-" << dist_str << "->";
			}
			std::cout << to->name();
			sum += dist;
		}
		std::cout << "，抵达，总计";
		std::string sum_str;
		if (sum <= 1e3) {
			sum_str = std::to_string(sum) + "米";
		}
		else if (sum <= 1e7) {
			sum_str = std::to_string(sum / 1e3) + "公里";
		}
		else {
			sum_str = std::to_string(sum / 1e7) + "万公里";
		}
		std::cout << sum_str << "\n";
	}
	std::vector<size_t> plat::astar_search(const location& start, const location& goal) const {
		std::priority_queue<
			astar_node,
			std::vector<astar_node>,
			std::greater<>
		> open_list;
		std::unordered_map<size_t, astar_node> all_nodes;
		std::unordered_set<size_t> closed_set;
		double h_start = astar_node::heuristic(start.plane(), goal.plane());
		open_list.push({ 0.0,h_start,0 });
		all_nodes[start.id()] = { 0.0,h_start,0 };
		while (!open_list.empty()) {
			astar_node curr_node = open_list.top();
			open_list.pop();
			size_t curr_id = 0;
			for (const auto& [id, node] : all_nodes) {
				if (node == curr_node) {
					curr_id = id;
					break;
				}
			}
			if (curr_id == goal.id()) {
				return reconstruct_path(all_nodes, goal.id());
			}
			if (closed_set.count(curr_id)) continue;
			closed_set.insert(curr_id);
			auto curr_place = place(curr_id);
			if (!curr_place) continue;
			for (const auto& [neighbor_id, distance] : curr_place->roads()) {
				if (closed_set.count(neighbor_id)) continue;
				auto neighbor_place = place(neighbor_id);
				if (!neighbor_place) continue;
				double g_new = all_nodes[curr_id].g + distance;
				double h_new = astar_node::heuristic(neighbor_place->plane(), goal.plane());
				if (!all_nodes.count(neighbor_id) || g_new < all_nodes[neighbor_id].g) {
					double f_new = g_new + h_new;
					all_nodes[neighbor_id] = { g_new,f_new,curr_id };
					open_list.push({ g_new,f_new,curr_id });
				}
			}
		}
		return {};
	}
	std::vector<size_t> plat::reconstruct_path(const std::unordered_map<size_t, astar_node>& nodes, size_t end_id) const {
		std::vector<size_t> path;
		size_t curr_id = end_id;
		while (curr_id != 0) {
			path.push_back(curr_id);
			auto it = nodes.find(curr_id);
			if (it == nodes.end()) break;
			curr_id = it->second.parent;
		}
		std::reverse(path.begin(), path.end());
		return path;
	}
	std::vector<std::pair<size_t, std::string>> plat::fuzzy_find_places(const std::string& keyword) const {
		std::vector<std::pair<size_t, std::string>> results;
		if (keyword.empty()) {
			return results;
		}
		std::string lower_keyword = keyword;
		std::transform(lower_keyword.begin(), lower_keyword.end(), lower_keyword.begin(), ::tolower);
		for (const auto& [town_id, town_ptr] : towns_) {
			for (const auto& [place_id, place_ptr] : town_ptr->places()) {
				std::string lower_name = town_ptr->name() + place_ptr->name();
				std::transform(lower_name.begin(), lower_name.end(), lower_name.begin(), ::tolower);
				if (lower_name.find(lower_keyword) != std::string::npos) {
					results.emplace_back(place_id, town_ptr->name() + " " + place_ptr->name());
				}
			}
		}
		std::sort(results.begin(), results.end(), [](const auto& a, const auto& b) { return a.second < b.second; });
		return results;
	}
	double plat::add_road(size_t from, size_t to) const {
		unsigned from_town_id = from >> 32;
		unsigned to_town_id = to >> 32;
		auto from_town = town(from_town_id);
		auto to_town = town(to_town_id);
		if (!from_town) {
			throw std::invalid_argument("起点所在城市不存在");
		}
		if (from_town_id == to_town_id) {
			return from_town->add_road(from, to);
		}
		else {
			auto to_place = place(to);
			if (!to_place) {
				throw std::invalid_argument("终点地点不存在");
			}
			return from_town->add_intercity_road(from, to, to_place->plane());
		}
	}
	double plat::add_bidirectional_road(size_t from, size_t to) const {
		unsigned from_town_id = from >> 32;
		unsigned to_town_id = to >> 32;
		if (from_town_id == to_town_id) {
			auto town_ptr = town(from_town_id);
			if (!town_ptr) {
				throw std::invalid_argument("城市不存在");
			}
			return town_ptr->add_bidirectional_road(from, to);
		}
		else {
			double dist1 = add_road(from, to);
			double dist2 = add_road(to, from);
			return std::max(dist1, dist2);
		}
	}
	bool plat::has_road(size_t from, size_t to) const {
		auto from_place = place(from);
		return from_place ? from_place->has_road_to(to) : false;
	}
	double plat::road_length(size_t from, size_t to) const {
		auto from_place = place(from);
		return from_place ? from_place->road_length_to(to) : 0.0;
	}
	void plat::save_all_cities_as_json(const std::filesystem::path& path) const {
		std::ofstream file(path);
		if (!file.is_open()) throw std::runtime_error("无法打开文件进行保存: " + path.string());
		file << "{\n";
		file << "  \"cities\": [\n";
		bool first_city = true;
		for (const auto& [town_id, town_ptr] : towns_) {
			if (!first_city) file << ",\n";
			else first_city = false;
			file << "    {\n";
			file << "      \"id\": " << town_id << ",\n";
			file << "      \"name\": " << town_ptr->name() << ",\n";
			file << "      \"places\": [\n";
			bool first_place = true;
			for (const auto& [place_id, place_ptr] : town_ptr->places()) {
				if (!first_place) file << ",\n";
				else first_place = false;
				file << "        {\n";
				file << "          \"id\": " << place_id << ",\n";
				file << "          \"name\": \"" << place_ptr->name() << "\",\n";
				file << "          \"longitude\": " << place_ptr->longitude() << ",\n";
				file << "          \"latitude\": " << place_ptr->latitude() << "\n";
				file << "        }";
			}
			file << "\n      ],\n";
			file << "      \"roads\": [\n";
			bool first_road = true;
			std::unordered_set<std::string> saved_roads;
			for (const auto& [from_id, from_loc] : town_ptr->places()) {
				for (const auto& [to_id, distance] : from_loc->roads()) {
					std::string road_key = std::to_string(from_id) + "-" + std::to_string(to_id);
					std::string reverse_key = std::to_string(to_id) + "-" + std::to_string(from_id);
					if (saved_roads.count(reverse_key)) {
						continue;
					}
					bool is_bidirectional = false;
					if (town_ptr->places().count(to_id)) {
						const auto& to_roads = town_ptr->places().at(to_id)->roads();
						is_bidirectional = (to_roads.count(from_id) > 0);
					}
					if (!first_road) file << ",\n";
					else first_road = false;
					file << "        {\n";
					file << "          \"from\": " << from_id << ",\n";
					file << "          \"to\": " << to_id << ",\n";
					file << "          \"bidirectional\": " << (is_bidirectional ? "true" : "false") << "\n";
					file << "        }";
					saved_roads.insert(road_key);
				}
			}
			file << "\n      ]\n";
			file << "    }";
		}
		file << "\n  ]\n";
		file << "}\n";
	}
	void plat::load_all_cities_from_json(const std::filesystem::path& path) {
		std::ifstream file(path);
		if (!file.is_open()) throw std::runtime_error("无法打开文件进行加载: " + path.string());
		towns_.clear();
		std::string line;
		bool in_cities_array = false;
		bool in_city_object = false;
		bool in_places_array = false;
		bool in_roads_array = false;
		unsigned current_city_id = 0;
		std::shared_ptr<city> current_city = nullptr;
		while (std::getline(file, line)) {
			line.erase(0, line.find_first_not_of(" \t\r\n"));
			line.erase(line.find_last_not_of(" \t\r\n") + 1);
			if (line == "\"cities\": [") {
				in_cities_array = true;
				continue;
			}
			if (line == "]" && in_cities_array && !in_city_object) {
				in_cities_array = false;
				break;
			}
			if (in_cities_array) {
				if (in_city_object) {
					std::string name;
					if (line.find("\"id\":") != std::string::npos) {
						size_t pos = line.find(":");
						std::string id_str = line.substr(pos + 1);
						id_str.erase(0, id_str.find_first_not_of(" \t"));
						id_str.erase(id_str.find_last_not_of(" \t,") + 1);
						current_city_id = std::stoull(id_str);
					}
					else if (line.find("\"name\":") != std::string::npos) {
						size_t pos = line.find(":");
						std::string val = line.substr(pos + 1);
						val.erase(0, val.find_first_not_of(" \t\""));
						val.erase(val.find_last_not_of(" \t\",") + 1);
						name = val;
					}
					if (current_city_id != 0 && !name.empty()) {
						current_city = std::make_shared<city>(current_city_id, name);
						towns_[current_city_id] = current_city;
						continue;
					}
					if (line == "\"places\": [") {
						in_places_array = true;
						in_roads_array = false;
						continue;
					}
					if (line == "\"roads\": [") {
						in_roads_array = true;
						in_places_array = false;
						continue;
					}
					if (in_places_array && line.find("{") != std::string::npos) {
						size_t id = 0;
						std::string name;
						double lon = 0.0, lat = 0.0;
						while (std::getline(file, line)) {
							line.erase(0, line.find_first_not_of(" \t"));
							line.erase(line.find_last_not_of(" \t,") + 1);
							if (line == "}") break;
							if (line.find("\"id\":") != std::string::npos) {
								size_t pos = line.find(":");
								std::string val = line.substr(pos + 1);
								val.erase(0, val.find_first_not_of(" \t"));
								val.erase(val.find_last_not_of(" \t,") + 1);
								id = std::stoull(val);
							}
							else if (line.find("\"name\":") != std::string::npos) {
								size_t pos = line.find(":");
								std::string val = line.substr(pos + 1);
								val.erase(0, val.find_first_not_of(" \t\""));
								val.erase(val.find_last_not_of(" \t\",") + 1);
								name = val;
							}
							else if (line.find("\"longitude\":") != std::string::npos) {
								size_t pos = line.find(":");
								std::string val = line.substr(pos + 1);
								val.erase(0, val.find_first_not_of(" \t"));
								val.erase(val.find_last_not_of(" \t,") + 1);
								lon = std::stod(val);
							}
							else if (line.find("\"latitude\":") != std::string::npos) {
								size_t pos = line.find(":");
								std::string val = line.substr(pos + 1);
								val.erase(0, val.find_first_not_of(" \t"));
								val.erase(val.find_last_not_of(" \t,") + 1);
								lat = std::stod(val);
							}
						}
						if (current_city && id != 0 && !name.empty()) {
							current_city->add_place(id, name, { lat, lon });
						}
						continue;
					}
					if (in_roads_array && line.find("{") != std::string::npos) {
						size_t from = 0, to = 0;
						bool bidirectional = false;
						while (std::getline(file, line)) {
							line.erase(0, line.find_first_not_of(" \t"));
							line.erase(line.find_last_not_of(" \t,") + 1);
							if (line == "}") break;
							if (line.find("\"from\":") != std::string::npos) {
								size_t pos = line.find(":");
								std::string val = line.substr(pos + 1);
								val.erase(0, val.find_first_not_of(" \t"));
								val.erase(val.find_last_not_of(" \t,") + 1);
								from = std::stoull(val);
							}
							else if (line.find("\"to\":") != std::string::npos) {
								size_t pos = line.find(":");
								std::string val = line.substr(pos + 1);
								val.erase(0, val.find_first_not_of(" \t"));
								val.erase(val.find_last_not_of(" \t,") + 1);
								to = std::stoull(val);
							}
							else if (line.find("\"bidirectional\":") != std::string::npos) {
								size_t pos = line.find(":");
								std::string val = line.substr(pos + 1);
								val.erase(0, val.find_first_not_of(" \t"));
								bidirectional = (val.find("true") != std::string::npos);
							}
						}
						if (current_city && from != 0 && to != 0) {
							if (bidirectional) {
								current_city->add_bidirectional_road(from, to);
							}
							else {
								current_city->add_road(from, to);
							}
						}
						continue;
					}
				}
				if (line == "{") {
					in_city_object = true;
					current_city_id = 0;
					current_city = nullptr;
					continue;
				}
				if (line == "}," || line == "}") {
					in_city_object = false;
					in_places_array = false;
					in_roads_array = false;
					continue;
				}
			}
		}
	}
}