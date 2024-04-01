/*
Copyright (C) 1997-2001 Id Software, Inc.

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
// gl_init.c - Contains GLAD and GLFW init stuff
// =============================================


#include <assert.h>
#include <windows.h>
#include "../ref_gl/gl_local.h"
#include "winquake.h"


gl_state_t  gl_state;			// The OpenGL state

static bool GLimp_SwitchFullscreen( int32_t width, int32_t height );
bool GL_Init (void);

extern cvar_t *vid_fullscreen;
extern cvar_t *vid_ref;

static bool VerifyDriver( void )
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
#define GLFW_ERROR_MAX	256

bool VID_CreateWindow( int32_t width, int32_t height, bool fullscreen)
{
	char glfw_error[GLFW_ERROR_MAX];

	GLFWmonitor* monitor = NULL;

	// set fullscreen on the primary monitor
	if (fullscreen)
	{
		monitor = glfwGetPrimaryMonitor();

		if (!monitor)
		{
			glfwGetError(&glfw_error);
			ri.Con_Printf(PRINT_ALL, "Failed to set fullscreen (couldn't get primary monitor): %d\n", glfw_error);
			return false;
		}
	}

	gl_state.window = glfwCreateWindow(width, height, "Zombono (Legacy OpenGL 1.5) "CPUSTRING, NULL, NULL);

	if (!gl_state.window)
	{
		int error_code = glfwGetError(&glfw_error);
		ri.Con_Printf(PRINT_ALL, "GLFW failed to create a window: %s\n", glfw_error);
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
	glfwSetWindowCloseCallback(gl_state.window, GL_Shutdown);

	// let the sound and input subsystems know about the new window
	ri.Vid_NewWindow (width, height);

	return true;
}


/*
** GLimp_SetMode
*/
rserr_t GL_SetMode( int32_t *pwidth, int32_t *pheight, int32_t mode, bool fullscreen )
{
	int32_t width, height;
	const char *win_fs[] = { "Windowed", "Fullscreen" };

	ri.Con_Printf( PRINT_ALL, "Initializing OpenGL display\n");

	ri.Con_Printf (PRINT_ALL, "...setting mode %d:", mode );

	if ( !ri.Vid_GetModeInfo( &width, &height, mode ) )
	{
		ri.Con_Printf( PRINT_ALL, " invalid mode\n" );
		return rserr_invalid_mode;
	}

	ri.Con_Printf( PRINT_ALL, " %d %d %s\n", width, height, win_fs[fullscreen] );

	// destroy the existing window
	if (gl_state.window)
	{
		GL_Shutdown ();
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

/*
** GLimp_Shutdown
**
** This routine does all OS specific shutdown procedures for the OpenGL
** subsystem.  Under OpenGL this means NULLing out the current DC and
** HGLRC, deleting the rendering context, and releasing the DC acquired
** for the window.  The state structure is also nulled out.
**
*/
void GL_Shutdown(void)
{
	glfwDestroyWindow(gl_state.window);
	gl_state.window = NULL;
}


/*
** GLimp_Init
**
** This routine is responsible for initializing the OS specific portions
** of OpenGL.  Under Win32 this means dealing with the pixelformats and
** doing the wgl interface stuff.
*/
int32_t GLimp_Init( void *hinstance, void *wndproc )
{
	return true;
}

// This routine initialises GLAD and GLFW, and gets an OpenGL context.
bool GL_Init (void)
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
** GLimp_BeginFrame
*/
void GLimp_BeginFrame( float camera_separation )
{
	if ( gl_bitdepth->modified )
	{
		gl_bitdepth->modified = false;
	}

	if ( camera_separation < 0 && gl_state.stereo_enabled )
	{
		glDrawBuffer( GL_BACK_LEFT );
	}
	else if ( camera_separation > 0 && gl_state.stereo_enabled )
	{
		glDrawBuffer( GL_BACK_RIGHT );
	}
	else
	{
		glDrawBuffer( GL_BACK );
	}
}

/*
** GLimp_EndFrame
** 
** Responsible for doing a swapbuffers and possibly for other stuff
** as yet to be determined.  Probably better not to make this a GLimp
** function and instead do a call to GLimp_SwapBuffers.
*/
void GLimp_EndFrame (void)
{
	int		err;

	err = glGetError();
	assert( err == GL_NO_ERROR );

	// swap the buffers
	glfwSwapBuffers(gl_state.window);
	glfwSwapInterval(1);
}

/*
** GLimp_AppActivate
*/
void GLimp_AppActivate( bool active )
{
	/*
	if ( active )
	{
		
		SetForegroundWindow( gl_state.hWnd );
		ShowWindow( gl_state.hWnd, SW_RESTORE );
	}
	else
	{
		if ( vid_fullscreen->value )
			ShowWindow( gl_state.hWnd, SW_MINIMIZE );
	}
	*/
}
