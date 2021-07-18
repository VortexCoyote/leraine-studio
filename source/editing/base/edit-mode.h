#pragma once

#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>

#include "../../global/global-types.h"
#include "../../global/global-functions.h"
#include "../edit-flags.h"

class EditMode
{
public:

	virtual bool OnMouseLeftButtonClicked(const bool InIsShiftDown);
	virtual bool OnMouseLeftButtonReleased();

	virtual bool OnMouseRightButtonClicked(const bool InIsShiftDown);
	virtual bool OnMouseRightButtonReleased();

	virtual bool OnMouseDrag();

	virtual bool OnCopy();
	virtual bool OnPaste();
	virtual bool OnMirror();
	virtual bool OnDelete();
	virtual bool OnSelectAll();

	virtual void OnReset();
	virtual void SubmitToRenderGraph(TimefieldRenderGraph& InOutTimefieldRenderGraph, const Time InTimeBegin, const Time InTimeEnd);
	virtual void Tick();

public:

	static void SetChart(Chart* const InOutChart);
	static void SetCursorData(const Cursor& InCursor);

	static EditFlags static_Flags;
	
protected:

	static Chart* static_Chart;
	static Cursor static_Cursor;
};


template <class ... Args>
struct EditModeCollection //yeah I don't know myself
{
	EditModeCollection()
	{
		_InstancedEditModes.reserve(10);
		(PushEditMode<Args>(), ...);
	}

	~EditModeCollection()
	{
		for (auto* editMode : _InstancedEditModes)
			delete editMode;
	}

	EditMode* operator [] (const int InIndex) 
	{
		return _InstancedEditModes[InIndex];
	}

	template<class T>
	T* Get()
	{
		return (T*)_InstanceLookup[typeid(T).hash_code()];
	}

	template<class T>
	size_t GetIndex()
	{
		auto* instance = Get<T>();
		return find(_InstancedEditModes.begin(), _InstancedEditModes.end(), instance) - _InstancedEditModes.begin();
	}

private:

	template<class T>
	void PushEditMode()
	{
		auto instance = new T();

		_InstancedEditModes.push_back(instance);
		_InstanceLookup[typeid(T).hash_code()] = instance;
	}

	std::vector<EditMode*> _InstancedEditModes;
	std::map<size_t, EditMode*> _InstanceLookup;
};