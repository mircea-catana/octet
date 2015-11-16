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
        float SEGMENT_WIDTH = 0.1f;

        ref<visual_scene> app_scene;

        tree t;

        dynarray<node> node_stack;

        float tree_max_y = 0.0f;

        material *bark_material;
        material *leaf_material;
        material *drawing_material;
        int current_level = 0;

        const int MIN_FILE_NO = 0;
        const int MAX_FILE_NO = 8;
        int current_file_no = 0;

        const float MIN_ANGLE = 20.0f;
        const float MAX_ANGLE = 45.0f;
        float angle_increment = 25.0f;

    public:
        lsystems(int argc, char **argv) : app(argc, argv) {
        }

        void app_init() {
            t.read_file(current_file_no);

            app_scene = new visual_scene();
            app_scene->create_default_camera_and_lights();
            app_scene->get_camera_instance(0)->get_node()->translate(vec3(0.0f, 0.0f, 1.0f));

            bark_material = new material(vec4(0.8f, 0.4f, 0.2f, 1.0f));
            leaf_material = new material(vec4(0.3f, 0.5f, 0.1f, 1.0f));
            drawing_material = bark_material;

            create_geometry();
        }

        void draw_world(int x, int y, int w, int h) {
            handle_input();

            app_scene->begin_render(w, h);

            app_scene->update(1.0f / 30.0f);

            app_scene->render((float)w / h);
        }

        void redraw() {
            app_scene = new visual_scene();
            app_scene->create_default_camera_and_lights();

            tree_max_y = 0.0f;
            create_geometry();

            app_scene->get_camera_instance(0)->get_node()->translate(vec3(0, tree_max_y / 2.0f, 2.0f));
        }

        void handle_input() {
            if (is_key_going_down(key_up)) {
                t.iterate();
                redraw();
            }

            if (is_key_going_down(key_down)) {
                t.deiterate();
                redraw();
            }

            if (is_key_going_down(key_right)) {
                current_file_no = ++current_file_no % MAX_FILE_NO;
                t.read_file(current_file_no);

                if (current_file_no == 6) {
                    angle_increment = 90.0f;
                } else if (current_file_no == 7) {
                    angle_increment = 60.0f;
                } else {
                    angle_increment = 25.0f;
                }

                redraw();
            }

            if (is_key_going_down(key_left)) {
                if (--current_file_no < 0) {
                    current_file_no = MAX_FILE_NO - 1;
                }
                t.read_file(current_file_no);

                if (current_file_no == 6) {
                    angle_increment = 90.0f;
                } else if (current_file_no == 7) {
                    angle_increment = 60.0f;
                } else {
                    angle_increment = 25.0f;
                }

                redraw();
            }

            if (is_key_down(key_num_plus)) {
                if (angle_increment < MAX_ANGLE) {
                    angle_increment += 1.0f;
                    redraw();
                }
            }

            if (is_key_down(key_num_minus)) {
                if (angle_increment > MIN_ANGLE) {
                    angle_increment -= 1.0f;
                    redraw();
                }
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
            mtw.translate(mid_pos);
            mtw.rotate(angle, 0.0f, 0.0f, 1.0f);
            mesh_box *box = new mesh_box(vec3(SEGMENT_WIDTH, SEGMENT_LENGTH, SEGMENT_WIDTH), mtw);

            scene_node *node = new scene_node();
            app_scene->add_child(node);
            app_scene->add_mesh_instance(new mesh_instance(node, box, drawing_material));
           
            

            return end_pos;
        }

        void create_geometry() {
            dynarray<char> axiom = t.get_axiom();
            vec3 pos = vec3(0.0f, 0.0f, 0.0f);
            float angle = 0.0f;
            current_level = 0;

            for (unsigned int i = 0; i < axiom.size(); ++i) {
                if (axiom[i] == '+') {
                    angle += angle_increment;
                } else if (axiom[i] == '-') {
                    angle -= angle_increment;
                } else if (axiom[i] == '[') {
                    ++current_level;
                    node n = node(pos, angle);
                    node_stack.push_back(n);
                } else if (axiom[i] == ']') {
                    node n = node_stack[node_stack.size() - 1];
                    --current_level;
                    node_stack.pop_back();
                    angle = n.get_angle();
                    pos = n.get_pos();
                } else if (axiom[i] == 'F') {
                    drawing_material = bark_material;
                    for (unsigned int j = i+1; j < axiom.size(); ++j) {
                        if (axiom[j] == ']') {
                            drawing_material = leaf_material;
                        } else if (axiom[j] == 'F') {
                            break;
                        }
                    }
                    pos = draw_segment(pos, angle);
                } else if (axiom[i] == 'A') {
                    pos = draw_segment(pos, angle);
                } else if (axiom[i] == 'B') {
                    pos = draw_segment(pos, angle);
                }
            }
        }

    };
}
