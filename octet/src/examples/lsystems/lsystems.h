#include "tree.h"

namespace octet {
    class lsystems : public app {

        class node {
            vec3 pos;
            float z_angle;
            float y_angle;
            float segment_width;
        public:
            node() {
                pos = vec3(0.0f, 0.0f, 0.0f);
                z_angle = 0.0f;
                y_angle = 0.0f;
                segment_width = 0.0f;
            }

            node(vec3 pos_, float z_angle_, float segment_width_, float y_angle_ = 0.0f) {
                pos = pos_;
                z_angle = z_angle_;
                y_angle = y_angle_;
                segment_width = segment_width_;
            }

            vec3& get_pos() {
                return pos;
            }

            float& get_z_angle() {
                return z_angle;
            }

            float& get_y_angle() {
                return y_angle;
            }

            float& get_segment_width() {
                return segment_width;
            }
        };

        const float PI = 3.14159265f;
        float SEGMENT_LENGTH = 5.0f;
        float SEGMENT_WIDTH = 2.6f;

        ref<visual_scene> app_scene;

        tree t;
        float tree_max_y = 0.0f;
        dynarray<node> node_stack;
        
        material *red_material;
        material *bark_material;
        material *leaf_material;
        material *drawing_material;
        int current_level = 0;

        const int MIN_FILE_NO = 0;
        const int MAX_FILE_NO = 8;
        int current_file_no = 0;

        const float MIN_Z_ANGLE = 20.0f;
        const float MAX_Z_ANGLE = 45.0f;
        float z_angle_increment = 25.0f;
        float y_angle_increment = 20.0f;

        mouse_look mouse_look_helper;
        ref<camera_instance> camera;

        bool rotateTree = false;
        bool is3D = false;

    public:
        lsystems(int argc, char **argv) : app(argc, argv) {
        }

        void app_init() {
            t.read_file(current_file_no);

            mouse_look_helper.init(this, 200.0f / 360.0f, false);

            app_scene = new visual_scene();
            app_scene->create_default_camera_and_lights();
            camera = app_scene->get_camera_instance(0);
            camera->get_node()->translate(vec3(0.0f, 0.0f, 1.0f));

            image *bark_img = new image("assets/lsystems/bark.gif");
            bark_material = new material(bark_img);
            leaf_material = new material(vec4(0.3f, 0.5f, 0.1f, 1.0f));
            red_material = new material(vec4(0.8f, 0.2f, 0.1f, 1.0f));
            drawing_material = bark_material;

            create_geometry();
        }

        void draw_world(int x, int y, int w, int h) {
            handle_input();
            rotate_tree();

            app_scene->begin_render(w, h);

            mat4t &camera_to_world = camera->get_node()->access_nodeToParent();
            mouse_look_helper.update(camera_to_world);

            app_scene->update(1.0f / 30.0f);

            app_scene->render((float)w / h);
        }

        void rotate_tree() {
            if (!rotateTree) {
                return;
            }

            for (unsigned int i = 0; i < app_scene->get_num_mesh_instances(); ++i) {
                app_scene->get_mesh_instance(i)->get_node()->rotate(1.0f, vec3(0.0f, 1.0f, 0.0f));
            }
        }

        void redraw() {
            SEGMENT_LENGTH = 5.0f;
            SEGMENT_WIDTH = 2.6f;

            app_scene = new visual_scene();
            app_scene->create_default_camera_and_lights();
            camera = app_scene->get_camera_instance(0);

            camera->set_far_plane(500.0f);
            

            tree_max_y = 0.0f;
            create_geometry();

            float dy = (tree_max_y + SEGMENT_LENGTH) / 2.0f - camera->get_node()->get_position().y();
            float dz = (tree_max_y + SEGMENT_LENGTH) / 2.0f - camera->get_node()->get_position().z();
            camera->get_node()->translate(vec3(0.0f, dy, dz));
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
                    z_angle_increment = 90.0f;
                } else if (current_file_no == 7) {
                    z_angle_increment = 60.0f;
                } else {
                    z_angle_increment = 25.0f;
                }

                redraw();
            }

            if (is_key_going_down(key_left)) {
                if (--current_file_no < 0) {
                    current_file_no = MAX_FILE_NO - 1;
                }
                t.read_file(current_file_no);

                if (current_file_no == 6) {
                    z_angle_increment = 90.0f;
                } else if (current_file_no == 7) {
                    z_angle_increment = 60.0f;
                } else {
                    z_angle_increment = 25.0f;
                }

                redraw();
            }

            if (is_key_down(key_num_plus)) {
                if (current_file_no >= 6) {
                    return;
                }

                if (z_angle_increment < MAX_Z_ANGLE) {
                    z_angle_increment += 1.0f;
                    redraw();
                }
            }

            if (is_key_down(key_num_minus)) {
                if (current_file_no >= 6) {
                    return;
                }

                if (z_angle_increment > MIN_Z_ANGLE) {
                    z_angle_increment -= 1.0f;
                    redraw();
                }
            }

            if (is_key_going_down(key_r)) {
                rotateTree = !rotateTree;
            }

            if (is_key_going_down(key_t)) {
                is3D = !is3D;
                redraw();
            }

            // movement controls
            if (is_key_down(key_w)) {
                camera->get_node()->translate(vec3(0.0f, 0.0f, -1.0f));
            }

            if (is_key_down(key_s)) {
                camera->get_node()->translate(vec3(0.0f, 0.0f, 1.0f));
            }

            if (is_key_down(key_a)) {
                camera->get_node()->translate(vec3(-1.0f, 0.0f, 0.0f));
            }

            if (is_key_down(key_d)) {
                camera->get_node()->translate(vec3(1.0f, 0.0f, 0.0f));
            }
        }

        vec3 draw_segment(vec3 start_pos, float z_angle, float y_angle = 0.0f) {
            vec3 mid_pos;
            vec3 end_pos;

            mid_pos.x() = start_pos.x() + SEGMENT_LENGTH * cos((z_angle + 90.0f) * PI / 180.0f);
            mid_pos.y() = start_pos.y() + SEGMENT_LENGTH * sin((z_angle + 90.0f) * PI / 180.0f);
            end_pos.x() = start_pos.x() + SEGMENT_LENGTH * 2.0f * cos((90.0f + z_angle) * PI / 180.0f);
            end_pos.y() = start_pos.y() + SEGMENT_LENGTH * 2.0f * sin((90.0f + z_angle) * PI / 180.0f);

            if (tree_max_y < end_pos.y()) {
                tree_max_y = end_pos.y() + SEGMENT_LENGTH;
            }

            mat4t mtw1, mtw2;
            
            mtw1.loadIdentity();
            mtw1.rotate(y_angle, 0.0f, 1.0f, 0.0f);

            mtw2.loadIdentity();
            mtw2.translate(mid_pos);
            mtw2.rotate(z_angle, 0.0f, 0.0f, 1.0f);

            mtw2 = mtw2*mtw1;

            if (current_file_no < 6) {
                mat4t mtw;
                mtw.loadIdentity();
                mtw.rotate(90.0f, 1.0f, 0.0f, 0.0f);
                zcylinder cyl = zcylinder(vec3(0), SEGMENT_WIDTH, SEGMENT_LENGTH);
                mesh_cylinder *cylinder = new mesh_cylinder(cyl, mtw*mtw2);

                scene_node *node = new scene_node();
                app_scene->add_child(node);
                app_scene->add_mesh_instance(new mesh_instance(node, cylinder, drawing_material));
            } else {
                mesh_box *box = new mesh_box(vec3(SEGMENT_WIDTH, SEGMENT_LENGTH, SEGMENT_WIDTH), mtw2);
                scene_node *node = new scene_node();
                app_scene->add_child(node);
                app_scene->add_mesh_instance(new mesh_instance(node, box, red_material));
            }

            return end_pos;
        }

        void create_geometry() {
            vec3 pos = vec3(0.0f, 0.0f, 0.0f);
            float z_angle = 0.0f;
            float y_angle = 0.0f;
            current_level = 0;

            dynarray<char> axiom = t.get_axiom();

            int thinning_steps = 8;
            int num_segments = t.segments_in_tree();
            int num_segments_step = num_segments / thinning_steps;
            int segment_count = 0;

            for (unsigned int i = 0; i < axiom.size(); ++i) {
                if (axiom[i] == '+') {
                    z_angle += z_angle_increment;
                    y_angle += y_angle_increment;
                } else if (axiom[i] == '-') {
                    z_angle -= z_angle_increment;
                    y_angle -= y_angle_increment;
                } else if (axiom[i] == '[') {
                    ++current_level;
                    node n = node(pos, z_angle, SEGMENT_WIDTH, y_angle);
                    node_stack.push_back(n);
                } else if (axiom[i] == ']') {
                    node n = node_stack[node_stack.size() - 1];
                    --current_level;
                    node_stack.pop_back();
                    z_angle = n.get_z_angle();
                    y_angle = n.get_y_angle();
                    pos = n.get_pos();
                } else if (axiom[i] == 'F') {
                    ++segment_count;
                    if (segment_count >= num_segments_step) {
                        SEGMENT_WIDTH /= 1.1f;
                        segment_count = 0;
                    }

                    drawing_material = bark_material;
                    for (unsigned int j = i+1; j < axiom.size(); ++j) {
                        if (axiom[j] == ']') {
                            drawing_material = leaf_material;
                        } else if (axiom[j] == 'F') {
                            break;
                        }
                    }

                    if (is3D) {
                        pos = draw_segment(pos, z_angle, y_angle);
                    } else {
                        pos = draw_segment(pos, z_angle);
                    }
                    
                } else if (axiom[i] == 'A') {
                    pos = draw_segment(pos, z_angle);
                } else if (axiom[i] == 'B') {
                    pos = draw_segment(pos, z_angle);
                }
            }
        }

    };
}
