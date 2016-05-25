#include "Scene.h"
#include "..\resources\ResourceContainer.h"

namespace ds {

	Scene::Scene(const SceneDescriptor& descriptor) {
		_meshBuffer = res::getMeshBuffer(descriptor.meshBuffer);
		_camera = res::getCamera(descriptor.camera);
		_depthEnabled = descriptor.depthEnabled;
	}

	Scene::~Scene()	{
	}

	// ------------------------------------
	// add entity
	// ------------------------------------
	ID Scene::add(const char* meshName, const v3& position, DrawMode mode) {
		Mesh* m = res::getMesh(meshName);
		return add(m, position);
	}
	
	// ------------------------------------
	// add entity
	// ------------------------------------
	ID Scene::add(Mesh* mesh, const v3& position, DrawMode mode) {
		ID id = _entities.add();
		Entity& e = _entities.get(id);
		e.mesh = mesh;
		e.position = position;
		e.scale = v3(1, 1, 1);
		e.rotation = v3(0, 0, 0);
		e.timer = 0.0f;
		e.active = true;
		e.type = -1;
		e.world = matrix::mat4Transform(position);
		e.parent = INVALID_ID;
		e.value = 0;
		e.mode = mode;
		return id;
	}

	// ------------------------------------
	// attach
	// ------------------------------------
	void Scene::attach(ID child, ID parent)	{
		if (_entities.contains(child) && _entities.contains(parent)) {
			Entity& e = _entities.get(child);
			e.parent = parent;
		}
	}

	// ------------------------------------
	// draw
	// ------------------------------------
	void Scene::draw() {
		graphics::setCamera(_camera);
		if (_depthEnabled) {
			graphics::turnOnZBuffer();
		}
		else {
			graphics::turnOffZBuffer();
		}
		_meshBuffer->begin();
		for (EntityList::iterator it = _entities.begin(); it != _entities.end(); ++it) {
			const Entity& e = (*it);			
			if (e.active) {
				if (e.mode == DrawMode::IMMEDIATE) {
					_meshBuffer->flush();
					_meshBuffer->drawImmediate(e.mesh, e.world, e.scale, e.rotation, e.color);
				}
				else if (e.mode == DrawMode::TRANSFORM) {
					_meshBuffer->add(e.mesh, e.position, e.scale, e.rotation);
				}
			}
		}
		_meshBuffer->end();
	}

	// ------------------------------------
	// find entities by type
	// ------------------------------------
	int Scene::find(int type, ID* ids, int max) {
		int cnt = 0;
		for (EntityList::iterator it = _entities.begin(); it != _entities.end(); ++it) {
			const Entity& e = (*it);
			if (e.type == type && cnt < max) {
				ids[cnt++] = e.id;
			}
		}
		return cnt;
	}

	// ------------------------------------
	// transform
	// ------------------------------------
	void Scene::transform() {
		for (EntityList::iterator it = _entities.begin(); it != _entities.end(); ++it) {
			Entity& e = (*it);
			if (e.parent == INVALID_ID) {
				e.world = matrix::mat4Transform(e.position);
			}
			else {
				const Entity& parent = _entities.get(e.parent);
				e.world = matrix::mat4Transform(e.position) * parent.world;
			}
		}
	}

	// ------------------------------------
	// get entity
	// ------------------------------------
	Entity& Scene::get(ID id) {
		return _entities.get(id);
	}

	// ------------------------------------
	// get entity
	// ------------------------------------
	const Entity& Scene::get(ID id) const {
		return _entities.get(id);
	}

	// ------------------------------------
	// remove entity
	// ------------------------------------
	void Scene::remove(ID id) {
		// check if this is a parent to anyone and then remove this as well?
		_entities.remove(id);
	}

}