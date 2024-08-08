/*
Copyright (C) 2023-2024 starfrost

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/

// cl_ui_eventhandling.c : Split UI Event Handling from UI Code (December 27, 2023)
#include <client/client.h>

bool UI_SetEventOnClickDown(char* ui_name, char* control_name, void (*func)(int32_t btn, int32_t x, int32_t y))
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, control_name);

	if (!ui_control_ptr)
	{
		Com_Printf("ERROR: Tried to set unknown control on-click handler %s for UI %s!", control_name, current_ui->name);
		return false;
	}

	if (!func)
	{
		Com_Printf("ERROR: Tried to set UI clicked event callback for control %s on UI %s to null!", ui_control_ptr->name, current_ui->name);
		return false;
	}

	ui_control_ptr->on_click_down = func;
	return true;
}

bool UI_SetEventOnClickUp(char* ui_name, char* control_name, void (*func)(int32_t btn, int32_t x, int32_t y))
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, control_name);

	if (!ui_control_ptr)
	{
		Com_Printf("ERROR: Tried to set unknown control on-click handler %s for UI %s!", control_name, current_ui->name);
		return false;
	}

	if (!func)
	{
		Com_Printf("ERROR: Tried to set UI clicked event callback for control %s on UI %s to null!", ui_control_ptr->name, current_ui->name);
		return false;
	}

	ui_control_ptr->on_click_up = func;
	return true;
}

bool UI_SetEventOnKeyDown(char* ui_name, char* control_name, void (*func)(int32_t btn))
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, control_name);

	if (!ui_control_ptr)
	{
		Com_Printf("ERROR: Tried to set unknown control on-click handler %s for UI %s!", control_name, current_ui->name);
		return false;
	}

	if (!func)
	{
		Com_Printf("ERROR: Tried to set UI clicked event callback for control %s on UI %s to null!", ui_control_ptr->name, current_ui->name);
		return false;
	}

	ui_control_ptr->on_key_down = func;
	return true;
}

bool UI_SetEventOnKeyUp(char* ui_name, char* control_name, void (*func)(int32_t btn))
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, control_name);

	if (!ui_control_ptr)
	{
		Com_Printf("ERROR: Tried to set unknown control on-click handler %s for UI %s!", control_name, current_ui->name);
		return false;
	}

	if (!func)
	{
		Com_Printf("ERROR: Tried to set UI clicked event callback for control %s on UI %s to null!", ui_control_ptr->name, current_ui->name);
		return false;
	}

	ui_control_ptr->on_key_up = func;
	return true;
}

bool UI_SetEventOnUpdate(char* ui_name, char* control_name, void (*func)())
{
	ui_control_t* ui_control_ptr = UI_GetControl(ui_name, control_name);

	if (!ui_control_ptr)
	{
		Com_Printf("ERROR: Tried to set unknown control on-click handler %s for UI %s!", control_name, current_ui->name);
		return false;
	}

	if (!func)
	{
		Com_Printf("ERROR: Tried to set UI clicked event callback for control %s on UI %s to null!", ui_control_ptr->name, current_ui->name);
		return false;
	}

	ui_control_ptr->on_update = func;
	return true;
}

void UI_HandleEventOnClickUp(int32_t btn, int32_t x, int32_t y)
{
	if (current_ui == NULL) return;

	for (int32_t ui_num = 0; ui_num < num_uis; ui_num++)
	{
		ui_t* ui_ptr = &ui_list[ui_num];

		for (int32_t ui_control_num = 0; ui_control_num < ui_ptr->num_controls; ui_control_num++)
		{
			ui_control_t* ui_control_ptr = &ui_ptr->controls[ui_control_num];

			float final_pos_x = ui_control_ptr->position_x * r_width->value;
			float final_pos_y = ui_control_ptr->position_y * r_height->value;

			if (!current_ui->passive && !current_ui->activated)
				return;

			// Handle focus changes for key events
			// TODO: Scaling
			if (x >= final_pos_x
				&& y >= final_pos_y
				&& x <= final_pos_x + ((ui_control_ptr->size_x) * vid_hudscale->value)
				&& y <= final_pos_y + ((ui_control_ptr->size_y) * vid_hudscale->value)
				&& ui_ptr == current_ui)
			{
				ui_control_ptr->focused = true; 

				// if the UI has an onclickdown event handler, call it
				if (ui_control_ptr->on_click_up)
				{
					// YES this is terribly inefficient because you have to check 8 billion uis. I dont care. Fuck UI
					ui_control_ptr->on_click_up(btn, x, y);
				}
			}
			else
			{
				ui_control_ptr->focused = false;
			}
		}
	}
}

void UI_HandleEventOnClickDown(int32_t btn, int32_t x, int32_t y)
{
	if (current_ui == NULL) return;

	for (int32_t ui_num = 0; ui_num < num_uis; ui_num++)
	{
		ui_t* ui_ptr = &ui_list[ui_num];

		for (int32_t ui_control_num = 0; ui_control_num < ui_ptr->num_controls; ui_control_num++)
		{
			ui_control_t* ui_control_ptr = &ui_ptr->controls[ui_control_num];

			float final_pos_x = ui_control_ptr->position_x * r_width->value;
			float final_pos_y = ui_control_ptr->position_y * r_height->value;

			// Handle focus changes for key events
			// TODO: Scaling
			if (x >= final_pos_x
				&& y >= final_pos_y
				&& x <= final_pos_x + ((ui_control_ptr->size_x) * vid_hudscale->value)
				&& y <= final_pos_y + ((ui_control_ptr->size_y) * vid_hudscale->value)
				&& ui_ptr == current_ui)
			{
				ui_control_ptr->focused = true;

				// if the UI has an onclickdown event handler, call it
				// YES this is terribly inefficient because you have to check 8 billion uis. I dont care. Fuck UI
				if (ui_control_ptr->on_click_down)
					ui_control_ptr->on_click_down(btn, x, y);
			}
			else
			{
				ui_control_ptr->focused = false;
			}
		}
	}
}

void UI_HandleEventOnKeyDown(int32_t btn)
{
	for (int32_t ui_num = 0; ui_num < num_uis; ui_num++)
	{
		ui_t* ui_ptr = &ui_list[ui_num];

		for (int32_t ui_control_num = 0; ui_control_num < ui_ptr->num_controls; ui_control_num++)
		{
			ui_control_t* ui_control_ptr = &ui_ptr->controls[ui_control_num];

			// checking for focusing is the choice of each individual UI
			if (ui_control_ptr->on_key_down)
				ui_control_ptr->on_key_down(btn);
		}		
	}
}

void UI_HandleEventOnKeyUp(int32_t btn)
{
	for (int32_t ui_num = 0; ui_num < num_uis; ui_num++)
	{
		ui_t* ui_ptr = &ui_list[ui_num];

		for (int32_t ui_control_num = 0; ui_control_num < ui_ptr->num_controls; ui_control_num++)
		{
			ui_control_t* ui_control_ptr = &ui_ptr->controls[ui_control_num];

			// checking for focusing is the choice of each individual UI
			if (ui_control_ptr->on_key_up)
				ui_control_ptr->on_key_up(btn);
		}
	}
}

void UI_HandleEventOnUpdate(ui_control_t* ui_control_ptr)
{
	if (ui_control_ptr->on_update)
		ui_control_ptr->on_update();
}