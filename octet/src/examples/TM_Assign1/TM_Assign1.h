////////////////////////////////////////////////////////////////////////////////
//
// (C) Andy Thomason 2012-2014
//
// Modular Framework for OpenGLES2 rendering on multiple platforms.
//

#include "plank.h"

namespace octet {
    /// Scene containing a box with octet.

    class bullet {
        mesh_instance *mi;
        int timer;
    public:
        bullet() {
            timer = 0;
        }

        bullet(mesh_instance *mi_) {
            mi = mi_;
            timer = 0;
        }

        mesh_instance& get_mesh_instance() {
            return *mi;
        }

        mesh_instance* getp_mesh_instance() {
            return mi;
        }

        int& get_timer() {
            return timer;
        }
    };

    class TM_Assign1 : public app {
    private:
        //constants
        const float PI = 3.14159;

        btDiscreteDynamicsWorld *dynamics_world;

        // scene for drawing box
        ref<visual_scene> app_scene;

        //camera & fps members
        mouse_look mouse_look_helper;
        ref<camera_instance> main_camera;

        helper_fps_controller fps_helper;
        ref<scene_node> player_node;

        // storing all bullets so we can do clean-up
        dynarray<bullet> bullets;

        // jukebox (plays sound when you hit it)
        int jukebox_index;
        int player_index;

        ALuint sound;
        unsigned int sound_source;
        unsigned int num_sound_sources = 32;
        ALuint sources[32];
        bool can_play_sound;

        int frame_count = 0;

        //terrain 
        struct terrain_mesh_source : mesh_terrain::geometry_source {
            mesh::vertex vertex(vec3_in bb_min, vec3_in uv_min, vec3_in uv_delta, vec3_in pos) {
                vec3 p = bb_min + pos;
                vec3 uv = uv_min + vec3((float)pos.x(), (float)pos.z(), 0) * uv_delta;
                return mesh::vertex(p, vec3(0, 1, 0), uv);
            }
        };
        terrain_mesh_source terrain_source;

    public:
        /// this is called when we construct the class before everything is initialised.
        TM_Assign1(int argc, char **argv) : app(argc, argv) {
        }

        /// this is called once OpenGL is initialized
        void app_init() {
            app_scene = new visual_scene();
            app_scene->create_default_camera_and_lights();
            dynamics_world = app_scene->get_bt_world();

            mouse_look_helper.init(this, 200.0f / 360, false);
            fps_helper.init(this);

            main_camera = app_scene->get_camera_instance(0);
            main_camera->get_node()->translate(vec3(0, 4, 0));
            main_camera->set_far_plane(10000);

            mat4t mat;
            mat.loadIdentity();

            mesh_instance *mi = app_scene->add_shape(
                mat,
                new mesh_terrain(vec3(100.0f, 0.5f, 100.0f), ivec3(100, 1, 100), terrain_source),
                new material(new image("assets/grass.jpg")),
                false, 0
                );
            btRigidBody *rb = mi->get_node()->get_rigid_body();

            float player_height = 1.8f;
            float player_radius = 0.25f;
            float player_mass = 90.0f;

            mat.loadIdentity();
            mat.translate(0.0f, player_height*6.0f, 50.0f);

            mesh_instance *mi2 = app_scene->add_shape(
                mat,
                new mesh_sphere(vec3(0), player_radius),
                new material(vec4(1, 0, 0, 1)),
                true, player_mass,
                new btCapsuleShape(0.25f, player_height)
                );
            player_node = mi2->get_node();
            player_index = player_node->get_rigid_body()->getUserIndex();

            mat.loadIdentity();
            mat.translate(vec3(30, 1, 0));
            mesh_instance *mi3 = app_scene->add_shape(mat, new mesh_box(vec3(2)), new material(vec4(0.2, 0.1, 0.5, 1)), false);
            jukebox_index = mi3->get_node()->get_rigid_body()->getUserIndex();

            create_bridge();
            create_springs();

            sound = resource_dict::get_sound_handle(AL_FORMAT_MONO16, "assets/invaderers/bang.wav");
            sound_source = 0;
            alGenSources(num_sound_sources, sources);
            can_play_sound = true;
        }

        void create_bridge() {
            mat4t mtw;
            mtw.loadIdentity();
            mtw.translate(vec3(0, 0.5f, 0));
            mesh_instance *b1 = app_scene->add_shape(mtw, new mesh_box(vec3(1, 1, 1)), new material(vec4(1, 0, 0, 1)), false);

            mtw.loadIdentity();
            mtw.translate(vec3(1.6f, 1.25f, 0.0f));
            mesh_instance *p1 = app_scene->add_shape(mtw, new mesh_box(vec3(0.5f, 0.25f, 1)), new material(vec4(0, 1, 0, 1)), true);

            mtw.loadIdentity();
            mtw.translate(vec3(2.7f, 1.25f, 0.0f));
            mesh_instance *p2 = app_scene->add_shape(mtw, new mesh_box(vec3(0.5f, 0.25f, 1)), new material(vec4(0, 1, 1, 1)), true);

            mtw.loadIdentity();
            mtw.translate(vec3(3.8f, 1.25f, 0.0f));
            mesh_instance *p3 = app_scene->add_shape(mtw, new mesh_box(vec3(0.5f, 0.25f, 1)), new material(vec4(0, 1, 0, 1)), true);

            mtw.loadIdentity();
            mtw.translate(vec3(4.9f, 1.25f, 0.0f));
            mesh_instance *p4 = app_scene->add_shape(mtw, new mesh_box(vec3(0.5f, 0.25f, 1)), new material(vec4(0, 1, 1, 1)), true);

            mtw.loadIdentity();
            mtw.translate(vec3(6.5f, 0.5f, 0.0f));
            mesh_instance *b2 = app_scene->add_shape(mtw, new mesh_box(vec3(1, 1, 1)), new material(vec4(1, 0, 0, 1)), false);

            // hinges

            btHingeConstraint *c1 = new btHingeConstraint(*(b1->get_node()->get_rigid_body()), *(p1->get_node()->get_rigid_body()),
                btVector3(0.5f, 0.5f, 0.0f), btVector3(-0.5f, 0.25f, 0.0f),
                btVector3(0, 0, 1), btVector3(0, 0, 1), false);
            c1->setLimit(-PI * 0.1f, PI* 0.1f);
            dynamics_world->addConstraint(c1);

            btHingeConstraint *c2 = new btHingeConstraint(*(p1->get_node()->get_rigid_body()), *(p2->get_node()->get_rigid_body()),
                btVector3(0.5f, 0.25f, 0.0f), btVector3(-0.5f, 0.25f, 0.0f),
                btVector3(0, 0, 1), btVector3(0, 0, 1), false);
            c2->setLimit(-PI * 0.1f, PI* 0.1f);
            dynamics_world->addConstraint(c2);

            btHingeConstraint *c3 = new btHingeConstraint(*(p2->get_node()->get_rigid_body()), *(p3->get_node()->get_rigid_body()),
                btVector3(0.5f, 0.25f, 0.0f), btVector3(-0.5f, 0.25f, 0.0f),
                btVector3(0, 0, 1), btVector3(0, 0, 1), false);
            c3->setLimit(-PI * 0.1f, PI* 0.1f);
            dynamics_world->addConstraint(c3);

            btHingeConstraint *c4 = new btHingeConstraint(*(p3->get_node()->get_rigid_body()), *(p4->get_node()->get_rigid_body()),
                btVector3(0.5f, 0.25f, 0.0f), btVector3(-0.5f, 0.25f, 0.0f),
                btVector3(0, 0, 1), btVector3(0, 0, 1), false);
            c4->setLimit(-PI * 0.1f, PI* 0.1f);
            dynamics_world->addConstraint(c4);

            btHingeConstraint *c5 = new btHingeConstraint(*(p4->get_node()->get_rigid_body()), *(b2->get_node()->get_rigid_body()),
                btVector3(0.5f, 0.25f, 0.0f), btVector3(-0.5f, 0.5f, 0.0f),
                btVector3(0, 0, 1), btVector3(0, 0, 1), false);
            c5->setLimit(-PI * 0.1f, PI* 0.1f);
            dynamics_world->addConstraint(c5);
        }

        void create_springs() {
            mat4t mtw;
            mtw.translate(-3, 10, 0);
            btRigidBody *rb1 = NULL;
            mesh_instance *mi1 = app_scene->add_shape(mtw, new mesh_box(vec3(1, 1, 1)), new material(vec4(1, 0, 0, 1)), false);
            rb1 = mi1->get_node()->get_rigid_body();

            mtw.loadIdentity();
            mtw.translate(-3, 8, 0);
            btRigidBody *rb2 = NULL;
            mesh_instance *mi2 = app_scene->add_shape(mtw, new mesh_box(vec3(1, 1, 1)), new material(vec4(0, 1, 0, 1)), true, 1.0f);
            rb2 = mi2->get_node()->get_rigid_body();

            btTransform frameInA, frameInB;
            frameInA = btTransform::getIdentity();
            frameInA.setOrigin(btVector3(btScalar(0.0f), btScalar(-0.5f), btScalar(0.0f)));
            frameInB = btTransform::getIdentity();
            frameInB.setOrigin(btVector3(btScalar(0.0f), btScalar(0.5f), btScalar(0.0f)));

            btGeneric6DofSpringConstraint *c1 = new btGeneric6DofSpringConstraint(*rb2, *rb1, frameInA, frameInB, true);
            c1->setLinearUpperLimit(btVector3(0.0f, 5.0f, 0.0f));
            c1->setLinearLowerLimit(btVector3(0.0f, -5.0f, 0.0f));

            c1->setAngularLowerLimit(btVector3(-1.5f, -1.5f, -1.5f));
            c1->setAngularUpperLimit(btVector3(1.5f, 1.5f, 1.5f));

            dynamics_world->addConstraint(c1, false);

            c1->setDbgDrawSize(btScalar(5.f));
            c1->enableSpring(0.0f, true);
            c1->setStiffness(0.0f, 10.0f);
            c1->setDamping(0.0f, 0.5f);
        }

        void shoot() {
            mat4t mtw;
            mtw.translate(main_camera->get_node()->get_position());
            bullet b = bullet(app_scene->add_shape(mtw, new mesh_sphere(vec3(1), 0.2f), new material(vec4(1, 0, 0.8f, 1)), true, 0.01f));
            vec3 fwd = -main_camera->get_node()->get_z();
            b.get_mesh_instance().get_node()->apply_central_force(fwd*30.0f);
            bullets.push_back(b);
        }

        void bullet_cleanup() {
            for (unsigned int i = 0; i < bullets.size(); ++i) {
                if (bullets[i].get_timer() > 150) {
                    app_scene->delete_mesh_instance(bullets[i].getp_mesh_instance());
                    bullets[i] = bullets[bullets.size() - 1];
                    bullets.resize(bullets.size() - 1);
                }
            }
        }

        ALuint get_sound_source() {
            sound_source = sound_source % num_sound_sources;
            sound_source++;
            return sources[sound_source];
        }

        void check_collisions() {
            int num_manifolds = dynamics_world->getDispatcher()->getNumManifolds();
            for (unsigned int i = 0; i < num_manifolds; ++i) {
                btPersistentManifold *manifold = dynamics_world->getDispatcher()->getManifoldByIndexInternal(i);
                int index0 = manifold->getBody0()->getUserIndex();
                int index1 = manifold->getBody1()->getUserIndex();

                if (index0 == player_index || index1 == player_index) {
                    if (index0 == jukebox_index || index1 == jukebox_index) {
                        if (can_play_sound) {
                            ALuint source = get_sound_source();
                            alSourcei(source, AL_BUFFER, sound);
                            alSourcePlay(source);
                            can_play_sound = false;
                        }
                    }
                }
            }
        }

        /// this is called to draw the world
        void draw_world(int x, int y, int w, int h) {

            int vx = 0, vy = 0;
            get_viewport_size(vx, vy);
            app_scene->begin_render(vx, vy);

            if (is_key_going_down(key_lmb)) {
                shoot();
            }

            bullet_cleanup();

            check_collisions();

            if (++frame_count > 100) {
                frame_count = 0;
                can_play_sound = true;
            }

            //update camera
            scene_node *camera_node = main_camera->get_node();
            mat4t &camera_to_world = camera_node->access_nodeToParent();
            mouse_look_helper.update(camera_to_world);

            fps_helper.update(player_node, camera_node);

            // update matrices. assume 30 fps.
            app_scene->update(1.0f / 30);

            // draw the scene
            app_scene->render((float)vx / vy);
        }
    };
}
