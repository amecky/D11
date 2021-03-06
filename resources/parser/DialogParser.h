#pragma once
#include "ResourceParser.h"
/*
dialog {
	name : "MainMenu"
	file : "MainMenu"
	font : "nullshock"
}
*/
namespace ds {

	namespace res {

		class DialogParser : public ResourceParser {

		public:
			DialogParser(ResourceContext* ctx) : ResourceParser(ctx) {}
			RID parse(JSONReader& reader, int childIndex);
		private:
			RID createDialog(const char* name, const GUIDialogDescriptor& descriptor);
		};

	}

}