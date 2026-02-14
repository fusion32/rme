#ifndef RME_PROBLEMS_WINDOW_H_
#define RME_PROBLEMS_WINDOW_H_ 1

#include "map.h"
#include "position.h"
#include <wx/listctrl.h>

enum ProblemSeverity {
	SEVERITY_NOTICE  = 0,
	SEVERITY_WARNING = 1,
	SEVERITY_ERROR   = 2,
};

enum ProblemSourceType {
	SOURCE_NONE         = 0,
	SOURCE_OBJECT_TYPE  = 1,
	SOURCE_MONSTER_TYPE = 2,
	SOURCE_POSITION     = 3,
};

struct ProblemSource {
	ProblemSourceType type = SOURCE_NONE;
	union{
		int typeId;
		int raceId;
		Position position;
	};

	static ProblemSource FromObjectType(int typeId){
		ProblemSource source = {};
		source.type = SOURCE_OBJECT_TYPE;
		source.typeId = typeId;
		return source;
	}

	static ProblemSource FromMonsterType(int raceId){
		ProblemSource source = {};
		source.type = SOURCE_MONSTER_TYPE;
		source.raceId = raceId;
		return source;
	}

	static ProblemSource FromPosition(Position position){
		ProblemSource source = {};
		source.type = SOURCE_POSITION;
		source.position = position;
		return source;
	}

	static ProblemSource FromPosition(int x, int y, int z){
		ProblemSource source = {};
		source.type = SOURCE_POSITION;
		source.position.x = x;
		source.position.x = y;
		source.position.x = z;
		return source;
	}

	static ProblemSource FromSector(int sectorX, int sectorY, int sectorZ){
		ProblemSource source = {};
		source.type = SOURCE_POSITION;
		source.position.x = sectorX * MAP_SECTOR_SIZE + MAP_SECTOR_SIZE / 2;
		source.position.y = sectorY * MAP_SECTOR_SIZE + MAP_SECTOR_SIZE / 2;
		source.position.z = sectorZ;
		return source;
	}
};

struct Problem {
	ProblemSeverity severity;
	ProblemSource source;
	wxString message;
};

class ProblemsWindow: public wxListCtrl {
public:
	ProblemsWindow(wxWindow *parent);
	~ProblemsWindow(void) override;
	wxString OnGetItemText(long item, long column) const override;
	void OnItemSelected(wxListEvent &event);
	void Insert(ProblemSeverity severity, ProblemSource source, wxString message);

private:
	std::vector<Problem> problems = {};
};

#endif //RME_PROBLEMS_WINDOW_H_
