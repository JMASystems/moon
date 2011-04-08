/* -*- Mode: C++; tab-width: 8; indent-tabs-mode: t; c-basic-offset: 8 -*- */
/*
 * window-gtk.cpp: MoonWindow implementation using gtk widgets.
 *
 * Contact:
 *   Moonlight List (moonlight-list@lists.ximian.com)
 *
 * Copyright 2007-2008 Novell, Inc. (http://www.novell.com)
 *
 * See the LICENSE file included with the distribution for details.
 * 
 */

#include "config.h"

#include <glib.h>

#include "android_native_app_glue.h"

#include "window-android.h"
#include "clipboard-android.h"
#include "pixbuf-android.h"
#include "deployment.h"
#include "timemanager.h"
#include "enums.h"
#include "context-cairo.h"
#ifdef USE_GALLIUM
#define __MOON_GALLIUM__
#include "context-gallium.h"
#ifdef CLAMP
#undef CLAMP
#endif
#include "util/u_inlines.h"
#endif

// Gallium context cache size.
//
#define CONTEXT_CACHE_SIZE 1

using namespace Moonlight;

#ifdef USE_GALLIUM
int MoonWindowAndroid::gctxn = 0;
#endif

MoonWindowAndroid::MoonWindowAndroid (MoonWindowType windowType, int w, int h, MoonWindow *parent, Surface *surface)
	: MoonWindow (windowType, w, h, parent, surface)
{
	this->width = w;
	this->height = h;
	damage = new Region ();

	CreateCairoContext ();
}

MoonWindowAndroid::~MoonWindowAndroid ()
{
}

void
MoonWindowAndroid::ConnectToContainerPlatformWindow (gpointer container_window)
{
}

MoonClipboard*
MoonWindowAndroid::GetClipboard (MoonClipboardType clipboardType)
{
	return new MoonClipboardAndroid (this, clipboardType);
}

gpointer
MoonWindowAndroid::GetPlatformWindow ()
{
	// FIXME
	return NULL;
}

void
MoonWindowAndroid::Resize (int width, int height)
{
	if (this->width == width && this->height == height)
		return;
	
	g_warning ("buffer = (%d,%d) surface = (%d,%d)", width, height, this->width, this->height);
	
	this->width = width;
	this->height = height;

	
	delete damage;
	damage = new Region (0.0, 0.0, width, height);

	CreateCairoContext();
	
	if (surface)
		surface->HandleUIWindowAllocation (true);
	
}	

void
MoonWindowAndroid::SetBackgroundColor (Color *color)
{
	// FIXME
}

void
MoonWindowAndroid::SetCursor (CursorType cursor)
{
	// FIXME
}

void
MoonWindowAndroid::Invalidate (Rect r)
{
	// FIXME
	damage->Union (r);
}

void
MoonWindowAndroid::ProcessUpdates ()
{
	// FIXME
}

gboolean
MoonWindowAndroid::HandleEvent (gpointer platformEvent)
{
	// FIXME
	return TRUE;
}

void
MoonWindowAndroid::Show ()
{
	if (surface) {
		surface->HandleUIWindowUnavailable ();
		surface->HandleUIWindowAvailable ();
	}
}

void
MoonWindowAndroid::Hide ()
{
	if (surface)
		surface->HandleUIWindowAvailable ();
}

void
MoonWindowAndroid::EnableEvents (bool first)
{
	// FIXME
}

void
MoonWindowAndroid::DisableEvents ()
{
	// FIXME
}

void
MoonWindowAndroid::GrabFocus ()
{
	// FIXME
}

bool
MoonWindowAndroid::HasFocus ()
{
	// FIXME
	return false;
}

void
MoonWindowAndroid::SetLeft (double left)
{
	// FIXME
}

double
MoonWindowAndroid::GetLeft ()
{
	return left;
}

void
MoonWindowAndroid::SetTop (double top)
{
	// FIXME
}

double
MoonWindowAndroid::GetTop ()
{
	return top;
}

void
MoonWindowAndroid::SetWidth (double width)
{
	if (this->width == width)
		return;
	
	Resize (width, this->height);
}

void
MoonWindowAndroid::SetHeight (double height)
{
	if (this->height == height)
		return;

	Resize (this->width, height);
}

void
MoonWindowAndroid::SetTitle (const char *title)
{
	// FIXME
}

void
MoonWindowAndroid::SetIconFromPixbuf (MoonPixbuf *pixbuf)
{
	// FIXME
}

void
MoonWindowAndroid::SetStyle (WindowStyle style)
{
	// FIXME
}

static inline guint8
convert_color_channel (guint8 src, guint8 alpha)
{
	return alpha ? src / (alpha / 255.0) : 0;
}

static inline void
convert_bgra_to_rgba (guint8 const *src, guint8 *dst, gint width, gint height)
{
	guint8 const *src_pixel = src;
	guint8 * dst_pixel = dst;
	int y;

	for (y = 0; y < height; y++)
	{   
		int x;

		for (x = 0; x < width; x++)
		{   
			dst_pixel[0] = convert_color_channel (src_pixel[2], src_pixel[3]);
			dst_pixel[1] = convert_color_channel (src_pixel[1], src_pixel[3]);
			dst_pixel[2] = convert_color_channel (src_pixel[0], src_pixel[3]);
			dst_pixel[3] = src_pixel[3];

			dst_pixel += 4;
			src_pixel += 4;
		}   
	}   
}

void
MoonWindowAndroid::CreateCairoContext ()
{
	CairoSurface *target;

	target = new CairoSurface (width, height);
	ctx = new CairoContext (target);

	backing_image_data = target->GetData ();
	target->unref ();
}

void
MoonWindowAndroid::Paint (gpointer data)
{
	ANativeWindow_Buffer buffer;
	unsigned char *pixels;
	cairo_t *cr;
	struct android_app *app = (struct android_app *) data;

	g_warning ("Painting");
	SetCurrentDeployment ();

	if (app->window == NULL)
		return;
		
	if (damage->IsEmpty ()) {
		//g_warning ("no damage");
		return;
	}
		
	g_warning ("Lock");
	if (ANativeWindow_lock (app->window, &buffer, NULL) < 0)
		return;
	
	if (width != buffer.width || height != buffer.height) {
		Resize (buffer.width, buffer.height);
		delete damage;
		damage = new Region (Rect (0, 0, buffer.width, buffer.height));
	}
	pixels = (unsigned char *) buffer.bits;
	
	Rect extents = damage->GetExtents();
	g_warning ("Blit damage = (%g,%g,%g,%g", extents.y, extents.x, extents.width, extents.height);
	surface->Paint (ctx, damage, transparent, true);
	delete damage;
	damage = new Region ();
	g_warning ("Splat");
	convert_bgra_to_rgba (backing_image_data, pixels, buffer.width, buffer.height);

	ANativeWindow_unlockAndPost (app->window);
}