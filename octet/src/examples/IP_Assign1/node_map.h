#include <fstream>
#include <vector>
#include <math.h>

class node_map {

    struct node {
        int type = 0;
        int x = 0;
        int y = 0;
        int g_cost = 999;
        int h_cost = 0;
        node* parent = nullptr;
    };

    const int G_NORMAL = 10;
    const int G_DIAGONAL = 14;

    std::vector<node*> open_list;
    std::vector<node*> closed_list;

public:

    static const int map_width = 50;
    static const int map_height = 50;
    node map[map_height][map_width];

    node *start_node;
    node *end_node;
    node *current_node;

    void init(int **map_) {
        for (int i = 0; i < map_height; ++i) {
            for (int j = 0; j < map_width; ++j) {
                node temp;
                temp.type = map_[i][j];
                temp.x = j;
                temp.y = i;
                map[i][j] = temp;

                if (map_[i][j] == 2) {
                    end_node = &map[i][j];
                } else if (map_[i][j] == 3) {
                    start_node = &map[i][j];
                }
            }
        }
    }

    void compute_shortest_path() {
        compute_h_values();

        open_list = std::vector<node*>();
        closed_list = std::vector<node*>();

        open_list.clear();
        closed_list.clear();

        while (true) {
            select_current_node();
            if (current_node->x == end_node->x && current_node->y == end_node->y) {
                break;
            }
            add_neighbors_to_open();
            compute_g_values();
        }
    }

    node* get_end_node() {
        return end_node;
    }

    node* get_start_node() {
        return start_node;
    }

    void print_map() {
        for (int i = 0; i < map_height; ++i) {
            for (int j = 0; j < map_width; ++j) {
                printf("%d ", map[i][j].type);
            }
            printf("\n");
        }
    }

    void print_map_h() {
        for (int i = 0; i < map_height; ++i) {
            for (int j = 0; j < map_width; ++j) {
                printf("%d ", map[i][j].h_cost);
            }
            printf("\n");
        }
    }

    void print_map_g() {
        for (int i = 0; i < map_height; ++i) {
            for (int j = 0; j < map_width; ++j) {
                printf("%d ", map[i][j].g_cost);
            }
            printf("\n");
        }
    }

    void print_map_f() {
        for (int i = 0; i < map_height; ++i) {
            for (int j = 0; j < map_width; ++j) {
                printf("%d ", get_f_cost(map[i][j]));
            }
            printf("\n");
        }
    }

private:

    int get_f_cost(node n) {
        return n.g_cost + n.h_cost;
    }

    void read_file() {
        std::ifstream file("map.csv");

        char buffer[2048];
        int i = 0;
        while (!file.eof()) {
            file.getline(buffer, sizeof(buffer));

            char *b = buffer;
            for (int j = 0;; ++j) {
                char *e = b;
                while (*e != 0 && *e != ',') {
                    ++e;
                }

                node temp;
                temp.type = std::atoi(b);
                temp.x = j;
                temp.y = i;
                map[i][j] = temp;

                if (map[i][j].type == 2) {
                    end_node = &map[i][j];
                } else if (map[i][j].type == 3) {
                    map[i][j].g_cost = 0;
                    start_node = &map[i][j];
                }

                if (*e != ',') {
                    break;
                }

                b = e + 1;
            }

            ++i;
        }
    }

    bool is_in_open(node *n) {
        for (int i = 0; i < open_list.size(); ++i) {
            if (open_list[i]->x == n->x && open_list[i]->y == n->y) {
                return true;
            }
        }
        return false;
    }

    bool is_in_closed(node *n) {
        for (int i = 0; i < closed_list.size(); ++i) {
            if (closed_list[i]->x == n->x && closed_list[i]->y == n->y) {
                return true;
            }
        }
        return false;
    }

    void compute_h_values() {
        for (int i = 0; i < map_height; ++i) {
            for (int j = 0; j < map_width; ++j) {
                int dx = abs(end_node->x - j);
                int dy = abs(end_node->y - i);
                int min = (dx < dy) ? dx : dy;
                int max = (dx > dy) ? dx : dy;
                map[i][j].h_cost = G_DIAGONAL*min + G_NORMAL*(max - min);
            }
        }
    }

    void select_current_node() {

        if (closed_list.size() == 0) {
            current_node = start_node;
            int i = current_node->y;
            int j = current_node->x;
            closed_list.push_back(&map[i][j]);
        } else {
            int min = 9999;
            for (int i = 0; i < open_list.size(); ++i) {
                if (get_f_cost(*(open_list[i])) < min && !is_in_closed(open_list[i])) {
                    current_node = open_list[i];
                    min = get_f_cost(*current_node);
                }
            }

            int i = current_node->y;
            int j = current_node->x;
            closed_list.push_back(&map[i][j]);
        }
    }

    void add_neighbors_to_open() {
        int startX = (current_node->x - 1 >= 0) ? current_node->x - 1 : 0;
        int endX = (current_node->x + 1 < map_width) ? current_node->x + 1 : map_width - 1;
        int startY = (current_node->y - 1 >= 0) ? current_node->y - 1 : 0;
        int endY = (current_node->y + 1 < map_height) ? current_node->y + 1 : map_height - 1;

        for (int i = startY; i <= endY; ++i) {
            for (int j = startX; j <= endX; ++j) {
                if (i != current_node->y || j != current_node->x) {
                    if (map[i][j].type != 1 && !is_in_open(&map[i][j]) && !is_in_closed(&map[i][j])) {
                        open_list.push_back(&map[i][j]);
                    }
                }
            }
        }
    }

    void compute_g_values() {
        for (int i = 0; i < open_list.size(); ++i) {

            node *n = open_list[i];
            if (abs(n->x - current_node->x) <= 1 && abs(n->y - current_node->y) <= 1) {

                int count = abs(n->x - current_node->x) + abs(n->y - current_node->y);
                if (count == 1) {
                    // case node is horizontal or vertical
                    if (current_node->g_cost + G_NORMAL < n->g_cost) {
                        n->g_cost = current_node->g_cost + G_NORMAL;
                        n->parent = current_node;
                    }

                } else if (count == 2) {
                    // case node is diagonal
                    if (current_node->g_cost + G_DIAGONAL < n->g_cost) {
                        n->g_cost = current_node->g_cost + G_DIAGONAL;
                        n->parent = current_node;
                    }
                }
            }
        }
    }

};