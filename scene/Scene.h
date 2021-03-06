#pragma once
#include "core\lib\DataArray.h"
#include <Vector.h>
#include "core\Common.h"
#include "..\renderer\MeshBuffer.h"
#include "..\renderer\Camera.h"
#include "EntityArray.h"
#include <core\world\ActionEventBuffer.h>
#include "core\math\tweening.h"
#include "..\particles\ParticleSystem.h"
#include "..\postprocess\PostProcess.h"

namespace ds {

	//const int MAX_ACTIONS = 32;

	struct StaticMesh {
		ID id;
		uint32_t index;
		uint32_t size;
		AABBox boundingBox;
	};

	// ----------------------------------------
	// Basic scene
	// ----------------------------------------
	class Scene {

	public:
		Scene(const SceneDescriptor& descriptor);
		virtual ~Scene();
		ID add(const char* meshName, const v3& position, RID material, DrawMode mode = IMMEDIATE);
		ID add(Mesh* mesh, const v3& position, RID material, DrawMode mode = IMMEDIATE);
		ID addStatic(Mesh* mesh, const v3& position, RID material);
		void attach(ID child, ID parent);
		void remove(ID id);
		virtual void draw();
		int find(int type, ID* ids, int max);
		ID intersects(const Ray& ray);
		uint32_t numEntities() const {
			return _data.num;
		}
		virtual void tick(float dt);
		void rotate(ID id, const v3& r);
		void activate(ID id);
		void deactivate(ID id);
		void setColor(ID id, const Color& clr);
		void clear();
		bool isActive(ID id) const;
		void setPosition(ID id, const v3& p);

		// actions
		bool hasEvents() const {
			return _eventBuffer.events.size() > 0;
		}
		uint32_t numEvents() const {
			return _eventBuffer.events.size();
		}

		const ActionEvent& getEvent(uint32_t idx) {
			return _eventBuffer.events[idx];
		}


		void save(const ReportWriter& writer);
		const bool isActive() const {
			return _active;
		}
		void setActive(bool active) {
			_active = active;
		}
	protected:
		EntityArray _data;
	private:
		bool _active;
		void updateWorld(int idx);

		SceneDescriptor _descriptor;
		RID _currentMaterial;
		
		MeshBuffer* _meshBuffer;
		Camera* _camera;
		Array<PNTCVertex> _staticVertices;
		Array<StaticMesh> _staticMeshes;
		
		ActionEventBuffer _eventBuffer;
		bool _depthEnabled;
	};

	// ----------------------------------------
	// 2D scene
	// ----------------------------------------
	struct ParticleSystemMapping {

		ID id;
		ParticleSystem* system;
		ID instanceID;
	};

	class Scene2D : public Scene {

	public:
		Scene2D(const SceneDescriptor& descriptor) : Scene(descriptor), _renderTarget(INVALID_RID), _rtActive(false) {}
		~Scene2D() {}
		ID add(const v2& pos, const Texture& t, RID material);
		
		ID addParticleSystem(ID systemID);
		ID startParticleSystem(ID id, const v2& pos);
		void stopParticleSystem(ID id);

		void addPostProcess(PostProcess* pp);
		virtual void tick(float dt);
		void draw();
		void setTexture(ID id, const Texture& t);
		void useRenderTarget(const char* name);
		void activateRenderTarget();
		void deactivateRenderTarget();
	private:
		Array<PostProcess*> _postProcesses;
		DataArray<ParticleSystemMapping> _particleSystems;
		OrthoCamera* _camera;
		RID _renderTarget;
		bool _rtActive;
	};

	// ----------------------------------------
	// 3D scene
	// ----------------------------------------
	class Scene3D : public Scene {

	public:
		Scene3D(const SceneDescriptor& descriptor) : Scene(descriptor) {}
		~Scene3D() {}
	private:
		FPSCamera* _camera;
	};
}
