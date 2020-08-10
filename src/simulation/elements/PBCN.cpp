#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);
bool Element_PCLN_ctypeDraw(CTYPEDRAW_FUNC_ARGS);

void Element::Element_PBCN()
{
	Identifier = "DEFAULT_PT_PBCN";
	Name = "PBCN";
	Colour = PIXPACK(0x3B1D0A);
	MenuVisible = 1;
	MenuSection = SC_POWERED;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.97f;
	Loss = 0.50f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 12;

	Weight = 100;

	HeatConduct = 251;
	Description = "Powered breakable clone.";

	Properties = TYPE_SOLID | PROP_NOCTYPEDRAW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

	Update = &update;
	Graphics = &graphics;
	CtypeDraw = &Element_PCLN_ctypeDraw;
}

constexpr float ADVECTION = 0.1f;

static int update(UPDATE_FUNC_ARGS)
{
	int r, rx, ry, rt;
	if (!parts[i].tmp2 && sim->pv[y/CELL][x/CELL]>4.0f)
		parts[i].tmp2 = RNG::Ref().between(80, 119);
	if (parts[i].tmp2)
	{
		parts[i].vx += ADVECTION*sim->vx[y/CELL][x/CELL];
		parts[i].vy += ADVECTION*sim->vy[y/CELL][x/CELL];
		parts[i].tmp2--;
		if(!parts[i].tmp2){
			sim->kill_part(i);
			return 1;
		}
	}
	if (parts[i].ctype<=0 || parts[i].ctype>=PT_NUM || !sim->elements[parts[i].ctype].Enabled || (parts[i].ctype==PT_LIFE && (parts[i].tmp<0 || parts[i].tmp>=NGOL)))
		for (rx=-1; rx<2; rx++)
			for (ry=-1; ry<2; ry++)
				if (BOUNDS_CHECK)
				{
					r = sim->photons[y+ry][x+rx];
					if (!r)
						r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					rt = TYP(r);
					if (rt!=PT_CLNE && rt!=PT_PCLN &&
					    rt!=PT_BCLN &&  rt!=PT_SPRK &&
					    rt!=PT_NSCN && rt!=PT_PSCN &&
					    rt!=PT_STKM && rt!=PT_STKM2 &&
					    rt!=PT_PBCN && rt<PT_NUM)
					{
						parts[i].ctype = rt;
						if (rt==PT_LIFE || rt==PT_LAVA)
							parts[i].tmp = parts[ID(r)].ctype;
					}
				}
	if (parts[i].life!=10)
	{
		if (parts[i].life>0)
			parts[i].life--;
	}
	else
	{
		for (rx=-2; rx<3; rx++)
			for (ry=-2; ry<3; ry++)
				if (BOUNDS_CHECK && (rx || ry))
				{
					r = pmap[y+ry][x+rx];
					if (!r)
						continue;
					if (TYP(r)==PT_PBCN)
					{
						if (parts[ID(r)].life<10&&parts[ID(r)].life>0)
							parts[i].life = 9;
						else if (parts[ID(r)].life==0)
							parts[ID(r)].life = 10;
					}
				}
		if (parts[i].ctype>0 && parts[i].ctype<PT_NUM && sim->elements[parts[i].ctype].Enabled)
		{
			if (parts[i].ctype==PT_PHOT) {//create photons a different way
				for (rx=-1; rx<2; rx++)
					for (ry = -1; ry < 2; ry++)
						if (rx || ry)
						{
							int r = sim->create_part(-1, x + rx, y + ry, PT_PHOT);
							if (r != -1)
							{
								parts[r].vx = rx * 3;
								parts[r].vy = ry * 3;
								if (r>i)
								{
									// Make sure movement doesn't happen until next frame, to avoid gaps in the beams of photons produced
									parts[r].flags |= FLAG_SKIPMOVE;
								}
							}
						}
			}
			else if (parts[i].ctype==PT_LIFE)//create life a different way
				for (rx=-1; rx<2; rx++)
					for (ry=-1; ry<2; ry++)
						sim->create_part(-1, x+rx, y+ry, PT_LIFE, parts[i].tmp);

			else if (parts[i].ctype!=PT_LIGH || RNG::Ref().chance(1, 30))
			{
				int np = sim->create_part(-1, x + RNG::Ref().between(-1, 1), y + RNG::Ref().between(-1, 1), TYP(parts[i].ctype));
				if (np>-1)
				{
					if (parts[i].ctype==PT_LAVA && parts[i].tmp>0 && parts[i].tmp<PT_NUM && sim->elements[parts[i].tmp].HighTemperatureTransition==PT_LAVA)
						parts[np].ctype = parts[i].tmp;
				}
			}
		}
	}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int lifemod = ((cpart->life>10?10:cpart->life)*10);
	*colr += lifemod;
	*colg += lifemod/2;
	return 0;
}
