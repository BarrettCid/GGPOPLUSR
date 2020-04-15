#pragma once
namespace trainingmodehelper {
	enum PositionState {
		center = 0,
		leftCorner = 1,
		rightCorner = 2,
		defaultSides = 3,
		swappedSides = 4
	};
	extern int positionState;
	extern int swapState;
}
