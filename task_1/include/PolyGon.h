#pragma once
#include "PolyLine.h"
#include "Strategys.h"

class PolyGon : public PolyLine
{
public:
	PolyGon() = default;

public:
	virtual void draw(WDraw& wdraw) const override;
	virtual BoundyBox getBoundyBox() const override;
	virtual double lengthSeg() const override;
};

