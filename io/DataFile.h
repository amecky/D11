#pragma once
#include "..\io\json.h"
#include "..\utils\Profiler.h"

namespace ds {

	class DataFile {

	public:
		DataFile() {}
		virtual ~DataFile() {}
		virtual bool saveData(JSONWriter& writer) = 0;
		virtual bool loadData(const JSONReader& loader) = 0;
		virtual const char* getFileName() const = 0;
		bool save();		
		bool load();
	};

	class AssetFile {

	public:
		AssetFile(const char* name) : _loaded(false) {
			sprintf_s(_name, 64, "%s", name);
		}
		virtual ~AssetFile() {}
		virtual bool loadData(const JSONReader& loader) = 0;
		virtual bool reloadData(const JSONReader& loader) = 0;
		bool load();
		const char* getName() const {
			return _name;
		}
	private:
		bool _loaded;
		char _name[64];
	};

}