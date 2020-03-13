#include <iostream>
#include <fstream>
#include <random>
#include <ctime>
#include <vector>
#include <set>
#include <functional>
#include <chrono>
#include <thread>
#include <cstdlib>

typedef std::vector<std::vector<bool>> bool_matrix;

const int MAX_TIME_DELAY = 2e9;
const int DEFAULT_TIME_DELAY = 1e3;

void create_start_field(bool_matrix &field, int size_n = 10, int size_m = 10) {
    std::default_random_engine generator(time(nullptr));
    std::uniform_int_distribution<int> distribution(0, 1);
    auto dice = std::bind(distribution, generator);

    field.resize(size_n, std::vector<bool>(size_m));

    for (auto &row : field) {
        for (auto cell : row) {
            cell = dice();
        }
    }
}

void read_start_field(bool_matrix &field, const std::string &file) {
    std::ifstream in(file, std::ifstream::in);
    int size_n, size_m;
    in >> size_n >> size_m;

    if (size_n <= 0 || size_m <= 0) {
        std::cout << "please write correct field size";
    }
    field.resize(size_n, std::vector<bool>(size_m));

    for (auto &row : field) {
        for (auto cell : row) {
            int dice;
            in >> dice;
            cell = dice != 0;
        }
    }
}

void print_field(bool_matrix &field) {
    for (auto &row : field) {
        for (auto cell : row) {
            std::cout << (cell ? '#' : '.') << " ";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}

int normalize(int a, int n) {
    return (n + a) % n;
}

int get_neighbors_count(bool_matrix &field, int i, int j) {
    int result = 0;
    for (int a = i - 1; a <= i + 1; a++) {
        for (int b = j - 1; b <= j + 1; b++) {
            if (a == i && b == j) continue;
            int a_normalize = normalize(a, field.size());
            int b_normalize = normalize(b, field[i].size());
            result += (field[a_normalize][b_normalize] ? 1 : 0);
        }
    }
    return result;
}

bool set_cell(bool_matrix &field, int i, int j, bool alive, bool_matrix &prev_field) {
    field[i][j] = alive;
    return (prev_field[i][j] != alive);
}

void add_neighbors_to_list(bool_matrix &field, int a, int b, std::set<std::pair<int, int>> &list) {
    for (int i = a - 1; i <= a + 1; i++) {
        for (int j = b - 1; j <= b + 1; j++) {
            list.insert({ normalize(i, field.size()), normalize(j, field[a].size()) });
        }
    }
}

bool view_cell(bool_matrix &field, int i, int j) {
    int neighbors_count = get_neighbors_count(field, i, j);
    bool current_alive = field[i][j];
    bool alive = true;
    if (current_alive) {
        if (neighbors_count < 2 || neighbors_count > 3) {
            alive = false;
        }
    } else {
        if (neighbors_count != 3) {
            alive = false;
        }
    }
    return alive;
}

void run(bool_matrix &field, int steps_delay) {
    std::set<std::pair<int, int>> candidates;
    std::set<std::pair<int, int>> new_candidates;
    bool_matrix new_field = field;

    for (int i = 0; i < field.size(); i++) {
        for (int j = 0; j < field[i].size(); j++) {
            candidates.insert({ i, j });
        }
    }
    int steps_count = 1;
    std::cout << "start field" << std::endl;
    print_field(field);
    while (!candidates.empty()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(steps_delay));
        new_candidates.clear();
        for (auto cell : candidates) {
            int i = cell.first;
            int j = cell.second;
            if (set_cell(new_field, i, j, view_cell(field, i, j), field)) {
                add_neighbors_to_list(field, i, j, new_candidates);
            }
        }
        std::swap(field, new_field);
        std::swap(candidates, new_candidates);
        if (candidates.empty()) {
            break;
        }

        //system("CLS");

        std::cout << "step = " << steps_count++ << std::endl;
        print_field(field);
    }
}

int main(int argc, char *argv[]) {
    bool_matrix game_field(0, std::vector<bool>(0));

    int time_delay = DEFAULT_TIME_DELAY;

    if (argc > 1) {
        if (std::string(argv[1]) == "c" || std::string(argv[1]) == "custom") {
            if (argc == 4) {
                time_delay = atoi(argv[3]);
            }
            read_start_field(game_field, argv[2]);
        } else {
            if (argc == 5) {
                time_delay = atoi(argv[4]);
            }
            create_start_field(game_field, atoi(argv[2]), atoi(argv[3]));
        }

    } else {
        std::cout << "run with arguments: \n"
                     "\"random/r n m (time_delay)\" "
                     "or \"custom/c file_name\" (time_delay)" << std::endl;
        return 0;
    }

    if (time_delay > MAX_TIME_DELAY) {
        std::cout << "too big delay_time" << std::endl;
    }

    run(game_field, time_delay);

    return 0;
}
