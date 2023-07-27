#pragma once
#include "PolyLine.h"
#include "Strategys.h"

class PolyGon : public PolyLine
{
public:
	PolyGon() = default;

public:
	virtual void draw(WDraw& wdraw) override;
	virtual Rectangle box() override;
};
