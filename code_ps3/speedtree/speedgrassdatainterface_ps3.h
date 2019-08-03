#ifndef SPEEDGRASSDATAINTERFACE_PS3_H
#define SPEEDGRASSDATAINTERFACE_PS3_H

struct ISpeedGrassData
{
	virtual float	GetLODCutoff() = 0;
	virtual float	GetViewTransactionLength() = 0;
	virtual float	GetWindPeriod() = 0;
	virtual float	GetWindSpeed() = 0;

	virtual	~ISpeedGrassData() {}
};

#endif
