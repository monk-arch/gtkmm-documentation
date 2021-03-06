//$Id: dndwindow.cc 177 2004-03-28 18:58:09Z murrayc $ -*- c++ -*-

/* gtkmm example Copyright (C) 2002 gtkmm development team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "dndwindow.h"
#include "dnd_images.h"
#include <gtkmm/application.h>
#include <iostream>

DnDWindow::DnDWindow()
: m_Label_Drop("Drop here\n"), m_Label_Popup("Popup\n"),
  m_Button("Drag Here\n"),
  m_have_drag(false),
  m_PopupWindow(Gtk::WINDOW_POPUP)
{
  m_popped_up = false;
  m_in_popup = false;

  add(m_Grid);

  m_drag_icon = Gdk::Pixbuf::create_from_xpm_data(drag_icon_xpm);
  m_trashcan_open = Gdk::Pixbuf::create_from_xpm_data(trashcan_open_xpm);
  m_trashcan_closed = Gdk::Pixbuf::create_from_xpm_data(trashcan_closed_xpm);

  //Targets:
  m_listTargets.push_back( Gtk::TargetEntry("STRING", Gtk::TargetFlags(0), TARGET_STRING) );
  m_listTargets.push_back( Gtk::TargetEntry("text/plain", Gtk::TargetFlags(0), TARGET_STRING) );
  m_listTargets.push_back( Gtk::TargetEntry("application/x-rootwin-drop", Gtk::TargetFlags(0), TARGET_ROOTWIN) );

  //Targets without rootwin:
  m_listTargetsNoRoot.assign(m_listTargets.begin(), --m_listTargets.end());

  m_Label_Drop.drag_dest_set(m_listTargetsNoRoot, Gtk::DEST_DEFAULT_ALL, Gdk::DragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));

  m_Label_Drop.signal_drag_data_received().connect( sigc::mem_fun(*this, &DnDWindow::on_label_drop_drag_data_received) );

  m_Grid.attach(m_Label_Drop, 0, 0, 1, 1);
  m_Label_Drop.set_hexpand(true);
  m_Label_Drop.set_vexpand(true);

  m_Label_Popup.drag_dest_set(m_listTargetsNoRoot, Gtk::DEST_DEFAULT_ALL, Gdk::DragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));

  m_Grid.attach(m_Label_Popup, 1, 1, 1, 1);
  m_Label_Popup.set_hexpand(true);
  m_Label_Popup.set_vexpand(true);

  m_Label_Popup.signal_drag_motion().connect( sigc::mem_fun(*this, &DnDWindow::on_label_popup_drag_motion) );
  m_Label_Popup.signal_drag_leave().connect( sigc::mem_fun(*this, &DnDWindow::on_label_popup_drag_leave) );

  m_Image.set(m_trashcan_closed);
  m_Image.drag_dest_set();

  m_Grid.attach(m_Image, 1, 0, 1, 1);
  m_Image.set_hexpand(true);
  m_Image.set_vexpand(true);

  m_Image.signal_drag_leave().connect( sigc::mem_fun(*this, &DnDWindow::on_image_drag_leave) );
  m_Image.signal_drag_motion().connect( sigc::mem_fun(*this, &DnDWindow::on_image_drag_motion) );
  m_Image.signal_drag_drop().connect( sigc::mem_fun(*this, &DnDWindow::on_image_drag_drop) );
  m_Image.signal_drag_data_received().connect( sigc::mem_fun(*this, &DnDWindow::on_image_drag_data_received) );

  /* Drag site */

  m_Button.drag_source_set(m_listTargets, Gdk::ModifierType(GDK_BUTTON1_MASK | GDK_BUTTON3_MASK),
                           Gdk::DragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));

  m_Button.drag_source_set_icon(m_drag_icon);

  m_Grid.attach(m_Button, 0, 1, 1, 1);
  m_Button.set_hexpand(true);
  m_Button.set_vexpand(true);

  m_Button.signal_drag_data_get().connect( sigc::mem_fun(*this, &DnDWindow::on_button_drag_data_get));
  m_Button.signal_drag_data_delete().connect( sigc::mem_fun(*this, &DnDWindow::on_button_drag_data_delete));

  create_popup();

  show_all();
}

DnDWindow::~DnDWindow()
{
}


void DnDWindow::on_label_drop_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time)
{
  const int length = selection_data.get_length();
  const guchar* data = selection_data.get_data();

  if((length >= 0) && (selection_data.get_format() == 8))
  {
    g_print ("Received \"%s\" in label\n", (gchar *)data);
  }

  context->drag_finish(false, false, time);
}

bool DnDWindow::on_label_popup_drag_motion(const Glib::RefPtr<Gdk::DragContext>&, int, int, guint)
{
  if(!m_popup_timer)
    m_popup_timer = Glib::signal_timeout().connect( sigc::mem_fun(*this, &DnDWindow::on_popup_timeout), 500);

  return true;
}

void DnDWindow::on_label_popup_drag_leave(const Glib::RefPtr<Gdk::DragContext>&, guint)
{
 if(m_in_popup)
 {
   m_in_popup = false;
   if(!m_popdown_timer)
   {
     g_print ("added popdown\n");
     m_popdown_timer = Glib::signal_timeout().connect( sigc::mem_fun(*this, &DnDWindow::on_popdown_timeout), 500);
   }
 }
}

void DnDWindow::on_image_drag_data_received(const Glib::RefPtr<Gdk::DragContext>& context, int, int, const Gtk::SelectionData& selection_data, guint, guint time)
{
  const int length = selection_data.get_length();
  const guchar* data = selection_data.get_data();

  if( (length >= 0) && (selection_data.get_format() == 8) )
  {
    g_print ("Received \"%s\" in trashcan\n", (gchar*)data);
  }

  context->drag_finish(false, false, time);
}

bool DnDWindow::on_image_drag_motion(const Glib::RefPtr<Gdk::DragContext>& context, int, int, guint time)
{
  if(!m_have_drag)
  {
    m_have_drag = true;
    m_Image.set(m_trashcan_open);
  }

  Gtk::Widget* source_widget = Gtk::Widget::drag_get_source_widget(context);
  g_print ("motion, source %s\n", source_widget ?
           G_OBJECT_TYPE_NAME (source_widget) :
           "NULL");

  typedef std::vector<std::string> type_targets;
  const type_targets targets = context->list_targets();
  for(type_targets::const_iterator iter = targets.begin(); iter != targets.end(); ++iter)
  {
    const Glib::ustring name = *iter;
    g_print ("%s\n", name.c_str());
  }

  context->drag_status(context->get_suggested_action(), time);
  return true;
}

void DnDWindow::on_image_drag_leave(const Glib::RefPtr<Gdk::DragContext>&, guint)
{
  g_print("leave\n");
  m_have_drag = false;
  m_Image.set(m_trashcan_closed);
}

bool DnDWindow::on_image_drag_drop(const Glib::RefPtr<Gdk::DragContext>& context, int, int, guint time)
{
  g_print("drop\n");
  m_have_drag = false;

  m_Image.set(m_trashcan_closed);

  std::vector<std::string> targets = context->list_targets();
  if(!targets.empty())
  {
    drag_get_data( context, targets[0], time );
  }

  return true;
}


void DnDWindow::on_button_drag_data_get(const Glib::RefPtr<Gdk::DragContext>&, Gtk::SelectionData& selection_data, guint info, guint)
{
  if(info == TARGET_ROOTWIN)
    g_print ("I was dropped on the rootwin\n");
  else
    selection_data.set(selection_data.get_target(),
                       8 /* 8-bits format */, (const guchar*)"I'm Data!", 9 /* The length of I'm Data in bytes */);
}

void DnDWindow::on_button_drag_data_delete(const Glib::RefPtr<Gdk::DragContext>&)
{
  g_print ("Delete the data!\n");
}

bool DnDWindow::on_popdown_timeout()
{
  m_popdown_timer.disconnect();

  m_PopupWindow.hide();
  m_popped_up = false;

  return false;
}

bool DnDWindow::on_popup_timeout()
{
  if(!m_popped_up)
  {
    m_PopupWindow.show();
    m_popped_up = true;
  }

  m_popdown_timer = Glib::signal_timeout().connect( sigc::mem_fun(*this, &DnDWindow::on_popdown_timeout), 500);
  g_print ("added popdown\n");

  m_popup_timer.disconnect();

  return false;
}

void DnDWindow::create_popup()
{
  m_PopupWindow.set_position(Gtk::WIN_POS_MOUSE);

  //Create Grid and fill it:
  Gtk::Grid* pGrid = Gtk::manage(new Gtk::Grid());

  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < 3; j++)
    {
      char buffer[128];
      g_snprintf(buffer, sizeof(buffer), "%d,%d", i, j);
      Gtk::Button* pButton = Gtk::manage(new Gtk::Button(buffer));
      pGrid->attach(*pButton, i, j, 1, 1);

      pButton->drag_dest_set(m_listTargetsNoRoot, Gtk::DEST_DEFAULT_ALL, Gdk::DragAction(GDK_ACTION_COPY | GDK_ACTION_MOVE));
      pButton->signal_drag_motion().connect( sigc::mem_fun(*this, &DnDWindow::on_popup_button_drag_motion) );                      pButton->signal_drag_leave().connect( sigc::mem_fun(*this, &DnDWindow::on_popup_button_drag_leave) );
    }
  }

  pGrid->show_all();
  m_PopupWindow.add(*pGrid);
}

bool DnDWindow::on_popup_button_drag_motion(const Glib::RefPtr<Gdk::DragContext>&, int, int, guint)
{
  if(!m_in_popup)
  {
    m_in_popup = true;
    if(m_popdown_timer)
    {
      g_print ("removed popdown\n");
      m_popdown_timer.disconnect();
    }
  }

  return true;
}

void DnDWindow::on_popup_button_drag_leave(const Glib::RefPtr<Gdk::DragContext>&, guint)
{
 if(m_in_popup)
 {
   m_in_popup = false;
   if(!m_popdown_timer)
   {
     g_print ("added popdown\n");
     m_popdown_timer = Glib::signal_timeout().connect( sigc::mem_fun(*this, &DnDWindow::on_popdown_timeout), 500);
   }
 }
}
