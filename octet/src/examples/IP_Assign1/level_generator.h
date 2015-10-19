#include <stdlib.h>
#include <time.h>

class level_generator {

	int **map = 0;
    int **map2 = 0;
	int width = 0;
	int height = 0;

public:

	int** generate_level(unsigned w, unsigned h, unsigned seed = 0) {
		
		if (seed == 0) {
			srand(time(NULL));
		} else {
			srand(seed);
		}
		
		width = w;
		height = h;
		map = new int*[height];
        map2 = new int*[height];

		for (unsigned i = 0; i < height; ++i) {
			map[i] = new int[width];
            map2[i] = new int[width];
			for (unsigned j = 0; j < width; ++j) {
				if (i == 0 || i == height - 1 || j == 0 || j == width - 1) {
					map[i][j] = 1;
				} else {
                    int wallProb = rand() % 100;
                    if (wallProb <= 40) {
                        map[i][j] = 1;
                    } else {
                        map[i][j] = 0;
                    }
				}
                map2[i][j] = 1;
			}
		}

        for (unsigned i = 0; i < 4; ++i) {
			smooth_map();
		}

        print_map();

		return map;
	}

    // Map generation based on Cellular Automata Method
    // Reference: http://www.roguebasin.com/index.php?title=Cellular_Automata_Method_for_Generating_Random_Cave-Like_Levels
    void smooth_map() {
        for (int i = 1; i < height - 1; ++i) {
            for (int j = 1; j < width - 1; ++j) {
                int adjCount1 = 0, adjCount2 = 0;

                for (int ii = -1; ii <= 1; ++ii) {
                    for (int jj = -1; jj <= 1; ++jj) {
                        if (map[i + ii][j + jj] == 1) {
                            adjCount1++;
                        }
                    }
                }

                for (int ii = i - 2; ii <= i + 2; ++ii) {
                    for (int jj = j - 2; jj <= j + 2; ++jj) {
                        if (abs(ii - i) == 2 && abs(jj - j) == 2)
                            continue;
                        if (ii<0 || jj<0 || ii >= height || jj >= width)
                            continue;
                        if (map[ii][jj] == 1)
                            adjCount2++;
                    }
                }

                if (adjCount1 >= 5 || adjCount2 <= 2)
                    map2[i][j] = 1;
                else
                    map2[i][j] = 0;
            }
        }

        for (int i = 1; i < height - 1; ++i) {
            for (int j = 1; j < width - 1; ++j) {
                map[i][j] = map2[i][j];
            }
        }
    }

    void print_map() {
        printf("\n");
        for (int i = 0; i < height; ++i) {
            for (int j = 0; j < width; ++j) {
                if (map[i][j]) {
                    printf("#", map[i][j]);
                } else {
                    printf(" ", map[i][j]);
                }
            }
            printf("\n");
        }
        printf("\n");
    }

};