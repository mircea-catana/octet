#include <stdlib.h>
#include <time.h>

#include "node_map.h"

namespace octet {
    class level_generator {

        int **int_map = 0;
        int **int_map2 = 0;
        int width = 0;
        int height = 0;

        node_map nmap;

    public:

        // Map generation based on Cellular Automata Method
        // Reference: http://www.roguebasin.com/index.php?title=Cellular_Automata_Method_for_Generating_Random_Cave-Like_Levels
        void generate_level(unsigned w, unsigned h, unsigned seed = 0) {

            if (seed == 0) {
                srand(time(NULL));
            } else {
                srand(seed);
            }

            width = w;
            height = h;
            int_map = new int*[height];
            int_map2 = new int*[height];

            for (unsigned i = 0; i < height; ++i) {
                int_map[i] = new int[width];
                int_map2[i] = new int[width];
                for (unsigned j = 0; j < width; ++j) {
                    if (i == 0 || i == height - 1 || j == 0 || j == width - 1) {
                        int_map[i][j] = 1;
                    } else {
                        int wallProb = rand() % 100;
                        if (wallProb <= 40) {
                            int_map[i][j] = 1;
                        } else {
                            int_map[i][j] = 0;
                        }
                    }
                    int_map2[i][j] = 1;
                }
            }

            for (unsigned i = 0; i < 4; ++i) {
                smooth_map();
            }
            nmap.init(int_map);
        }

        node_map* get_node_map() {
            return &nmap;
        }

    private:

        void smooth_map() {
            for (int i = 1; i < height - 1; ++i) {
                for (int j = 1; j < width - 1; ++j) {
                    int adjCount1 = 0, adjCount2 = 0;

                    for (int ii = -1; ii <= 1; ++ii) {
                        for (int jj = -1; jj <= 1; ++jj) {
                            if (int_map[i + ii][j + jj] == 1) {
                                adjCount1++;
                            }
                        }
                    }

                    for (int ii = i - 2; ii <= i + 2; ++ii) {
                        for (int jj = j - 2; jj <= j + 2; ++jj) {
                            if (abs(ii - i) == 2 && abs(jj - j) == 2)
                                continue;
                            if (ii < 0 || jj < 0 || ii >= height || jj >= width)
                                continue;
                            if (int_map[ii][jj] == 1)
                                adjCount2++;
                        }
                    }

                    if (adjCount1 >= 5 || adjCount2 <= 2)
                        int_map2[i][j] = 1;
                    else
                        int_map2[i][j] = 0;
                }
            }

            for (int i = 1; i < height - 1; ++i) {
                for (int j = 1; j < width - 1; ++j) {
                    int_map[i][j] = int_map2[i][j];
                }
            }
        }

    };
}
