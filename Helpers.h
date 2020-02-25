#ifndef HELPERS_H
#define HELPERS_H

#include <vector>

#include "intensities.h"
#include "zone.h"

namespace Helpers
{
	void deleteZones(std::vector<Zone*> **zonesPtr);

	void clearEvaluatedIntensities(vector<EvaluatedIntensity*> **evalIntensityListPtr);
};

#endif // HELPERS_H
