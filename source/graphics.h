//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_GRAPHICS_H_
#define RME_GRAPHICS_H_

#include "common.h"
#include "creature.h"
#include <deque>

#include <wx/artprov.h>

enum SpriteSize {
	SPRITE_SIZE_16x16,
	//SPRITE_SIZE_24x24,
	SPRITE_SIZE_32x32,
	SPRITE_SIZE_COUNT
};

enum AnimationDirection {
	ANIMATION_FORWARD = 0,
	ANIMATION_BACKWARD = 1
};

enum ItemAnimationDuration {
	ITEM_FRAME_DURATION = 500
};

enum {
	EDITOR_SPRITE_SELECTION_MARKER = -1000,
	EDITOR_SPRITE_BRUSH_CD_1x1,
	EDITOR_SPRITE_BRUSH_CD_3x3,
	EDITOR_SPRITE_BRUSH_CD_5x5,
	EDITOR_SPRITE_BRUSH_CD_7x7,
	EDITOR_SPRITE_BRUSH_CD_9x9,
	EDITOR_SPRITE_BRUSH_CD_15x15,
	EDITOR_SPRITE_BRUSH_CD_19x19,
	EDITOR_SPRITE_BRUSH_SD_1x1,
	EDITOR_SPRITE_BRUSH_SD_3x3,
	EDITOR_SPRITE_BRUSH_SD_5x5,
	EDITOR_SPRITE_BRUSH_SD_7x7,
	EDITOR_SPRITE_BRUSH_SD_9x9,
	EDITOR_SPRITE_BRUSH_SD_15x15,
	EDITOR_SPRITE_BRUSH_SD_19x19,
	EDITOR_SPRITE_OPTIONAL_BORDER_TOOL,
	EDITOR_SPRITE_ERASER,
	EDITOR_SPRITE_PZ_TOOL,
	EDITOR_SPRITE_PVPZ_TOOL,
	EDITOR_SPRITE_NOLOG_TOOL,
	EDITOR_SPRITE_NOPVP_TOOL,
	EDITOR_SPRITE_DOOR_NORMAL,
	EDITOR_SPRITE_DOOR_LOCKED,
	EDITOR_SPRITE_DOOR_MAGIC,
	EDITOR_SPRITE_DOOR_QUEST,
	EDITOR_SPRITE_WINDOW_NORMAL,
	EDITOR_SPRITE_WINDOW_HATCH,
	EDITOR_SPRITE_SELECTION_GEM,
	EDITOR_SPRITE_DRAWING_GEM,

	EDITOR_SPRITE_SPAWNS,
	EDITOR_SPRITE_HOUSE_EXIT,
	EDITOR_SPRITE_PICKUPABLE_ITEM,
	EDITOR_SPRITE_MOVEABLE_ITEM,
	EDITOR_SPRITE_PICKUPABLE_MOVEABLE_ITEM,
};

class MapCanvas;
class GraphicManager;
class FileReadHandle;
class Animator;

struct SpriteLight {
	uint8_t intensity = 0;
	uint8_t color = 0;
};

class Sprite
{
public:
	Sprite() {}
	Sprite(const Sprite&) = delete;

	virtual ~Sprite() {}
	virtual void DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width = -1, int height = -1) = 0;
	virtual void unloadDC() = 0;
};

class EditorSprite : public Sprite
{
public:
	EditorSprite(wxBitmap* b16x16, wxBitmap* b32x32);

	~EditorSprite() override;
	void DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width = -1, int height = -1) override;
	void unloadDC() override;

protected:
	wxBitmap* bm[SPRITE_SIZE_COUNT];
};

class GameSprite : public Sprite
{
public:
	GameSprite();
	~GameSprite() override;

	int getIndex(int width, int height, int layer, int pattern_x, int pattern_y, int pattern_z, int frame) const;
	GLuint getHardwareID(int _x, int _y, int _layer, int _pattern_x, int _pattern_y, int _pattern_z, int _frame);
	GLuint getHardwareID(int _x, int _y, int _dir, int _addon, int _pattern_z, const Outfit& _outfit, int _frame);
	void DrawTo(wxDC* dc, SpriteSize sz, int start_x, int start_y, int width = -1, int height = -1) override;
	void DrawTo(wxDC* context, const wxRect& rect, const Outfit& outfit);

	void unloadDC() override;

	void clean(time_t time);

	uint16_t getDrawHeight() const noexcept { return draw_height; }
	const wxPoint& getDrawOffset() const noexcept { return draw_offset; }
	uint8_t getMiniMapColor() const noexcept { return minimap_color; }

	bool hasLight() const noexcept { return has_light; }
	const SpriteLight& getLight() const noexcept { return light; }

	static GameSprite* createFromBitmap(const wxArtID& bitmapId);

protected:
	class Image;
	class NormalImage;
	class TemplateImage;

	wxMemoryDC* getDC(SpriteSize size);
	wxMemoryDC* getDC(const Outfit& outfit);
	TemplateImage* getTemplateImage(int sprite_index, const Outfit& outfit);

	class Image {
	public:
		Image();
		virtual ~Image();

		bool isGLLoaded;
		time_t lastaccess;

		void visit();
		virtual void clean(time_t time);

		virtual GLuint getHardwareID() = 0;
		virtual uint8_t* getRGBData() = 0;
		virtual uint8_t* getRGBAData() = 0;

	protected:
		virtual void createGLTexture(GLuint textureId);
		virtual void unloadGLTexture(GLuint textureId);
	};

	class NormalImage : public Image {
	public:
		NormalImage();
		~NormalImage() override;

		// We use the sprite id as GL texture id
		uint32_t id;

		// This contains the pixel data
		uint16_t size;
		uint8_t* dump;

		void clean(time_t time) override;

		GLuint getHardwareID() override;
		uint8_t* getRGBData() override;
		uint8_t* getRGBAData() override;

	protected:
		void createGLTexture(GLuint textureId = 0) override;
		void unloadGLTexture(GLuint textureId = 0) override;
	};

	class EditorImage : public NormalImage {
	public:
		EditorImage(const wxArtID& bitmapId);
	protected:
		void createGLTexture(GLuint textureId) override;
		void unloadGLTexture(GLuint textureId) override;
	private:
		wxArtID bitmapId;
	};

	class TemplateImage : public Image {
	public:
		TemplateImage(GameSprite* parent, int v, const Outfit& outfit);
		~TemplateImage() override;

		GLuint getHardwareID() override;
		uint8_t* getRGBData() override;
		uint8_t* getRGBAData() override;

		GLuint gl_tid;
		GameSprite* parent;
		int sprite_index;
		uint8_t lookHead;
		uint8_t lookBody;
		uint8_t lookLegs;
		uint8_t lookFeet;
	protected:
		void colorizePixel(uint8_t color, uint8_t &r, uint8_t &b, uint8_t &g);

		void createGLTexture(GLuint ignored = 0) override;
		void unloadGLTexture(GLuint ignored = 0) override;
	};

	uint32_t id;
	wxMemoryDC* dc[SPRITE_SIZE_COUNT];

public:
	// GameSprite info
	uint8_t height;
	uint8_t width;
	uint8_t layers;
	uint8_t pattern_x;
	uint8_t pattern_y;
	uint8_t pattern_z;
	uint8_t frames;
	int numsprites;
	Animator* animator;

	uint16_t draw_height;
	wxPoint draw_offset;
	uint16_t minimap_color;

	bool has_light = false;
	SpriteLight light;

	std::vector<NormalImage*> spriteList;
	std::list<TemplateImage*> instanced_templates; // Templates that use this sprite

	friend class GraphicManager;
};

struct FrameDuration
{
	int min;
	int max;

	FrameDuration(int min, int max) : min(min), max(max)
	{
		ASSERT(min <= max);
	}

	int getDuration() const
	{
		if(min == max)
			return min;
		return uniform_random(min, max);
	};

	void setValues(int min, int max)
	{
		ASSERT(min <= max);
		this->min = min;
		this->max = max;
	}
};

class Animator
{
public:
	Animator(int frames, int start_frame, int loop_count, bool async);
	~Animator();

	int getStartFrame() const;

	FrameDuration* getFrameDuration(int frame);

	int getFrame();
	void setFrame(int frame);

	void reset();

private:
	int getDuration(int frame) const;
	int getPingPongFrame();
	int getLoopFrame();
	void calculateSynchronous();

	int frame_count;
	int start_frame;
	int loop_count;
	bool async;
	std::vector<FrameDuration*> durations;
	int current_frame;
	int current_loop;
	int current_duration;
	int total_duration;
	AnimationDirection direction;
	long last_time;
	bool is_complete;
};

class GraphicManager
{
public:
	GraphicManager();
	~GraphicManager();

	void clear();
	void cleanSoftwareSprites();

	Sprite* getSprite(int id);
	GameSprite* getCreatureSprite(int id);
	GameSprite* getEditorSprite(int id);

	long getElapsedTime() const { return (animation_timer->TimeInMicro() / 1000).ToLong(); }

	int getItemCount() const { return itemCount; }
	int getItemSpriteMinID() const { return itemBaseId; }
	int getItemSpriteMaxID() const { return itemBaseId + itemCount - 1; }

	int getCreatureCount() const { return creatureCount; }
	int getCreatureSpriteMinID() const { return creatureBaseId; }
	int getCreatureSpriteMaxID() const { return creatureBaseId + creatureCount - 1; }

	// Get an unused texture id (this is acquired by simply increasing a value starting from 0x10000000)
	GLuint getFreeTextureID();

	// This is part of the binary
	bool loadEditorSprites();
	// This fills the item / creature adress space
	bool loadSpriteMetadata(const wxString &projectDir);
	bool loadSpriteMetadataFlags(FileReadHandle& file, GameSprite* sType);
	bool loadSpriteData(const wxString &projectDir);

	// Cleans old & unused textures according to config settings
	void garbageCollection();
	void addSpriteToCleanup(GameSprite* spr);

	bool hasTransparency() const { return false; }
	bool isUnloaded() const;

private:
	bool unloaded;
	// This is used if memcaching is NOT on
	std::string spritefile;
	bool loadSpriteDump(uint8_t*& target, uint16_t& size, int sprite_id);

	// TODO(fusion): Both Sprite and Image pointers are "polymorphic" so it's
	// unlikely that we'd be able to turn these into a non-pointer vector without
	// another large change, which probably won't happen soon (or at all).
	std::unordered_map<int, Sprite*> sprite_space;
	std::unordered_map<int, GameSprite::Image*> image_space;
	std::deque<GameSprite*> cleanup_list;

	int itemBaseId;
	int itemCount;
	int creatureBaseId;
	int creatureCount;

	int loaded_textures;
	int lastclean;

	wxStopWatch* animation_timer;

	friend class GameSprite::Image;
	friend class GameSprite::NormalImage;
	friend class GameSprite::EditorImage;
	friend class GameSprite::TemplateImage;
};

#endif
