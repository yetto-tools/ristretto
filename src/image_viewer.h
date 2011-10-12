/*
 *  Copyright (c) Stephan Arts 2006-2010 <stephan@xfce.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __RISTRETTO_IMAGE_VIEWER_H__
#define __RISTRETTO_IMAGE_VIEWER_H__

G_BEGIN_DECLS

typedef enum
{
  RSTTO_IMAGE_VIEWER_ORIENT_NONE,
  RSTTO_IMAGE_VIEWER_ORIENT_90,
  RSTTO_IMAGE_VIEWER_ORIENT_180,
  RSTTO_IMAGE_VIEWER_ORIENT_270
} RsttoImageViewerOrientation;

#define RSTTO_TYPE_IMAGE_VIEWER rstto_image_viewer_get_type()

#define RSTTO_IMAGE_VIEWER(obj)( \
        G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                RSTTO_TYPE_IMAGE_VIEWER, \
                RsttoImageViewer))

#define RSTTO_IS_IMAGE_VIEWER(obj)( \
        G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                RSTTO_TYPE_IMAGE_VIEWER))

#define RSTTO_IMAGE_VIEWER_CLASS(klass)( \
        G_TYPE_CHECK_CLASS_CAST ((klass), \
                RSTTO_TYPE_IMAGE_VIEWER, \
                RsttoImageViewerClass))

#define RSTTO_IS_IMAGE_VIEWER_CLASS(klass)( \
        G_TYPE_CHECK_CLASS_TYPE ((klass), \
                RSTTO_TYPE_IMAGE_VIEWER()))

typedef struct _RsttoImageViewerPriv RsttoImageViewerPriv;

typedef struct _RsttoImageViewer RsttoImageViewer;

struct _RsttoImageViewer
{
    GtkWidget             parent;
    RsttoImageViewerPriv *priv;

    GtkAdjustment        *vadjustment;
    GtkAdjustment        *hadjustment;
};

typedef struct _RsttoImageViewerClass RsttoImageViewerClass;

struct _RsttoImageViewerClass
{
    GtkWidgetClass  parent_class;

    gboolean (* set_scroll_adjustments) (RsttoImageViewer *viewer,
          GtkAdjustment     *hadjustment,
          GtkAdjustment     *vadjustment);
};

GType
rstto_image_viewer_get_type();

GtkWidget *
rstto_image_viewer_new ();

void
rstto_image_viewer_set_file (
        RsttoImageViewer *viewer,
        RsttoFile *file,
        gdouble scale,
        RsttoImageViewerOrientation orientation);

void
rstto_image_viewer_set_scale (
        RsttoImageViewer *viewer,
        gdouble scale);

gdouble
rstto_image_viewer_get_scale (
        RsttoImageViewer *viewer);

void
rstto_image_viewer_set_orientation (
        RsttoImageViewer *viewer, 
        RsttoImageViewerOrientation orientation);

RsttoImageViewerOrientation
rstto_image_viewer_get_orientation (RsttoImageViewer *viewer);

void
rstto_image_viewer_set_menu (
    RsttoImageViewer *viewer,
    GtkMenu *menu);

G_END_DECLS

#endif /* __RISTRETTO_IMAGE_VIEWER_H__ */
