#include "tree.h"

namespace octet {
    class lsystems : public app {

        class node {
            vec3 pos;
            float angle;
        public:
            node() {
                pos = vec3(0.0f, 0.0f, 0.0f);
                angle = 0.0f;
            }

            node(vec3 pos_, float angle_) {
                pos = pos_;
                angle = angle_;
            }

            vec3& get_pos() {
                return pos;
            }

            float& get_angle() {
                return angle;
            }
        };

        const float PI = 3.14159265f;
        const float SEGMENT_LENGTH = 0.5f;
        const float SEGMENT_WIDTH = 0.1f;

        ref<visual_scene> app_scene;

        tree t;

        dynarray<node> node_stack;

        random rand_generator;
        float tree_max_y = 0.0f;

    public:
        lsystems(int argc, char **argv) : app(argc, argv) {
        }

        void app_init() {
            t.read_file("assets/lsystems/lsystem.txt");

            app_scene = new visual_scene();
            app_scene->create_default_camera_and_lights();
            app_scene->get_camera_instance(0)->get_node()->translate(vec3(0.0f, 0.0f, 1.0f));

            create_geometry();
            rand_generator = rand();
        }

        void draw_world(int x, int y, int w, int h) {
            handle_input();

            app_scene->begin_render(w, h);

            app_scene->update(1.0f / 30.0f);

            app_scene->render((float)w / h);
        }

        void handle_input() {
            if (is_key_going_down(key_space)) {
                t.iterate();

                app_scene = new visual_scene();
                app_scene->create_default_camera_and_lights();

                create_geometry();

                app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, tree_max_y / 2.0f, tree_max_y / 2.0f));
            }
        }

        vec3 draw_segment(vec3 start_pos, float angle) {
            vec3 mid_pos;
            vec3 end_pos;

            mid_pos.x() = start_pos.x() + SEGMENT_LENGTH * cos((angle + 90.0f) * PI / 180.0f);
            mid_pos.y() = start_pos.y() + SEGMENT_LENGTH * sin((angle + 90.0f) * PI / 180.0f);
            end_pos.x() = start_pos.x() + SEGMENT_LENGTH * 2.0f * cos((90.0f + angle) * PI / 180.0f);
            end_pos.y() = start_pos.y() + SEGMENT_LENGTH * 2.0f * sin((90.0f + angle) * PI / 180.0f);

            if (tree_max_y < end_pos.y()) {
                tree_max_y = end_pos.y();
            }

            mat4t mtw;
            mtw.loadIdentity();
            vec4 color = vec4(rand_generator.get(0.5f, 1.0f), rand_generator.get(0.5f, 1.0f), rand_generator.get(0.5f, 1.0f), 1.0f);
            mesh_box *box = new mesh_box(vec3(SEGMENT_WIDTH, SEGMENT_LENGTH, SEGMENT_WIDTH), mtw);

            scene_node *node = new scene_node();
            app_scene->add_child(node);
            app_scene->add_mesh_instance(new mesh_instance(node, box, new material(color)));

            node->translate(mid_pos);
            node->rotate(angle, vec3(0.0f, 0.0f, 1.0f));

            return end_pos;
        }

        void create_geometry() {
            dynarray<char> axiom = t.get_axiom();
            vec3 pos = vec3(0.0f, 0.0f, 0.0f);
            float angle = 0.0f;
            for (unsigned int i = 0; i < axiom.size(); ++i) {
                if (axiom[i] == '+') {
                    angle += 45.0f;
                } else if (axiom[i] == '-') {
                    angle -= 45.0f;
                } else if (axiom[i] == '[') {
                    node n = node(pos, angle);
                    node_stack.push_back(n);
                } else if (axiom[i] == ']') {
                    node n = node_stack[node_stack.size() - 1];
                    node_stack.pop_back();
                    angle = n.get_angle();
                    pos = n.get_pos();
                } else if (axiom[i] == 'F') {
                    pos = draw_segment(pos, angle);
                }
            }

        }

    };
}
