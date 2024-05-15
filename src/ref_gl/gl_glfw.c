/*
Copyright (C) 1997-2001 Id Software, Inc.
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

// =============================================
// gl_init.c - Contains GLAD and GLFW glue stuff
// =============================================


#include <assert.h>
#include "gl_local.h"

gl_state_t  gl_state;			// The OpenGL state

bool GL_Init();

void GLFW_Error(const char* error);
void GL_DestroyWindow();
void GL_WindowSizeChanged(GLFWwindow* window, int32_t width, int32_t height); 

extern cvar_t* vid_fullscreen;
extern cvar_t* vid_borderless;
extern cvar_t* vid_ref;

static bool VerifyDriver()
{
	char buffer[1024];

	if (!glGetString)
		return false;

	strcpy( buffer, glGetString( GL_RENDERER ) );
	strlwr( buffer );

	return (buffer == NULL);
}

/*
** VID_CreateWindow
*/

bool VID_CreateWindow(int32_t width, int32_t height, bool fullscreen)
{
	// Monitor is NULL for windowed mode,
	// non-NULL for fullscreen
	GLFWmonitor* monitor = NULL;

	// Fullscreen in GLFW Zombono is borderless windowed by default, unless the user chose vid_fullscreen
	if (fullscreen)
	{
		gl_state.fullscreen = true;

		// determine if the user wanted borderless fullscreen or real fullscreen
		bool dedicated_fullscreen = (vid_fullscreen->value);

		if (dedicated_fullscreen)
		{
			monitor = glfwGetPrimaryMonitor(); // set 

			if (!monitor)
			{
				ri.Con_Printf(PRINT_ALL, "Failed to obtain the primary monitor for dedicated fullscreen!");
				return false;
			}
		}
		else // fullscreen was specified, but not dedi
		{
			glfwWindowHint(GLFW_DECORATED, GLFW_FALSE); // dedicated fullscreen is a hint in GLFW
		}
	}

	gl_state.window = glfwCreateWindow(width, height, "Zombono (Legacy OpenGL 1.5) "CPU_ARCH, monitor, NULL);

	if (!gl_state.window)
	{
		ri.Con_Printf(PRINT_ALL, "GLFW failed to create a window\n");
		return false;
	}
	
	// make the GLFW context current
	glfwMakeContextCurrent(gl_state.window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		ri.Con_Printf(PRINT_ALL, "GL_Init: Failed to initialise GLAD\n");
		return false;
	}

	// set up callbacks
	glfwSetWindowSizeCallback(gl_state.window, GL_WindowSizeChanged);
	glfwSetWindowCloseCallback(gl_state.window, GL_Shutdown);

	//TODO: vid_xpos, vid_ypos...
	/*
	** get our various GL strings
	*/
	gl_config.vendor_string = glGetString(GL_VENDOR);
	ri.Con_Printf(PRINT_ALL, "GL_VENDOR: %s\n", gl_config.vendor_string);
	gl_config.renderer_string = glGetString(GL_RENDERER);
	ri.Con_Printf(PRINT_ALL, "GL_RENDERER: %s\n", gl_config.renderer_string);
	gl_config.version_string = glGetString(GL_VERSION);
	ri.Con_Printf(PRINT_ALL, "GL_VERSION: %s\n", gl_config.version_string);
	gl_config.extensions_string = glGetString(GL_EXTENSIONS);
	ri.Con_Printf(PRINT_ALL, "GL_EXTENSIONS: %s\n", gl_config.extensions_string);

	glfwSetErrorCallback(gl_state.window, GLFW_Error);
	
	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow (width, height);

	// move the window if the user specified 
	if (!fullscreen)
	{
		int vid_xpos = ri.Cvar_Get("vid_xpos", "0", 0)->value;
		int vid_ypos = ri.Cvar_Get("vid_ypos", "0", 0)->value;

		glfwSetWindowPos(gl_state.window, vid_xpos, vid_ypos);
	}

	glfwSwapInterval(gl_vsync->value);

	return true;
}

void GL_WindowSizeChanged(GLFWwindow* window, int32_t width, int32_t height)
{
	glViewport(0, 0, width, height);
}

/*
** GL_SetMode
*/
rserr_t GL_SetMode( int32_t* pwidth, int32_t* pheight, int32_t mode, bool fullscreen )
{
	int32_t width, height;
	const char *window_modes[] = { "Windowed", "Fullscreen"};

	ri.Con_Printf(PRINT_ALL, "Initializing OpenGL display\n");

	ri.Con_Printf(PRINT_ALL, "...setting mode %d:", mode );

	if ( !ri.Vid_GetModeInfo( &width, &height, mode ) )
	{
		ri.Con_Printf( PRINT_ALL, " invalid mode\n" );
		return rserr_invalid_mode;
	}

	// destroy the existing window
	if (gl_state.window)
	{
		GL_DestroyWindow ();
		gl_state.window = NULL;
	}

	// do a CDS if needed
	if ( fullscreen )
	{
		ri.Con_Printf( PRINT_ALL, "...attempting fullscreen\n" );

		ri.Con_Printf( PRINT_ALL, "...calling CDS: " );
		if ( VID_CreateWindow(width, height, true) )
		{
			*pwidth = width;
			*pheight = height;

			gl_state.fullscreen = true;

			ri.Con_Printf( PRINT_ALL, "ok\n" );
			return rserr_ok;
		}
		else
		{
			ri.Con_Printf( PRINT_ALL, " failed\n" );
			ri.Con_Printf( PRINT_ALL, "...setting windowed mode\n" );

			// gl_state.window is already null
			VID_CreateWindow(width, height, false);

			*pwidth = width;
			*pheight = height;
			gl_state.fullscreen = false;
			return rserr_invalid_fullscreen;
		}
	}
	else
	{
		ri.Con_Printf( PRINT_ALL, "...setting windowed mode\n" );

		*pwidth = width;
		*pheight = height;
		gl_state.fullscreen = false;
		if ( !VID_CreateWindow (width, height, false) )
			return rserr_invalid_mode;
	}

	return rserr_ok;
}

void GL_DestroyWindow()
{
	glfwDestroyWindow(gl_state.window);
}

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/
void GL_Shutdown()
{
	GL_DestroyWindow();
	glfwTerminate(gl_state.window);
	gl_state.window = NULL;
}

// This routine initialises GLAD and GLFW, and gets an OpenGL context.
bool GL_Init()
{
	if (glfwInit() == GLFW_FALSE)
	{
		ri.Con_Printf(PRINT_ALL, "GL_Init: Failed to initialise GLFW\n");
		return false;
	}

	// Set it to OpenGL 1.5, compatibility profile
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 1);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
	return true;
}

/*
** GL_BeginFrame
*/
void GL_BeginFrame()
{
	if (gl_state.window == NULL)
	{
		return;
	}

	if ( gl_bitdepth->modified )
	{
		gl_bitdepth->modified = false;
	}

	glDrawBuffer(GL_BACK);
}

/*
** GL_EndFrame
** 
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
void GL_EndFrame()
{
	// if the window closed exit
	if (gl_state.window == NULL)
	{
		return;
	}

	int		err;

	err = glGetError();
	assert(err == GL_NO_ERROR);

	glfwPollEvents();

	if (!stricmp(gl_drawbuffer->string, "GL_BACK"))
	{
		// swap the buffers
		glfwSwapBuffers(gl_state.window);
	}
}

void	GL_SetMousePressedProc(void proc(void* unused, int32_t button, int32_t action, int32_t mods))
{
	glfwSetMouseButtonCallback(gl_state.window, proc);
}

void	GL_SetMouseScrollProc(void proc(void* unused, double xoffset, double yoffset))
{
	glfwSetScrollCallback(gl_state.window, proc);
}

void	GL_SetKeyPressedProc(void proc(void* unused, int32_t key, int32_t scancode, int32_t action, int32_t mods))
{
	glfwSetKeyCallback(gl_state.window, proc);
}

void	GL_SetMouseMovedProc(void proc(void* unused, int32_t xpos, int32_t ypos))
{
	glfwSetCursorPosCallback(gl_state.window, proc);
}

void	GL_SetWindowFocusProc(void proc(void* unused, int32_t focused))
{
	glfwSetWindowFocusCallback(gl_state.window, proc);
}

void	GL_SetWindowIconifyProc(void proc(void* unused, int32_t iconified))
{
	glfwSetWindowIconifyCallback(gl_state.window, proc);
}

void	GL_GetCursorPosition(double* x, double* y)
{
	glfwGetCursorPos(gl_state.window, x, y);
}

// RELATIVE TO THE WINDOW
void	GL_SetCursorPosition(double x, double y)
{
	glfwSetCursorPos(gl_state.window, x, y);
}

void	GL_SetWindowPosition(double x, double y)
{
	glfwSetWindowPos(gl_state.window, x, y);
}

void	GL_EnableCursor(bool enabled)
{
	if (!enabled)
	{
		// turn off mouse acceleration
		if (glfwRawMouseMotionSupported())
		{
			glfwSetInputMode(gl_state.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
		}

		glfwSetInputMode(gl_state.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}
	else
	{
		glfwSetInputMode(gl_state.window, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);

		// turn it back on
		glfwSetInputMode(gl_state.window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}

void GLFW_Error(const char* error)
{
	Sys_Error("A fatal GLFW error occurred:\n\n\n%s", error);
}