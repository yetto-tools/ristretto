/*
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

#include <config.h>
#include <gtk/gtk.h>
#include <gtk/gtkmarshal.h>
#include <string.h>

#include <thunar-vfs/thunar-vfs.h>
#include <libexif/exif-data.h>

#include "navigator.h"
#include "thumbnail.h"
#include "thumbnail_bar.h"

struct _RsttoThumbnailBarPriv
{
    GtkOrientation  orientation;
    RsttoNavigator *navigator;
    gint dimension;
    gint offset;
    gboolean auto_center;
    gint begin;
    gint end;
    GSList *thumbs;
};

static void
rstto_thumbnail_bar_init(RsttoThumbnailBar *);
static void
rstto_thumbnail_bar_class_init(RsttoThumbnailBarClass *);

static void
rstto_thumbnail_bar_size_request(GtkWidget *, GtkRequisition *);
static void
rstto_thumbnail_bar_size_allocate(GtkWidget *, GtkAllocation *);

static void
rstto_thumbnail_bar_add(GtkContainer *container, GtkWidget *child);
static void
rstto_thumbnail_bar_remove(GtkContainer *container, GtkWidget *child);
static void
rstto_thumbnail_bar_forall(GtkContainer *container, gboolean include_internals, GtkCallback callback, gpointer callback_data);
static GType
rstto_thumbnail_bar_child_type(GtkContainer *container);

static GtkWidgetClass *parent_class = NULL;

static void
cb_rstto_thumbnail_bar_nav_new_entry (RsttoNavigator *nav,
                                    gint nr,
                                    RsttoNavigatorEntry *entry,
                                    RsttoThumbnailBar *bar);
static void
cb_rstto_thumbnail_bar_nav_iter_changed (RsttoNavigator *nav,
                                       gint nr,
                                       RsttoNavigatorEntry *entry,
                                       RsttoThumbnailBar *bar);
static void
cb_rstto_thumbnail_bar_nav_reordered (RsttoNavigator *nav,
                                    RsttoThumbnailBar *bar);

static void
cb_rstto_thumbnail_bar_thumbnail_toggled (RsttoThumbnail *thumb, RsttoThumbnailBar *bar);

static gint
cb_rstto_thumbnail_bar_compare (RsttoThumbnail *a, RsttoThumbnail *b);

GType
rstto_thumbnail_bar_get_type ()
{
    static GType rstto_thumbnail_bar_type = 0;

    if (!rstto_thumbnail_bar_type)
    {
        static const GTypeInfo rstto_thumbnail_bar_info = 
        {
            sizeof (RsttoThumbnailBarClass),
            (GBaseInitFunc) NULL,
            (GBaseFinalizeFunc) NULL,
            (GClassInitFunc) rstto_thumbnail_bar_class_init,
            (GClassFinalizeFunc) NULL,
            NULL,
            sizeof (RsttoThumbnailBar),
            0,
            (GInstanceInitFunc) rstto_thumbnail_bar_init,
            NULL
        };

        rstto_thumbnail_bar_type = g_type_register_static (GTK_TYPE_CONTAINER, "RsttoThumbnailBar", &rstto_thumbnail_bar_info, 0);
    }
    return rstto_thumbnail_bar_type;
}


static void
rstto_thumbnail_bar_init(RsttoThumbnailBar *bar)
{
    bar->priv = g_new0(RsttoThumbnailBarPriv, 1);

    bar->priv->auto_center = TRUE;

	GTK_WIDGET_SET_FLAGS(bar, GTK_NO_WINDOW);
	gtk_widget_set_redraw_on_allocate(GTK_WIDGET(bar), FALSE);
    bar->priv->orientation = GTK_ORIENTATION_HORIZONTAL;
    bar->priv->offset = 0;

}

static void
rstto_thumbnail_bar_class_init(RsttoThumbnailBarClass *bar_class)
{
    GtkWidgetClass *widget_class;
    GtkContainerClass *container_class;

    widget_class = (GtkWidgetClass*)bar_class;
    container_class = (GtkContainerClass*)bar_class;

    parent_class = g_type_class_peek_parent(bar_class);

    widget_class->size_request = rstto_thumbnail_bar_size_request;
    widget_class->size_allocate = rstto_thumbnail_bar_size_allocate;

	container_class->add = rstto_thumbnail_bar_add;
	container_class->remove = rstto_thumbnail_bar_remove;
	container_class->forall = rstto_thumbnail_bar_forall;
	container_class->child_type = rstto_thumbnail_bar_child_type;

	gtk_widget_class_install_style_property (widget_class,
		g_param_spec_int ("spacing",
		_("Spacing"),
		_("The amount of space between the thumbnails"),
		0, G_MAXINT, 3,
		G_PARAM_READABLE));
}

static void
rstto_thumbnail_bar_size_request(GtkWidget *widget, GtkRequisition *requisition)
{
    RsttoThumbnailBar *bar = RSTTO_THUMBNAIL_BAR(widget);
    gint border_width = GTK_CONTAINER(bar)->border_width;
    GSList *iter;

	GtkRequisition child_requisition;

    requisition->height = 70;
    requisition->width = 70;

    for(iter = bar->priv->thumbs; iter; iter = g_slist_next(iter))
    {
		gtk_widget_size_request(GTK_WIDGET(iter->data), &child_requisition);
		requisition->width = MAX(child_requisition.width, requisition->width);
		requisition->height = MAX(child_requisition.height, requisition->height);
    }

    requisition->height += (border_width * 2);
    requisition->width += (border_width * 2);

	widget->requisition = *requisition;
}

static void
rstto_thumbnail_bar_size_allocate(GtkWidget *widget, GtkAllocation *allocation)
{
    RsttoThumbnailBar *bar = RSTTO_THUMBNAIL_BAR(widget);
    gint border_width = GTK_CONTAINER(bar)->border_width;
    gint spacing = 0;
	gtk_widget_style_get(widget, "spacing", &spacing, NULL);
    widget->allocation = *allocation;
    GtkAllocation child_allocation;
    GtkRequisition child_requisition;

    child_allocation.x = allocation->x + border_width - bar->priv->offset;
    child_allocation.y = allocation->y + border_width;
    child_allocation.height = 0;
    child_allocation.width = 0;

    GSList *iter = bar->priv->thumbs;

    switch(bar->priv->orientation)
    {
        case GTK_ORIENTATION_HORIZONTAL:
            while(iter)
            {
                gtk_widget_get_child_requisition(GTK_WIDGET(iter->data), &child_requisition);
                allocation->height = MAX(child_requisition.height + (border_width * 2), allocation->height);

                iter = g_slist_next(iter);
            }
            
            iter = bar->priv->thumbs;

            while(iter)
            {
                gtk_widget_get_child_requisition(GTK_WIDGET(iter->data), &child_requisition);
                child_allocation.height = allocation->height - (border_width * 2);
                child_allocation.width = child_requisition.width;

                if ((child_allocation.x < (allocation->x + allocation->width)) &&
                    ((child_allocation.x + child_allocation.width) > allocation->x + border_width))
                {
                    gtk_widget_set_child_visible(GTK_WIDGET(iter->data), TRUE);
                    gtk_widget_size_allocate(GTK_WIDGET(iter->data), &child_allocation);
                }
                else
                    gtk_widget_set_child_visible(GTK_WIDGET(iter->data), FALSE);

                child_allocation.x += child_requisition.width + spacing;
                iter = g_slist_next(iter);
            }
            break;
        case GTK_ORIENTATION_VERTICAL:
            while(iter)
            {
                gtk_widget_get_child_requisition(GTK_WIDGET(iter->data), &child_requisition);
                allocation->width = MAX(child_requisition.width + (border_width * 2), allocation->width);

                iter = g_slist_next(iter);
            }
            
            iter = bar->priv->thumbs;

            while(iter)
            {

                gtk_widget_get_child_requisition(GTK_WIDGET(iter->data), &child_requisition);
                child_allocation.width = allocation->width - (border_width * 2);
                child_allocation.height = child_requisition.height;

                if (child_allocation.y < (allocation->y + allocation->height))
                    gtk_widget_set_child_visible(GTK_WIDGET(iter->data), TRUE);
                else
                    gtk_widget_set_child_visible(GTK_WIDGET(iter->data), FALSE);

                gtk_widget_size_allocate(GTK_WIDGET(iter->data), &child_allocation);
                child_allocation.y += child_requisition.height + spacing;
                iter = g_slist_next(iter);
            }
            break;
    }
}

GtkWidget *
rstto_thumbnail_bar_new(RsttoNavigator *navigator)
{
    RsttoThumbnailBar *bar;

    bar = g_object_new(RSTTO_TYPE_THUMBNAIL_BAR, NULL);

    bar->priv->navigator = navigator;

    g_signal_connect(G_OBJECT(navigator), "new-entry", G_CALLBACK(cb_rstto_thumbnail_bar_nav_new_entry), bar);
    g_signal_connect(G_OBJECT(navigator), "iter-changed", G_CALLBACK(cb_rstto_thumbnail_bar_nav_iter_changed), bar);
    g_signal_connect(G_OBJECT(navigator), "reordered", G_CALLBACK(cb_rstto_thumbnail_bar_nav_reordered), bar);

    return (GtkWidget *)bar;
}

/*
 * rstto_thumbnail_bar_set_orientation:
 *
 * @bar      : ThumbnailBar
 * @orientation :
 *
 */
void
rstto_thumbnail_bar_set_orientation (RsttoThumbnailBar *bar, GtkOrientation orientation)
{
    bar->priv->orientation = orientation;
}

/*
 * rstto_thumbnail_bar_get_orientation:
 *
 * @bar      : ThumbnailBar
 *
 * Return value : GtkOrientation
 *
 */
GtkOrientation
rstto_thumbnail_bar_get_orientation (RsttoThumbnailBar *bar)
{
    return bar->priv->orientation;
}

static void
rstto_thumbnail_bar_add(GtkContainer *container, GtkWidget *child)
{
	g_return_if_fail(GTK_IS_WIDGET(child));
    RsttoThumbnailBar *bar = RSTTO_THUMBNAIL_BAR(container);

	gtk_widget_set_parent(child, GTK_WIDGET(container));

    bar->priv->thumbs = g_slist_insert_sorted(bar->priv->thumbs, child, (GCompareFunc)cb_rstto_thumbnail_bar_compare);
}

static void
rstto_thumbnail_bar_remove(GtkContainer *container, GtkWidget *child)
{
	g_return_if_fail(GTK_IS_WIDGET(child));

    RsttoThumbnailBar *bar = RSTTO_THUMBNAIL_BAR(container);
	gboolean widget_was_visible;

	widget_was_visible = GTK_WIDGET_VISIBLE(child);

    bar->priv->thumbs = g_slist_remove(bar->priv->thumbs, child);

	gtk_widget_unparent(child);

	/* remove from list is somewhere else */
	if(widget_was_visible)
		gtk_widget_queue_resize(GTK_WIDGET(container));
}

static void
rstto_thumbnail_bar_forall(GtkContainer *container, gboolean include_internals, GtkCallback callback, gpointer callback_data)
{
    RsttoThumbnailBar *bar = RSTTO_THUMBNAIL_BAR(container);

    g_return_if_fail(callback != NULL);

    g_slist_foreach(bar->priv->thumbs, (GFunc)callback, callback_data);

}

static GType
rstto_thumbnail_bar_child_type(GtkContainer *container)
{
    return GTK_TYPE_WIDGET;
}

/*
 * cb_rstto_thumbnail_bar_nav_new_entry :
 *
 * @nav    : RsttoNavigator 
 * @nr     : nr
 * @entry  :
 * @bar :
 *
 */
static void
cb_rstto_thumbnail_bar_nav_new_entry(RsttoNavigator *nav, gint nr, RsttoNavigatorEntry *entry, RsttoThumbnailBar *bar)
{
    GtkWidget *thumb;
    if (g_slist_length(bar->priv->thumbs) > 0)
    {
        thumb = rstto_thumbnail_new_from_widget(entry, bar->priv->thumbs->data);
    }
    else
    {
        thumb = rstto_thumbnail_new(entry, NULL);
    }
    g_signal_connect(G_OBJECT(thumb), "toggled", G_CALLBACK(cb_rstto_thumbnail_bar_thumbnail_toggled), bar);
    gtk_container_add(GTK_CONTAINER(bar), thumb);
    gtk_widget_show(thumb);
}

/*
 * cb_rstto_thumbnail_bar_nav_iter_changed :
 *
 * @nav    : RsttoNavigator 
 * @nr     : nr
 * @entry  :
 * @bar :
 *
 */
static void
cb_rstto_thumbnail_bar_nav_iter_changed(RsttoNavigator *nav, gint nr, RsttoNavigatorEntry *entry, RsttoThumbnailBar *bar)
{
    GSList *iter = bar->priv->thumbs;

    while (iter != NULL)
    {
        if (entry != rstto_thumbnail_get_entry(RSTTO_THUMBNAIL(iter->data)))
        {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(iter->data), FALSE);
        }
        else
        {
            gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(iter->data), TRUE);
        }
        iter = g_slist_next(iter);
    }
}

/*
 * cb_rstto_thumbnail_bar_nav_reordered :
 *
 * @nav    : RsttoNavigator 
 * @bar :
 *
 */
static void
cb_rstto_thumbnail_bar_nav_reordered (RsttoNavigator *nav, RsttoThumbnailBar *bar)
{
}

static void
cb_rstto_thumbnail_bar_thumbnail_toggled (RsttoThumbnail *thumb, RsttoThumbnailBar *bar)
{
    if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(thumb)) == TRUE)
    {
        rstto_navigator_entry_select (rstto_thumbnail_get_entry(thumb));
    }
}

static gint
cb_rstto_thumbnail_bar_compare (RsttoThumbnail *a, RsttoThumbnail *b)
{
    RsttoNavigatorEntry *_a = rstto_thumbnail_get_entry(a);
    RsttoNavigatorEntry *_b = rstto_thumbnail_get_entry(b);

    if (rstto_navigator_entry_get_position(_a) < rstto_navigator_entry_get_position(_b))
    {
        return -1;
    }
    else
    {
        return 1;
    }
}
