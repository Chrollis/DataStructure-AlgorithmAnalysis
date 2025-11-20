#include "pathfinder.hpp"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <string>
#include <algorithm>

using namespace chr;

void print_help() {
    std::cout << "========== 地图导航系统命令行模式 ==========\n";
    std::cout << "命令格式: -command [参数]\n";
    std::cout << "可用命令:\n";
    std::cout << "  -load <path>                   加载地图\n";
    std::cout << "  -save <path>                   保存地图\n";
    std::cout << "  -city -add <id> <name>         添加城市\n";
    std::cout << "  -city -del <id>                删除城市\n";
    std::cout << "  -loc -add <city_id> <serial> <name> <lon> <lat>  添加地点\n";
    std::cout << "  -loc -del <city_id> <serial>   删除地点\n";
    std::cout << "  -road -add -uni <from_id> <to_id>     添加单向道路\n";
    std::cout << "  -road -add -bi <from_id> <to_id>      添加双向道路\n";
    std::cout << "  -road -del -uni <from_id> <to_id>     删除单向道路\n";
    std::cout << "  -road -del -bi <from_id> <to_id>      删除双向道路\n";
    std::cout << "  -search -locs <keyword>        查询地点\n";
    std::cout << "  -search -path <from_id> <to_id> 路径查找\n";
    std::cout << "  -show                          显示所有城市\n";
    std::cout << "  -clear                         清空屏幕\n";
    std::cout << "  -exit                          退出\n";
    std::cout << "  -help                          显示帮助\n";
}

void show_all_cities(plat& p) {
    std::cout << "\n--- 所有城市 ---\n";
    auto city_ids = p.get_all_town_ids();

    if (city_ids.empty()) {
        std::cout << "暂无城市数据\n";
        return;
    }
    for (unsigned city_id : city_ids) {
        auto town_ptr = p.town(city_id);
        std::cout << "城市" << city_id << ": " << town_ptr->name() << "，包含 " << town_ptr->places().size() << " 个地点\n";

        for (const auto& [place_id, place_ptr] : town_ptr->places()) {
            unsigned serial = place_id & 0xFFFFFFFF;
            std::cout << "  - 地点" << serial << ": " << place_ptr->name()
                << " (经度: " << place_ptr->longitude()
                << ", 纬度: " << place_ptr->latitude() << ")\n";
        }
    }
}

bool parse_command(int argc, char* argv[], plat& p) {
    if (argc < 2) {
        std::cout << "错误: 缺少命令参数\n";
        print_help();
        return false;
    }

    std::string command = argv[1];

    if (command == "-help") {
        print_help();
        return true;
    }
    else if (command == "-load") {
        if (argc < 3) {
            std::cout << "错误: 缺少文件路径参数\n";
            return false;
        }
        std::string path = argv[2];
        if (path.starts_with("\"") && path.ends_with("\"")) {
            path = path.substr(1, path.length() - 2);
        }
        try {
            p.load_all_cities_from_json(path);
            std::cout << "地图加载成功!\n";
        }
        catch (const std::exception& e) {
            std::cout << "加载失败: " << e.what() << std::endl;
        }
    }
    else if (command == "-save") {
        if (argc < 3) {
            std::cout << "错误: 缺少文件路径参数\n";
            return false;
        }
        std::string path = argv[2];
        if (path.starts_with("\"") && path.ends_with("\"")) {
            path = path.substr(1, path.length() - 2);
        }
        try {
            p.save_all_cities_as_json(path);
            std::cout << "地图保存成功!\n";
        }
        catch (const std::exception& e) {
            std::cout << "保存失败: " << e.what() << std::endl;
        }
    }
    else if (command == "-city") {
        if (argc < 3) {
            std::cout << "错误: 缺少城市操作类型\n";
            return false;
        }
        std::string op = argv[2];
        if (op == "-add") {
            if (argc < 5) {
                std::cout << "错误: 缺少城市ID或名称\n";
                return false;
            }
            unsigned id = std::stoul(argv[3]);
            std::string name = argv[4];
            try {
                p.add_town(id, name);
                std::cout << "城市添加成功!\n";
            }
            catch (const std::exception& e) {
                std::cout << "错误: " << e.what() << std::endl;
            }
        }
        else if (op == "-del") {
            if (argc < 4) {
                std::cout << "错误: 缺少城市ID\n";
                return false;
            }
            unsigned id = std::stoul(argv[3]);
            if (p.remove_town(id)) {
                std::cout << "城市删除成功!\n";
            }
            else {
                std::cout << "城市不存在!\n";
            }
        }
        else {
            std::cout << "错误: 未知的城市操作类型: " << op << "\n";
            return false;
        }
    }
    else if (command == "-loc") {
        if (argc < 3) {
            std::cout << "错误: 缺少地点操作类型\n";
            return false;
        }
        std::string op = argv[2];
        if (op == "-add") {
            if (argc < 8) {
                std::cout << "错误: 缺少地点参数\n";
                return false;
            }
            unsigned city_id = std::stoul(argv[3]);
            unsigned serial = std::stoul(argv[4]);
            std::string name = argv[5];
            double lon = std::stod(argv[6]);
            double lat = std::stod(argv[7]);

            auto town_ptr = p.town(city_id);
            if (!town_ptr) {
                std::cout << "城市不存在!\n";
                return false;
            }

            try {
                town_ptr->add_local_place(serial, name, { lat, lon });
                std::cout << "地点添加成功!\n";
            }
            catch (const std::exception& e) {
                std::cout << "错误: " << e.what() << std::endl;
            }
        }
        else if (op == "-del") {
            if (argc < 5) {
                std::cout << "错误: 缺少城市ID或地点序列号\n";
                return false;
            }
            unsigned city_id = std::stoul(argv[3]);
            unsigned serial = std::stoul(argv[4]);

            auto town_ptr = p.town(city_id);
            if (!town_ptr) {
                std::cout << "城市不存在!\n";
                return false;
            }

            if (town_ptr->remove_local_place(serial)) {
                std::cout << "地点删除成功!\n";
            }
            else {
                std::cout << "地点不存在!\n";
            }
        }
        else {
            std::cout << "错误: 未知的地点操作类型: " << op << "\n";
            return false;
        }
    }
    else if (command == "-road") {
        if (argc < 4) {
            std::cout << "错误: 缺少道路操作参数\n";
            return false;
        }
        std::string op = argv[2];
        std::string type = argv[3];

        if (argc < 6) {
            std::cout << "错误: 缺少起点或终点ID\n";
            return false;
        }
        size_t from = std::stoull(argv[4]);
        size_t to = std::stoull(argv[5]);

        if (op == "-add") {
            if (type == "-uni") {
                try {
                    double length = p.add_road(from, to);
                    std::cout << "单向道路添加成功! 长度: " << length << "米\n";
                }
                catch (const std::exception& e) {
                    std::cout << "错误: " << e.what() << std::endl;
                }
            }
            else if (type == "-bi") {
                try {
                    double length = p.add_bidirectional_road(from, to);
                    std::cout << "双向道路添加成功! 长度: " << length << "米\n";
                }
                catch (const std::exception& e) {
                    std::cout << "错误: " << e.what() << std::endl;
                }
            }
            else {
                std::cout << "错误: 未知的道路类型: " << type << "\n";
                return false;
            }
        }
        else if (op == "-del") {
            if (type == "-uni") {
                try {
                    unsigned city_id = from >> 32;
                    if (p.town(city_id)->place(from)->remove_road(to))
                        std::cout << "单向道路删除成功!\n";
                    else
                        std::cout << "单向道路删除失败!\n";
                }
                catch (const std::exception& e) {
                    std::cout << "错误: " << e.what() << std::endl;
                }
            }
            else if (type == "-bi") {
                try {
                    unsigned city_id = from >> 32;
                    bool success1 = false, success2 = false;

                    if (p.town(city_id)->place(from)->remove_road(to)) {
                        success1 = true;
                        std::cout << "去向道路删除成功!\n";
                    }
                    else {
                        std::cout << "去向道路删除失败!\n";
                    }

                    city_id = to >> 32;
                    if (p.town(city_id)->place(to)->remove_road(from)) {
                        success2 = true;
                        std::cout << "来向道路删除成功!\n";
                    }
                    else {
                        std::cout << "来向道路删除失败!\n";
                    }

                    if (success1 && success2) {
                        std::cout << "双向道路删除成功!\n";
                    }
                }
                catch (const std::exception& e) {
                    std::cout << "错误: " << e.what() << std::endl;
                }
            }
            else {
                std::cout << "错误: 未知的道路类型: " << type << "\n";
                return false;
            }
        }
        else {
            std::cout << "错误: 未知的道路操作类型: " << op << "\n";
            return false;
        }
    }
    else if (command == "-search") {
        if (argc < 3) {
            std::cout << "错误: 缺少搜索类型\n";
            return false;
        }
        std::string type = argv[2];
        if (type == "-locs") {
            if (argc < 4) {
                std::cout << "错误: 缺少搜索关键词\n";
                return false;
            }
            std::string keyword = argv[3];
            auto results = p.fuzzy_find_places(keyword);

            if (results.empty()) {
                std::cout << "未找到匹配的地点\n";
            }
            else {
                std::cout << "找到 " << results.size() << " 个匹配地点:\n";
                for (const auto& [id, name] : results) {
                    unsigned city_id = id >> 32;
                    unsigned serial = id & 0xFFFFFFFF;
                    std::cout << "ID: " << id << " (城市" << city_id << "-地点" << serial << "), 名称: " << name << std::endl;
                }
            }
        }
        else if (type == "-path") {
            if (argc < 5) {
                std::cout << "错误: 缺少起点或终点ID\n";
                return false;
            }
            size_t from = std::stoull(argv[3]);
            size_t to = std::stoull(argv[4]);
            auto path = p.find_path(from, to);

            if (path.empty()) {
                std::cout << "未找到路径\n";
            }
            else {
                std::cout << "找到路径:\n";
                p.print_path(path);
            }
        }
        else {
            std::cout << "错误: 未知的搜索类型: " << type << "\n";
            return false;
        }
    }
    else if (command == "-show") {
        show_all_cities(p);
    }
    else if (command == "-clear") {
        system("cls");
    }
    else if (command == "-exit") {
        std::cout << "感谢使用，再见!\n";
        return false;
    }
    else {
        std::cout << "错误: 未知命令: " << command << "\n";
        print_help();
        return false;
    }

    return true;
}

int main(int argc, char* argv[]) {
    plat p;

    // 如果有命令行参数，执行命令后进入交互模式
    if (argc > 1) {
        if (!parse_command(argc, argv, p)) {
            return 1; // 如果命令执行失败或用户要求退出，直接返回
        }
        // 命令执行成功后，继续进入交互模式而不是退出
        std::cout << "命令执行完成，进入交互模式...\n";
    }

    // 交互模式
    std::cout << "欢迎使用地图导航系统!\n";
    std::cout << "输入 -help 查看可用命令\n";

    std::string input;
    while (true) {
        std::cout << "\n> ";
        std::getline(std::cin, input);

        if (input.empty()) continue;

        // 解析输入为命令行参数
        std::vector<std::string> args;
        std::istringstream iss(input);
        std::string token;

        while (iss >> token) {
            args.push_back(token);
        }

        // 转换为char*数组格式
        std::vector<char*> cargs;
        cargs.push_back(argv[0]); // 程序名
        for (auto& arg : args) {
            cargs.push_back(&arg[0]);
        }

        if (!parse_command(cargs.size(), cargs.data(), p)) {
            if (args[0] == "-exit") {
                break;
            }
        }
    }

    std::cout << "感谢使用，再见!\n";
    return 0;
}
