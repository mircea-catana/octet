#include <stdlib.h>
#include <time.h>

class level_generator {

	int **map = 0;
	int width = 0;
	int height = 0;

public:
	level_generator() {

	}

	~level_generator() {

	}

	int** generate_level(unsigned w, unsigned h, unsigned seed = 0) {
		
		if (seed == 0) {
			srand(time(NULL));
		} else {
			srand(seed);
		}
		
		width = w;
		height = h;
		map = new int*[height];

		for (unsigned i = 0; i < height; ++i) {
			map[i] = new int[width];
			for (unsigned j = 0; j < width; ++j) {
				if (i == 0 || i == height - 1 || j == 0 || j == width - 1) {
					map[i][j] = 1;
				} else {
					map[i][j] = rand() % 2;
				}
			}
		}

		for (unsigned i = 0; i < 3; ++i) {
			smooth_map();
		}

		return map;
	}

	void smooth_map() {
		for (unsigned i = 1; i < height-1; ++i) {
			for (unsigned j = 1; j < width-1; ++j) {
				int density =	map[i - 1][j - 1] + map[i - 1][j] + map[i - 1][j + 1] +
					map[i][j - 1] + map[i][j + 1] +
					map[i + 1][j - 1] + map[i + 1][j] + map[i + 1][j + 1];
				if (density < 4) {
					map[i][j] = 0;
				} else if (density > 5) {
					map[i][j] = 1;
				}
			}
		}
	}

};