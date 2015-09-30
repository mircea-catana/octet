#ifndef PLANK
#define PLANK

#ifndef OCTET_BULLET
#define OCTET_BULLET 1
#endif
#include "..\..\octet.h"


namespace octet {
	class plank : public octet::resource {
	protected:
		scene_node *node;
		mat4t model_to_world;
		vec3 plank_size;
		mesh_box *plank_mesh;
		material *plank_material;
		

		btScalar plank_mass;
		btRigidBody *rigidbody;
		btMotionState *motion_state;

	public:
		plank() {
		}

		plank(mat4t mtw, vec3 size, material *mat, btScalar mass) {
			init(mtw, size, mat, mass);
		}

		~plank() {
		}

		void init(mat4t_in mtw, vec3 size, material *mat, btScalar mass = 1.0f) {
			model_to_world = mtw;
			plank_size = size;
			plank_material = mat;
			plank_mass = mass;
			btMatrix3x3 matrix(get_btMatrix3x3(model_to_world));
			btVector3 translation(get_btVector3(model_to_world[3].xyz()));
			motion_state = new btDefaultMotionState(btTransform(matrix, translation));

			btCollisionShape *shape = new btBoxShape(get_btVector3(size));
			btVector3 inertia;
			shape->calculateLocalInertia(mass, inertia);
			rigidbody = new btRigidBody(mass, motion_state, shape, inertia);
			rigidbody->setActivationState(DISABLE_DEACTIVATION);

			plank_mesh = new mesh_box(size);
			node = new scene_node(mtw, atom_);
		}

		btRigidBody* get_rigidbody() {
			return rigidbody;
		}

		scene_node* get_scene_node() {
			return node;
		}

		mesh_box* get_mesh() {
			return plank_mesh;
		}

		material* get_material() {
			return plank_material;
		}
	};
}

#endif