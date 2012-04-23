/*
 *  Copyright (c) Stephan Arts 2006-2012 <stephan@xfce.org>
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 * 
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 *  02110-1301, USA.
 */

#ifndef __RISTRETTO_MONITOR_CHOOSER_H__
#define __RISTRETTO_MONITOR_CHOOSER_H__

G_BEGIN_DECLS

#define RSTTO_TYPE_MONITOR_CHOOSER rstto_monitor_chooser_get_type()

#define RSTTO_MONITOR_CHOOSER(obj)( \
        G_TYPE_CHECK_INSTANCE_CAST ((obj), \
                RSTTO_TYPE_MONITOR_CHOOSER, \
                RsttoMonitorChooser))

#define RSTTO_IS_MONITOR_CHOOSER(obj)( \
        G_TYPE_CHECK_INSTANCE_TYPE ((obj), \
                RSTTO_TYPE_MONITOR_CHOOSER))

#define RSTTO_MONITOR_CHOOSER_CLASS(klass)( \
        G_TYPE_CHECK_CLASS_CAST ((klass), \
                RSTTO_TYPE_MONITOR_CHOOSER, \
                RsttoMonitorChooserClass))

#define RSTTO_IS_MONITOR_CHOOSER_CLASS(klass)( \
        G_TYPE_CHECK_CLASS_TYPE ((klass), \
                RSTTO_TYPE_MONITOR_CHOOSER()))

typedef struct _RsttoMonitorChooserPriv RsttoMonitorChooserPriv;

typedef struct _RsttoMonitorChooser RsttoMonitorChooser;

struct _RsttoMonitorChooser
{
    GtkWidget parent;
    RsttoMonitorChooserPriv *priv;
};

typedef struct _RsttoMonitorChooserClass RsttoMonitorChooserClass;

struct _RsttoMonitorChooserClass
{
    GtkWidgetClass  parent_class;
};

GType      rstto_monitor_chooser_get_type();

GtkWidget  *rstto_monitor_chooser_new ();

gint
rstto_monitor_chooser_add ( 
        RsttoMonitorChooser *,
        gint width,
        gint height);

gint
rstto_monitor_chooser_set_image_surface (
        RsttoMonitorChooser *chooser,
        gint monitor_id,
        cairo_surface_t *surface,
        GError **error);

gint
rstto_monitor_chooser_get_selected (
        RsttoMonitorChooser *);

void
rstto_monitor_chooser_get_dimensions (
        RsttoMonitorChooser *,
        gint nr,
        gint *width,
        gint *height);

G_END_DECLS

#endif /* __RISTRETTO_MONITOR_CHOOSER_H__ */
