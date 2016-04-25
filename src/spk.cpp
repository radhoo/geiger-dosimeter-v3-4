/*
 * Definitions for simple SPEAKER usage 
 * http://www.pocketmagic.net/
 *
 * Copyright (c) 2013 by Radu Motisan , radu.motisan@gmail.com
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * For more information on the GPL, please go to:
 * http://www.gnu.org/copyleft/gpl.html
 */





#include "spk.h"

int spk_state = 0;
int SPKInit()
{
	SPK_OUTPUT_MODE();
	
	return 1;
}

//--------------------------------------------------------------------------------------------
// set spk on off
int SPKSet(int st)	
{
	spk_state = st;
	if (st) 
		SPK_HIGH(); 
	else 
		SPK_LOW();
	return 1;
}

void SPKToggle() {
	if (spk_state) SPKSet(0); else SPKSet(1);
}

