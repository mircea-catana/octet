////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//
#include "tree.h"

namespace octet {
    /// Scene containing a box with octet.
    class lsystems : public app {

        // scene for drawing box
        ref<visual_scene> app_scene;

        tree t;

    public:
        /// this is called when we construct the class before everything is initialised.
        lsystems(int argc, char **argv) : app(argc, argv) {
        }

        /// this is called once OpenGL is initialized
        void app_init() {
            t.read_file("assets/lsystems/lsystem.txt");
        }

        /// this is called to draw the world
        void draw_world(int x, int y, int w, int h) {

        }

    };
}
